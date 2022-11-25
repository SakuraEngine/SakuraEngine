#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/render_device.h"
#include "utils/format.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"
#include "platform/memory.h"
#include "resource/resource_factory.h"
#include "resource/resource_system.h"
#include "containers/hashmap.hpp"
#include "containers/btree.hpp"
#include "utils/io.hpp"

#include "tracy/Tracy.hpp"

skr_stable_shader_hash_t::skr_stable_shader_hash_t(uint32_t v) SKR_NOEXCEPT
{
    value = v;
}

skr_stable_shader_hash_t::skr_stable_shader_hash_t(uint64_t v) SKR_NOEXCEPT
{
    value = v;
}

skr_stable_shader_hash_t::skr_stable_shader_hash_t(const char* str) SKR_NOEXCEPT
{
    value = std::stoull(str);
}

skr_stable_shader_hash_t::operator skr::string() const SKR_NOEXCEPT
{
    return skr::format("{}", value);
}

bool skr_shader_options_resource_t::flatten_options(eastl::vector<skr_shader_option_t>& dst, skr::span<skr_shader_options_resource_t*> srcs) SKR_NOEXCEPT
{
    eastl::set<eastl::string> keys;
    skr::flat_hash_map<eastl::string, eastl::vector<eastl::string>, eastl::hash<eastl::string>> kvs;
    // collect all keys & ensure unique
    for (auto& src : srcs)
    {
        for (auto& opt : src->options)
        {
            auto&& found = keys.find(opt.key);
            if (found != keys.end())
            {
                dst.empty();
                return false;
            }
            keys.insert(opt.key);
            kvs.insert({opt.key, opt.value_selections});
        }
    }
    dst.reserve(keys.size());
    for (auto& key : keys)
    {
        dst.push_back({key, kvs[key]});
    }
    return true;
}

skr_shader_options_md5_t skr_shader_options_md5_t::calculate(skr::span<skr_shader_option_instance_t> ordered_options)
{
    // TODO: check ordered here
    eastl::string sourceString;
    for (auto&& option : ordered_options)
    {
        sourceString += option.key;
        sourceString += "=";
        sourceString += option.value;
        sourceString += ";";
    }
    auto result = make_zeroed<skr_shader_options_md5_t>();
    // TODO: replace MD5 algorithm
    const uint32_t seeds[4] = { 114u, 514u, 1919u, 810u };
    result.a = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[0]);
    result.b = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[1]);
    result.c = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[2]);
    result.d = skr_hash32(sourceString.c_str(), (uint32_t)sourceString.size(), seeds[3]);
    return result;
}

namespace skr
{
namespace resource
{
struct SKR_RENDERER_API SShaderOptionsFactoryImpl : public SShaderOptionsFactory
{
    SShaderOptionsFactoryImpl(const SShaderOptionsFactoryImpl::Root& root)
        : root(root)
    {

    }

    ~SShaderOptionsFactoryImpl() noexcept = default;

    bool AsyncIO() override { return false; }
    skr_type_id_t GetResourceType() override
    {
        const auto collection_type = skr::type::type_id<skr_shader_options_resource_t>::get();
        return collection_type;
    }

    ESkrInstallStatus Install(skr_resource_record_t* record) override
    {
        return ::SKR_INSTALL_STATUS_SUCCEED; // no need to install
    }
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override
    {
        return ::SKR_INSTALL_STATUS_SUCCEED; // no need to install
    }

    bool Unload(skr_resource_record_t* record) override
    {
        auto options = (skr_shader_options_resource_t*)record->resource;
        SkrDelete(options);
        return true; 
    }
    bool Uninstall(skr_resource_record_t* record) override
    {
        return true;
    }
    void DestroyResource(skr_resource_record_t* record) override {}
    
    Root root;
};

SShaderOptionsFactory* SShaderOptionsFactory::Create(const Root& root)
{
    return SkrNew<SShaderOptionsFactoryImpl>(root);
}

void SShaderOptionsFactory::Destroy(SShaderOptionsFactory *factory)
{
    return SkrDelete(factory);
}

struct SKR_RENDERER_API SShaderResourceFactoryImpl : public SShaderResourceFactory
{
    SShaderResourceFactoryImpl(const SShaderResourceFactory::Root& root)
        : root(root)
    {

    }

    ~SShaderResourceFactoryImpl() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;
    void DestroyResource(skr_resource_record_t* record) override;

    ECGPUShaderBytecodeType GetRuntimeBytecodeType() const
    {
        const auto backend = root.render_device->get_backend();
        switch (backend)
        {
        case CGPU_BACKEND_D3D12: return CGPU_SHADER_BYTECODE_TYPE_DXIL;
        case CGPU_BACKEND_VULKAN: return CGPU_SHADER_BYTECODE_TYPE_SPIRV;
        case CGPU_BACKEND_METAL: return CGPU_SHADER_BYTECODE_TYPE_MTL;
        default: return CGPU_SHADER_BYTECODE_TYPE_COUNT;
        }
    }

