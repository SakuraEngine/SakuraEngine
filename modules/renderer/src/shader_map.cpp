#include "SkrRenderer/shader_map.h"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "platform/guid.hpp"
#include "platform/atomic.h"
#include "containers/hashmap.hpp"
#include "containers/sptr.hpp"
#include "utils/format.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"

namespace skr
{
struct ShaderMapImpl : public skr_shader_map_t
{
    CGPUShaderLibraryId find_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT override;
    ESkrShaderMapShaderStatus install_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT override;
    bool free_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT override;

    void new_frame(uint64_t frame_index) SKR_NOEXCEPT override;
    void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT override;

    ESkrShaderMapShaderStatus install_shader_from_vfs(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT;

    struct MappedShader
    {
        CGPUShaderLibraryId shader = nullptr;
        SAtomic64 frame = UINT64_MAX;
        SAtomic32 rc = 0;
    };

    uint64_t frame_index = 0;
    skr_shader_map_root_t root;

    struct ShaderRequest
    {
        ShaderRequest(ShaderMapImpl* factory, const char* uri, const skr_platform_shader_identifier_t& identifier)
            : factory(factory), bytes_uri(uri), identifier(identifier)
        {

        }

        bool createShader()
        {
            auto render_device = factory->root.render_device;
            const auto& shader_destination = bytes_destination;
    
            skr_atomic32_add_relaxed(&shader_status, SKR_SHADER_MAP_SHADER_STATUS_LOADED);
    
            auto desc = make_zeroed<CGPUShaderLibraryDescriptor>();
            desc.code = (const uint32_t*)shader_destination.bytes;
            desc.code_size = (uint32_t)shader_destination.size;
            desc.name = bytes_uri.c_str();
            desc.stage = identifier.shader_stage;
            const auto created_shader = cgpu_create_shader_library(render_device->get_cgpu_device(), &desc);
            if (!created_shader)
            {
                skr_atomic32_add_relaxed(&shader_status, SKR_SHADER_MAP_SHADER_STATUS_FAILED);
                skr_atomic64_store_relaxed(&factory->map[identifier]->frame, UINT64_MAX);
                return false;
            }
            else
            {
                factory->map[identifier]->shader = created_shader;
                skr_atomic32_add_relaxed(&shader_status, SKR_SHADER_MAP_SHADER_STATUS_INSTALLED);
                skr_atomic64_store_relaxed(&factory->map[identifier]->frame, factory->frame_index);
            }
            return true;
        }

        ShaderMapImpl* factory = nullptr;
        skr::string bytes_uri;
        skr_async_request_t bytes_request;
        skr_async_ram_destination_t bytes_destination;
        skr_async_request_t aux_request;
        skr_platform_shader_identifier_t identifier = {};
        SAtomic32 shader_status = 0;
    };
    skr::parallel_flat_hash_map<skr_platform_shader_identifier_t, SPtr<MappedShader>, skr_platform_shader_identifier_t::hasher> map;
    skr::parallel_flat_hash_map<skr_platform_shader_identifier_t, SPtr<ShaderRequest>, skr_platform_shader_identifier_t::hasher> mShaderRequests;
};

CGPUShaderLibraryId ShaderMapImpl::find_shader(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    auto found = map.find(identifier);
    if (found != map.end())
    {
        if (skr_atomic32_load_relaxed(&found->second->frame) != UINT64_MAX)
        {
            return found->second->shader;
        }
    }
    return nullptr;
}

bool ShaderMapImpl::free_shader(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    auto found = map.find(identifier);
    if (found != map.end())
    {
    #ifdef _DEBUG
        const auto before = skr_atomic32_load_relaxed(&found->second->rc);
        SKR_ASSERT(before > 0);
    #endif
        skr_atomic32_add_relaxed(&found->second->rc, -1);
        return true;
    }
    return false;
}

ESkrShaderMapShaderStatus ShaderMapImpl::install_shader(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    auto found = map.find(identifier);
    if (found != map.end())
    {
        skr_atomic32_add_relaxed(&found->second->rc, 1);
        skr_atomic64_store_relaxed(&found->second->frame, frame_index);
        return SKR_SHADER_MAP_SHADER_STATUS_INSTALLED;
    }
    else
    {
        auto mapped_shader = SPtr<MappedShader>::Create();
        skr_atomic32_add_relaxed(&mapped_shader->rc, 1);
        // keep mapped_shader::frame at UINT64_MAX until shader is loaded
        map.emplace(identifier, mapped_shader);
        return install_shader_from_vfs(identifier);
    }
}

ESkrShaderMapShaderStatus ShaderMapImpl::install_shader_from_vfs(const skr_platform_shader_identifier_t& identifier) SKR_NOEXCEPT
{
    bool launch_success = false;
    auto bytes_vfs = root.bytecode_vfs;
    const auto hash = identifier.hash;
    const auto uri = skr::format("{}#{}-{}-{}-{}.bytes", hash.flags, 
        hash.encoded_digits[0], hash.encoded_digits[1], hash.encoded_digits[2], hash.encoded_digits[3]);
    auto sRequest = SPtr<ShaderRequest>::Create(this, uri.c_str(), identifier);
    auto found = mShaderRequests.find(identifier);
    SKR_ASSERT(found == mShaderRequests.end());
    mShaderRequests.emplace(identifier, sRequest);
    
    auto ram_texture_io = make_zeroed<skr_ram_io_t>();
    ram_texture_io.path = sRequest->bytes_uri.c_str();
    ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
        ZoneScopedN("LoadShaderBytes");
        
        auto sRequest = (ShaderRequest*)data;
        auto factory = sRequest->factory;
        if (auto aux_service = factory->root.aux_service) // create shaders on aux thread
        {
            auto aux_task = make_zeroed<skr_service_task_t>();
            aux_task.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* usrdata){
                ZoneScopedN("CreateShader(AuxService)");

                auto sRequest = (ShaderRequest*)usrdata;
                sRequest->createShader();
            };
            aux_task.callback_datas[SKR_ASYNC_IO_STATUS_OK] = sRequest;
            aux_service->request(&aux_task, &sRequest->aux_request);
        }
        else // create shaders inplace
        {
            ZoneScopedN("CreateShader(InPlace)");

            sRequest->createShader();
        }
    };
    ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)sRequest.get();
    ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_RAM_LOADING] = +[](skr_async_request_t* request, void* data) noexcept {
        auto sRequest = (ShaderRequest*)data;
        skr_atomic32_add_relaxed(&sRequest->shader_status, SKR_SHADER_MAP_SHADER_STATUS_LOADING);
    };
    ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_RAM_LOADING] = (void*)sRequest.get();
    root.ram_service->request(bytes_vfs, &ram_texture_io, &sRequest->bytes_request, &sRequest->bytes_destination);
    launch_success = true;
    return launch_success ? SKR_SHADER_MAP_SHADER_STATUS_REQUESTED : SKR_SHADER_MAP_SHADER_STATUS_NONE;
}

void ShaderMapImpl::new_frame(uint64_t index) SKR_NOEXCEPT
{
    SKR_ASSERT(index > frame_index);
    
    frame_index = index;
}

void ShaderMapImpl::garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT
{
    for (auto it = map.begin(); it != map.end();)
    {
        if (skr_atomic32_load_relaxed(&it->second->rc) == 0 && skr_atomic64_load_relaxed(&it->second->frame) < critical_frame)
        {
            it = map.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


} // namespace skr