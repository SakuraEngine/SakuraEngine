#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/render_device.h"
#include "utils/format.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"
#include "platform/memory.h"
#include "resource/resource_factory.h"
#include "resource/resource_system.h"
#include "utils/io.hpp"

#include "tracy/Tracy.hpp"

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
    ESkrLoadStatus Load(skr_resource_record_t* record) override;
    ESkrLoadStatus UpdateLoad(skr_resource_record_t* record) override;
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
        eastl::string bytes_uri;
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

ESkrLoadStatus SShaderResourceFactoryImpl::Load(skr_resource_record_t* record)
{ 
    auto newShader = SkrNew<skr_platform_shader_resource_t>();    
    auto resourceRequest = record->activeRequest;
    auto loadedData = resourceRequest->GetData();

    struct SpanReader
    {
        gsl::span<const uint8_t> data;
        size_t offset = 0;
        int read(void* dst, size_t size)
        {
            if (offset + size > data.size())
                return -1;
            memcpy(dst, data.data() + offset, size);
            offset += size;
            return 0;
        }
    } reader = { loadedData };

    skr_binary_reader_t archive{reader};
    skr::binary::Archive(&archive, *newShader);

    record->resource = newShader;
    return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED; 
}

ESkrLoadStatus SShaderResourceFactoryImpl::UpdateLoad(skr_resource_record_t* record)
{
    return ESkrLoadStatus::SKR_LOAD_STATUS_SUCCEED; 
}

bool SShaderResourceFactoryImpl::Unload(skr_resource_record_t* record)
{ 
    auto shader_resource = (skr_platform_shader_resource_t*)record->resource;
    if (shader_resource->shader) cgpu_free_shader_library(shader_resource->shader);
    SkrDelete(shader_resource);
    return true; 
}

ESkrInstallStatus SShaderResourceFactoryImpl::Install(skr_resource_record_t* record)
{
    auto bytes_vfs = root.bytecode_vfs;
    auto platform_shader = static_cast<skr_platform_shader_resource_t*>(record->resource);
    bool launch_success = false;
    for (uint32_t i = 0u; i < platform_shader->identifiers.size(); i++)
    {
        const auto& identifier = platform_shader->identifiers[i];
        const auto runtime_bytecode_type = GetRuntimeBytecodeType();
        if (identifier.bytecode_type == runtime_bytecode_type)
        {
            const auto hash = identifier.hash;
            const auto uri = skr::format("{}#{}-{}-{}-{}.bytes", hash.flags, 
                hash.encoded_digits[0], hash.encoded_digits[1], hash.encoded_digits[2], hash.encoded_digits[3]);
            auto sRequest = SPtr<ShaderRequest>::Create(this, uri.c_str(), platform_shader);
            auto found = mShaderRequests.find(platform_shader);
            SKR_ASSERT(found == mShaderRequests.end());
            mShaderRequests.emplace(platform_shader, sRequest);
            
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
            platform_shader->active_slot = i;
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
    auto platform_shader = static_cast<skr_platform_shader_resource_t*>(record->resource);
    auto sRequest = mShaderRequests.find(platform_shader);
    if (sRequest != mShaderRequests.end())
    {
        auto okay = skr_atomic32_load_acquire(&sRequest->second->shader_created);
        auto status = okay ? SKR_INSTALL_STATUS_SUCCEED : SKR_INSTALL_STATUS_INPROGRESS;
        if (okay)
        {
            mShaderRequests.erase(platform_shader);
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