    struct ShaderRequest
    {
        ShaderRequest(SShaderResourceFactoryImpl* factory, const char* uri, skr_platform_shader_resource_t* platform_shader)
            : factory(factory), bytes_uri(uri), platform_shader(platform_shader)
        {

        }

        bool createShader()
        {
            auto render_device = factory->root.render_device;
            const auto& shader_destination = bytes_destination;
    
            auto desc = make_zeroed<CGPUShaderLibraryDescriptor>();
            desc.code = (const uint32_t*)shader_destination.bytes;
            desc.code_size = (uint32_t)shader_destination.size;
            desc.name = bytes_uri.c_str();
            desc.stage = (ECGPUShaderStage)platform_shader->shader_stage;
            platform_shader->shader = cgpu_create_shader_library(render_device->get_cgpu_device(), &desc);
            skr_atomic32_add_relaxed(&shader_created, 1);
            return true;
        }

        SShaderResourceFactoryImpl* factory = nullptr;
        skr::string bytes_uri;
        skr_platform_shader_resource_t* platform_shader = nullptr;
        skr_async_request_t bytes_request;
        skr_async_ram_destination_t bytes_destination;
        skr_async_request_t aux_request;
        SAtomic32 shader_created = 0;
    };

    Root root;
    skr::flat_hash_map<skr_platform_shader_resource_t*, SPtr<ShaderRequest>> mShaderRequests;
};

SShaderResourceFactory* SShaderResourceFactory::Create(const Root& root)
{
    return SkrNew<SShaderResourceFactoryImpl>(root);
}

void SShaderResourceFactory::Destroy(SShaderResourceFactory *factory)
{
    return SkrDelete(factory);
}

skr_type_id_t SShaderResourceFactoryImpl::GetResourceType()
{
    const auto resource_type = skr::type::type_id<skr_platform_shader_resource_t>::get();
    return resource_type;
}

bool SShaderResourceFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto shader_collection = (skr_platform_shader_collection_resource_t*)record->resource;
    for (auto&& [hash, variant] : shader_collection->variants)
    {
        if (variant.shader) cgpu_free_shader_library(variant.shader);
    }
    SkrDelete(shader_collection);
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::Install(skr_resource_record_t* record)
{
    auto bytes_vfs = root.bytecode_vfs;
    auto shader_collection = static_cast<skr_platform_shader_collection_resource_t*>(record->resource);
    const auto root_hash = skr_stable_shader_hash_t(0u);
    auto&& root_variant_iter = shader_collection->variants.find(root_hash);
    SKR_ASSERT(root_variant_iter != shader_collection->variants.end() && "Root shader variant missing!");
    auto* root_variant = &root_variant_iter->second;
    bool launch_success = false;
    for (uint32_t i = 0u; i < root_variant->identifiers.size(); i++)
    {
        const auto& identifier = root_variant->identifiers[i];
        const auto runtime_bytecode_type = GetRuntimeBytecodeType();
        if (identifier.bytecode_type == runtime_bytecode_type)
        {
            const auto hash = identifier.hash;
            const auto uri = skr::format("{}#{}-{}-{}-{}.bytes", hash.flags, 
                hash.encoded_digits[0], hash.encoded_digits[1], hash.encoded_digits[2], hash.encoded_digits[3]);
            auto sRequest = SPtr<ShaderRequest>::Create(this, uri.c_str(), root_variant);
            auto found = mShaderRequests.find(root_variant);
            SKR_ASSERT(found == mShaderRequests.end());
            mShaderRequests.emplace(root_variant, sRequest);
            
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
            root.ram_service->request(bytes_vfs, &ram_texture_io, &sRequest->bytes_request, &sRequest->bytes_destination);

            launch_success = true;
            root_variant->active_slot = i;
            break;
        }
    }
    return launch_success ? SKR_INSTALL_STATUS_INPROGRESS : SKR_INSTALL_STATUS_FAILED;
}

bool SShaderResourceFactoryImpl::Uninstall(skr_resource_record_t* record)
{
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::UpdateInstall(skr_resource_record_t* record)
{
    auto shader_collection = static_cast<skr_platform_shader_collection_resource_t*>(record->resource);
    const auto root_hash = skr_stable_shader_hash_t(0u);
    auto&& root_variant_iter = shader_collection->variants.find(root_hash);
    SKR_ASSERT(root_variant_iter != shader_collection->variants.end() && "Root shader variant missing!");
    auto* root_variant = &root_variant_iter->second;
    auto sRequest = mShaderRequests.find(root_variant);
    if (sRequest != mShaderRequests.end())
    {
        auto okay = skr_atomic32_load_acquire(&sRequest->second->shader_created);
        auto status = okay ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
        if (okay)
        {
            mShaderRequests.erase(root_variant);
        }
        return status;
    }
    else
    {
        SKR_UNREACHABLE_CODE();
    }
    return ESkrInstallStatus::SKR_INSTALL_STATUS_FAILED;
}

void SShaderResourceFactoryImpl::DestroyResource(skr_resource_record_t* record)
{
    return;
}

} // namespace resource
} // namespace skr