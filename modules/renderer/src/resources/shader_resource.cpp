#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/render_device.h"
#include "utils/format.hpp"
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"
#include "platform/memory.h"
#include "resource/resource_factory.h"
#include "resource/resource_system.h"
#include "containers/hashmap.hpp"
#include "containers/sptr.hpp"
#include "utils/io.h"
#include "option_utils.hpp"

#include "tracy/Tracy.hpp"

skr_stable_shader_hash_t::skr_stable_shader_hash_t(uint32_t a, uint32_t b, uint32_t c, uint32_t d) SKR_NOEXCEPT
    : valuea(a), valueb(b), valuec(c), valued(d)
{

}

size_t skr_stable_shader_hash_t::hasher::operator()(const skr_stable_shader_hash_t &hash) const
{
    return skr_hash(&hash, sizeof(hash), 114514u);
}

skr_stable_shader_hash_t skr_stable_shader_hash_t::hash_string(const char* str, uint32_t size) SKR_NOEXCEPT
{
    auto result = make_zeroed<skr_stable_shader_hash_t>();
    const uint32_t seeds[4] = { 114u, 514u, 1919u, 810u };
    result.valuea = skr_hash32(str, size, seeds[0]);
    result.valueb = skr_hash32(str, size, seeds[1]);
    result.valuec = skr_hash32(str, size, seeds[2]);
    result.valued = skr_hash32(str, size, seeds[3]);
    return result;
}

skr_stable_shader_hash_t skr_stable_shader_hash_t::from_string(const char* str) SKR_NOEXCEPT
{
    skr_stable_shader_hash_t result;
    result.valuea = std::stoul(str);
    result.valueb = std::stoul(str + 8);
    result.valuec = std::stoul(str + 16);
    result.valued = std::stoul(str + 24);
    return result;
}

skr_stable_shader_hash_t::operator skr::string() const SKR_NOEXCEPT
{
    return skr::format("{}{}{}{}", valuea, valueb, valuec, valued);
}

skr_stable_shader_hash_t skr_shader_switch_sequence_t::calculate_stable_hash(const skr_shader_switch_sequence_t& seq, skr::span<uint32_t> indices)
{
    option_utils::opt_signature_string signatureString;
    option_utils::stringfy(signatureString, seq, indices);
    return skr_stable_shader_hash_t::hash_string(signatureString.c_str(), (uint32_t)signatureString.size());
}

skr_stable_shader_hash_t skr_shader_option_sequence_t::calculate_stable_hash(const skr_shader_option_sequence_t& seq, skr::span<uint32_t> indices)
{
    option_utils::opt_signature_string signatureString;
    option_utils::stringfy(signatureString, seq, indices);
    return skr_stable_shader_hash_t::hash_string(signatureString.c_str(), (uint32_t)signatureString.size());
}

namespace skr
{
namespace resource
{
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
    const auto resource_type = skr::type::type_id<skr_platform_shader_collection_resource_t>::get();
    return resource_type;
}

bool SShaderResourceFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto shader_collection = (skr_platform_shader_collection_resource_t*)record->resource;
    for (auto&& [hash, variant] : shader_collection->switch_variants)
    {
        if (variant.shader) cgpu_free_shader_library(variant.shader);
    }
    SkrDelete(shader_collection);
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::Install(skr_resource_record_t* record)
{
    if (root.dont_create_shader) return SKR_INSTALL_STATUS_SUCCEED;
    
    auto bytes_vfs = root.bytecode_vfs;
    auto shader_collection = static_cast<skr_platform_shader_collection_resource_t*>(record->resource);
    auto&& root_switch_variant = shader_collection->GetRootStaticVariant();
    auto* pPlatformResource = &root_switch_variant;
    auto&& root_option_variant = root_switch_variant.GetRootDynamicVariants();
    bool launch_success = false;
    // load bytecode and create CGPU shader
    for (uint32_t i = 0u; i < root_option_variant.size(); i++)
    {
        const auto& identifier = root_option_variant[i];
        const auto runtime_bytecode_type = GetRuntimeBytecodeType();
        if (identifier.bytecode_type == runtime_bytecode_type)
        {
            const auto hash = identifier.hash;
            const auto uri = skr::format("{}#{}-{}-{}-{}.bytes", hash.flags, 
                hash.encoded_digits[0], hash.encoded_digits[1], hash.encoded_digits[2], hash.encoded_digits[3]);
            auto sRequest = SPtr<ShaderRequest>::Create(this, uri.c_str(), pPlatformResource);
            auto found = mShaderRequests.find(pPlatformResource);
            SKR_ASSERT(found == mShaderRequests.end());
            mShaderRequests.emplace(pPlatformResource, sRequest);
            
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
            pPlatformResource->active_slot = i;
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
    auto&& root_variant_iter = shader_collection->switch_variants.find(kZeroStableShaderHash);
    SKR_ASSERT(root_variant_iter != shader_collection->switch_variants.end() && "Root shader variant missing!");
    auto* root_variant = &root_variant_iter->second;
    auto sRequest = mShaderRequests.find(root_variant);
    if (sRequest != mShaderRequests.end())
    {
        auto okay = skr_atomic32_load_acquire(&sRequest->second->shader_created);
        auto status = okay ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
        if (okay)
        {
            sakura_free(sRequest->second->bytes_destination.bytes);
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

} // namespace resource
} // namespace skr