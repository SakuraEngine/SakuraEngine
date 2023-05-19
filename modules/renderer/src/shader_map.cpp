#include "SkrRenderer/shader_map.h"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/shader_hash.h"

#include "misc/log.h"
#include "misc/defer.hpp"
#include "misc/make_zeroed.hpp"
#include "misc/threaded_service.h"
#include "platform/atomic.h"
#include "async/thread_job.hpp"

#include "containers/hashmap.hpp"
#include "containers/string.hpp"
#include "containers/sptr.hpp"

namespace skr
{
struct ShaderMapImpl;
namespace
{

using ShaderFuture = skr::IFuture<bool>;
using JobQueueFuture = skr::ThreadedJobQueueFuture<bool>;
using SerialFuture = skr::SerialFuture<bool>;
struct ShaderFutureLauncher
{
    ShaderFutureLauncher(skr::JobQueue* q) : job_queue(q) {}
    template<typename F, typename... Args>
    ShaderFuture* async(F&& f, Args&&... args)
    {
        if (job_queue)
            return SkrNew<JobQueueFuture>(job_queue, std::forward<F>(f), std::forward<Args>(args)...);
        else
            return SkrNew<SerialFuture>(std::forward<F>(f), std::forward<Args>(args)...);
    }
    skr::JobQueue* job_queue = nullptr;
};

}

struct ShaderProgress : public skr::AsyncProgress<ShaderFutureLauncher, int, bool>
{
    ShaderProgress(ShaderMapImpl* factory, const char8_t* uri, const skr_platform_shader_identifier_t& identifier)
        : factory(factory), bytes_uri(uri), identifier(identifier)
    {

    }

    bool do_in_background() override;

    ShaderMapImpl* factory = nullptr;
    skr::string bytes_uri;
    skr_async_request_t bytes_request;
    skr_async_ram_destination_t bytes_destination;
    skr_platform_shader_identifier_t identifier = {};
};

struct ShaderMapImpl : public skr_shader_map_t
{
    ShaderMapImpl(const skr_shader_map_root_t& root)
        : root(root)
    {
        jq_launcher = SPtr<ShaderFutureLauncher>::Create(root.job_queue);
    }

    ~ShaderMapImpl()
    {
        for (auto iter : mShaderMap)
        {
            if (skr_atomicu32_load_relaxed(&iter.second->frame) != UINT64_MAX)
            {
                cgpu_free_shader_library(iter.second->shader);
            }
        }
    }

    CGPUShaderLibraryId find_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT override;
    ESkrShaderMapShaderStatus install_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT override;
    bool free_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT override;

    void new_frame(uint64_t frame_index) SKR_NOEXCEPT override;
    void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT override;

    ESkrShaderMapShaderStatus install_shader_from_vfs(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT;

    struct MappedShader
    {
        CGPUShaderLibraryId shader = nullptr;
        SAtomicU64 frame = UINT64_MAX;
        SAtomicU32 rc = 0;
        SAtomicU32 shader_status = 0;
    };

    SPtr<ShaderFutureLauncher> jq_launcher;
    uint64_t frame_index = 0;
    skr_shader_map_root_t root;

    skr::parallel_flat_hash_map<skr_platform_shader_identifier_t, SPtr<MappedShader>, skr_platform_shader_identifier_t::hasher> mShaderMap;
    skr::parallel_flat_hash_map<skr_platform_shader_identifier_t, SPtr<ShaderProgress>, skr_platform_shader_identifier_t::hasher> mShaderTasks;
};

bool ShaderProgress::do_in_background()
{
    auto device = factory->root.device;
    const auto& shader_destination = bytes_destination;
    auto& shader = factory->mShaderMap[identifier];

    skr_atomicu32_store_relaxed(&shader->shader_status, SKR_SHADER_MAP_SHADER_STATUS_LOADED);

    auto desc = make_zeroed<CGPUShaderLibraryDescriptor>();
    desc.code = (const uint32_t*)shader_destination.bytes;
    desc.code_size = (uint32_t)shader_destination.size;
    desc.name = bytes_uri.u8_str();
    desc.stage = identifier.shader_stage;
    const auto created_shader = cgpu_create_shader_library(device, &desc);
    if (!created_shader)
    {
        skr_atomicu32_store_relaxed(&shader->shader_status, SKR_SHADER_MAP_SHADER_STATUS_FAILED);
        skr_atomicu64_store_relaxed(&factory->mShaderMap[identifier]->frame, UINT64_MAX);
        return false;
    }
    else
    {
        shader->shader = created_shader;
        skr_atomicu32_store_relaxed(&shader->shader_status, SKR_SHADER_MAP_SHADER_STATUS_INSTALLED);
        skr_atomicu64_store_relaxed(&factory->mShaderMap[identifier]->frame, factory->frame_index);
    }
    return true;
}

CGPUShaderLibraryId ShaderMapImpl::find_shader(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    auto found = mShaderMap.find(identifier);
    if (found != mShaderMap.end())
    {
        if (skr_atomicu32_load_relaxed(&found->second->frame) != UINT64_MAX)
        {
            return found->second->shader;
        }
    }
    return nullptr;
}

bool ShaderMapImpl::free_shader(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    auto found = mShaderMap.find(identifier);
    if (found != mShaderMap.end())
    {
    #ifdef _DEBUG
        const auto frame = skr_atomicu32_load_relaxed(&found->second->frame);
        SKR_ASSERT(frame != UINT64_MAX && "this shader is freed but never installed, check your code for errors!");
    #endif
        skr_atomicu32_add_relaxed(&found->second->rc, -1);
        return true;
    }
    return false;
}

ESkrShaderMapShaderStatus ShaderMapImpl::install_shader(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    auto found = mShaderMap.find(identifier);
    // 1. found mapped shader
    if (found != mShaderMap.end())
    {
        // 1.1 found alive shader request
        auto status = skr_atomicu32_load_relaxed(&found->second->shader_status);

        // 1.2 request is done, add rc & record frame index
        skr_atomicu32_add_relaxed(&found->second->rc, 1);
        skr_atomicu64_store_relaxed(&found->second->frame, frame_index);
        return (ESkrShaderMapShaderStatus)status;
    }
    // 2. not found mapped shader
    else
    {
        // fire request
        auto mapped_shader = SPtr<MappedShader>::Create();
        skr_atomicu32_add_relaxed(&mapped_shader->rc, 1);
        // keep mapped_shader::frame at UINT64_MAX until shader is loaded
        mShaderMap.emplace(identifier, mapped_shader);
        return install_shader_from_vfs(identifier);
    }
}

ESkrShaderMapShaderStatus ShaderMapImpl::install_shader_from_vfs(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    bool launch_success = false;
    auto bytes_vfs = root.bytecode_vfs;
    SKR_ASSERT(bytes_vfs);
    const auto hash = identifier.hash;
    const auto uri = skr::format(u8"{}#{}-{}-{}-{}.bytes", hash.flags, 
        hash.encoded_digits[0], hash.encoded_digits[1], hash.encoded_digits[2], hash.encoded_digits[3]);
    auto sRequest = SPtr<ShaderProgress>::Create(this, uri.u8_str(), identifier);
    auto found = mShaderTasks.find(identifier);
    SKR_ASSERT(found == mShaderTasks.end());
    mShaderTasks.emplace(identifier, sRequest);
    
    auto ram_texture_io = make_zeroed<skr_ram_io_t>();
    ram_texture_io.path = sRequest->bytes_uri.u8_str();
    ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
        ZoneScopedN("CreateShaderFromBytes");
        
        auto progress = (ShaderProgress*)data;
        auto factory = progress->factory;
        if (auto launcher = factory->jq_launcher.get()) // create shaders on aux thread
        {
            progress->execute(*launcher);
        }
        else // create shaders inplace
        {
            SKR_ASSERT("launcher is unexpected null!");
        }
    };
    ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)sRequest.get();
    root.ram_service->request(bytes_vfs, &ram_texture_io, &sRequest->bytes_request, &sRequest->bytes_destination);
    launch_success = true;
    return launch_success ? SKR_SHADER_MAP_SHADER_STATUS_REQUESTED : SKR_SHADER_MAP_SHADER_STATUS_FAILED;
}

void ShaderMapImpl::new_frame(uint64_t index) SKR_NOEXCEPT
{
    SKR_ASSERT(index > frame_index);

    frame_index = index;
}

void ShaderMapImpl::garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT
{
    // erase request when exit this scope
    // SKR_DEFER({factory->mShaderTasks.erase(identifier);});
    for (auto it = mShaderMap.begin(); it != mShaderMap.end();)
    {
        auto status = skr_atomicu32_load_relaxed(&it->second->shader_status);
        if (status == SKR_SHADER_MAP_SHADER_STATUS_INSTALLED)
        {
            mShaderTasks.erase(it->first);
        }

        if (skr_atomicu32_load_relaxed(&it->second->rc) == 0 && 
            skr_atomicu64_load_relaxed(&it->second->frame) < critical_frame)
        {
            if (mShaderTasks.find(it->first) != mShaderTasks.end())
            {
                // shader is still loading, skip & wait for it to finish
                // TODO: cancel shader loading
                ++it;
            }
            else
            {
                it = mShaderMap.erase(it);
            }
        }
        else
        {
            ++it;
        }
    }
}
} // namespace skr

skr_shader_map_id skr_shader_map_t::Create(const struct skr_shader_map_root_t* desc) SKR_NOEXCEPT
{
    return SkrNew<skr::ShaderMapImpl>(*desc);
}

bool skr_shader_map_t::Free(skr_shader_map_id shader_map) SKR_NOEXCEPT
{
    SkrDelete(shader_map);
    return true;
}

skr_shader_map_id skr_shader_map_create(const struct skr_shader_map_root_t* desc)
{
    return skr_shader_map_t::Create(desc);
}

ESkrShaderMapShaderStatus skr_shader_map_install_shader(skr_shader_map_id shaderMap, const skr_platform_shader_identifier_t* key)
{
    return shaderMap->install_shader(*key);
}

CGPUShaderLibraryId skr_shader_map_find_shader(skr_shader_map_id shaderMap, const skr_platform_shader_identifier_t* id)
{
    return shaderMap->find_shader(*id);
}

void skr_shader_map_free_shader(skr_shader_map_id shaderMap, const skr_platform_shader_identifier_t* id)
{
    shaderMap->free_shader(*id);
}

void skr_shader_map_new_frame(skr_shader_map_id shaderMap, uint64_t frame_index)
{
    shaderMap->new_frame(frame_index);
}

void skr_shader_map_garbage_collect(skr_shader_map_id shaderMap, uint64_t critical_frame)
{
    shaderMap->garbage_collect(critical_frame);
}

void skr_shader_map_free(skr_shader_map_id id)
{
    skr_shader_map_t::Free(id);
}