#include "SkrGraphics/api.h"
#include "SkrRT/config.h"
#include <SkrContainers/string.hpp>

#include "spirv.h"
#include "dxil.h"
#include <vector>

#include "SkrTestFramework/framework.hpp"

template <ECGPUBackend backend>
class ResourceCreation
{
protected:
    void Initialize() SKR_NOEXCEPT
    {
        SKR_DECLARE_ZERO(CGPUInstanceDescriptor, desc)
        desc.backend = backend;
        desc.enable_debug_layer = false;
        desc.enable_gpu_based_validation = false;
        desc.enable_set_name = true;
        instance = cgpu_create_instance(&desc);

        EXPECT_NE(instance, CGPU_NULLPTR);
        EXPECT_NE(instance, nullptr);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, nullptr, &adapters_count);
        std::vector<CGPUAdapterId> adapters;
        adapters.resize(adapters_count);
        cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
        adapter = adapters[0];

        CGPUQueueGroupDescriptor G = { CGPU_QUEUE_TYPE_GRAPHICS, 1 };
        SKR_DECLARE_ZERO(CGPUDeviceDescriptor, descriptor)
        descriptor.queue_groups = &G;
        descriptor.queue_group_count = 1;
        device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);

        vertex_shaders[CGPU_BACKEND_VULKAN] = (const uint32_t*)triangle_vert_spirv;
        vertex_shader_sizes[CGPU_BACKEND_VULKAN] = sizeof(triangle_vert_spirv);
        frag_shaders[CGPU_BACKEND_VULKAN] = (const uint32_t*)triangle_frag_spirv;
        frag_shader_sizes[CGPU_BACKEND_VULKAN] = sizeof(triangle_frag_spirv);
        compute_shaders[CGPU_BACKEND_VULKAN] = (const uint32_t*)simple_compute_spirv;
        compute_shader_sizes[CGPU_BACKEND_VULKAN] = sizeof(simple_compute_spirv);

        vertex_shaders[CGPU_BACKEND_D3D12] = (const uint32_t*)triangle_vert_dxil;
        vertex_shader_sizes[CGPU_BACKEND_D3D12] = sizeof(triangle_vert_dxil);
        frag_shaders[CGPU_BACKEND_D3D12] = (const uint32_t*)triangle_frag_dxil;
        frag_shader_sizes[CGPU_BACKEND_D3D12] = sizeof(triangle_frag_dxil);
        compute_shaders[CGPU_BACKEND_D3D12] = (const uint32_t*)simple_compute_dxil;
        compute_shader_sizes[CGPU_BACKEND_D3D12] = sizeof(simple_compute_dxil);
    }

    ResourceCreation() SKR_NOEXCEPT
    {
        Initialize();
    }

    ~ResourceCreation() SKR_NOEXCEPT
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

    void test_all();

    CGPUInstanceId instance;
    CGPUAdapterId adapter;
    CGPUDeviceId device;
    const uint32_t* vertex_shaders[ECGPUBackend::CGPU_BACKEND_COUNT];
    uint32_t vertex_shader_sizes[ECGPUBackend::CGPU_BACKEND_COUNT];
    const uint32_t* frag_shaders[ECGPUBackend::CGPU_BACKEND_COUNT];
    uint32_t frag_shader_sizes[ECGPUBackend::CGPU_BACKEND_COUNT];
    const uint32_t* compute_shaders[ECGPUBackend::CGPU_BACKEND_COUNT];
    uint32_t compute_shader_sizes[ECGPUBackend::CGPU_BACKEND_COUNT];
};

template <ECGPUBackend backend>
void ResourceCreation<backend>::test_all()
    {
    SUBCASE("CreateDStorageQueue")
    {
        SKR_DECLARE_ZERO(CGPUDStorageQueueDescriptor, desc)
        desc.name = u8"DStorageQueue";
        desc.capacity = 1024;
        desc.priority = SKR_DSTORAGE_PRIORITY_NORMAL;
        desc.source = SKR_DSTORAGE_SOURCE_FILE;
    #ifdef _WIN32
        if (backend == CGPU_BACKEND_D3D12)
        {
            SkrDStorageConfig config = {};
            auto dstorageInstance = skr_create_dstorage_instance(&config);(void)dstorageInstance;
            const auto support = cgpu_query_dstorage_availability(device);
            EXPECT_EQ(support, SKR_DSTORAGE_AVAILABILITY_HARDWARE);
            auto dstorageQueue = cgpu_create_dstorage_queue(device, &desc);
            EXPECT_NE(dstorageQueue, nullptr);
            cgpu_free_dstorage_queue(dstorageQueue);
            skr_free_dstorage_instance(dstorageInstance);
        }
    #endif
    }

    SUBCASE("CreateIndexBuffer")
    {
        SKR_DECLARE_ZERO(CGPUBufferDescriptor, desc)
        desc.flags = CGPU_BCF_NONE;
        desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
        desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        desc.element_stride = sizeof(uint16_t);
        desc.element_count = 3;
        desc.size = sizeof(uint16_t) * 3;
        desc.name = u8"IndexBuffer";
        auto buffer = cgpu_create_buffer(device, &desc);
        EXPECT_NE(buffer, CGPU_NULLPTR);
        EXPECT_EQ(buffer->info->cpu_mapped_address, CGPU_NULLPTR);
        cgpu_free_buffer(buffer);
    }

    SUBCASE("CreateTexture")
    {
        SKR_DECLARE_ZERO(CGPUTextureDescriptor, desc)
        desc.name = u8"Texture";
        desc.flags = CGPU_TCF_DEDICATED_BIT;
        desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
        desc.start_state = CGPU_RESOURCE_STATE_COMMON;
        desc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
        desc.width = 512;
        desc.height = 512;
        desc.depth = 1;
        auto texture = cgpu_create_texture(device, &desc);
        EXPECT_NE(texture, CGPU_NULLPTR);
        cgpu_free_texture(texture);
    }

    SUBCASE("CreateTiledTexture")
    {
        if (backend == CGPU_BACKEND_D3D12)
        {
            SKR_DECLARE_ZERO(CGPUTextureDescriptor, desc)
            desc.name = u8"Texture";
            desc.flags = CGPU_TCF_DEDICATED_BIT | CGPU_TCF_TILED_RESOURCE;
            desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
            desc.start_state = CGPU_RESOURCE_STATE_COMMON;
            desc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
            desc.width = 512;
            desc.height = 512;
            desc.depth = 1;
            auto texture = cgpu_create_texture(device, &desc);
            EXPECT_NE(texture, CGPU_NULLPTR);
            cgpu_free_texture(texture);
        }
    }

    SUBCASE("CreateUploadBuffer")
    {
        SKR_DECLARE_ZERO(CGPUBufferDescriptor, desc)
        desc.flags = CGPU_BCF_NONE;
        desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER | CGPU_RESOURCE_TYPE_BUFFER;
        desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        desc.element_stride = sizeof(uint16_t);
        desc.element_count = 3;
        desc.size = sizeof(uint16_t) * 3;
        desc.name = u8"UploadBuffer";
        auto buffer = cgpu_create_buffer(device, &desc);
        EXPECT_NE(buffer, CGPU_NULLPTR);
        SKR_DECLARE_ZERO(CGPUBufferRange, range);
        range.offset = 0;
        range.size = desc.size;
        {
            cgpu_map_buffer(buffer, &range);
            uint16_t* indices = (uint16_t*)buffer->info->cpu_mapped_address;
            indices[0] = 2;
            indices[1] = 3;
            indices[2] = 3;
            cgpu_unmap_buffer(buffer);
        }
        {
            cgpu_map_buffer(buffer, &range);
            uint16_t* read_indices = (uint16_t*)buffer->info->cpu_mapped_address;
            EXPECT_EQ(read_indices[0], 2);
            EXPECT_EQ(read_indices[1], 3);
            EXPECT_EQ(read_indices[2], 3);
            cgpu_unmap_buffer(buffer);
        }
        cgpu_free_buffer(buffer);
    }

    SUBCASE("CreateUploadBufferPersistent")
    {
        SKR_DECLARE_ZERO(CGPUBufferDescriptor, desc)
        desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
        desc.descriptors = CGPU_RESOURCE_TYPE_BUFFER;
        desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        desc.element_stride = sizeof(uint16_t);
        desc.element_count = 3;
        desc.size = sizeof(uint16_t) * 3;
        desc.name = u8"UploadBuffer";
        auto buffer = cgpu_create_buffer(device, &desc);
        EXPECT_NE(buffer, CGPU_NULLPTR);
        EXPECT_NE(buffer->info->cpu_mapped_address, CGPU_NULLPTR);
        cgpu_free_buffer(buffer);
    }

    SUBCASE("CreateHostVisibleDeviceMemory")
    {
        SKR_DECLARE_ZERO(CGPUBufferDescriptor, desc)
        desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT | CGPU_BCF_HOST_VISIBLE;
        desc.descriptors = CGPU_RESOURCE_TYPE_BUFFER;
        desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        desc.element_stride = sizeof(uint16_t);
        desc.element_count = 3;
        desc.size = sizeof(uint16_t) * 3;
        desc.name = u8"UploadBuffer";
        auto buffer = cgpu_create_buffer(device, &desc);
        auto detail = cgpu_query_adapter_detail(device->adapter);
        if (detail->support_host_visible_vram)
        {
            EXPECT_NE(buffer, CGPU_NULLPTR);
            EXPECT_NE(buffer->info->cpu_mapped_address, CGPU_NULLPTR);
        }
        else
        {
            if (buffer)
            {
                EXPECT_EQ(buffer->info->cpu_mapped_address, CGPU_NULLPTR);
            }
        }
        cgpu_free_buffer(buffer);
    }

    SUBCASE("CreateConstantBufferX")
    {
        auto buffer = cgpux_create_mapped_constant_buffer(device,
            sizeof(uint16_t) * 3, u8"ConstantBuffer", true);
        EXPECT_NE(buffer, CGPU_NULLPTR);
        EXPECT_NE(buffer->info->cpu_mapped_address, CGPU_NULLPTR);
        cgpu_free_buffer(buffer);
    }

    SUBCASE("CreateModules")
    {
        SKR_DECLARE_ZERO(CGPUShaderLibraryDescriptor, vdesc)
        vdesc.code = vertex_shaders[backend];
        vdesc.code_size = vertex_shader_sizes[backend];
        vdesc.name = u8"VertexShaderLibrary";
        auto vertex_shader = cgpu_create_shader_library(device, &vdesc);

        SKR_DECLARE_ZERO(CGPUShaderLibraryDescriptor, fdesc)
        fdesc.code = frag_shaders[backend];
        fdesc.code_size = frag_shader_sizes[backend];
        fdesc.name = u8"FragmentShaderLibrary";
        auto fragment_shader = cgpu_create_shader_library(device, &fdesc);

        EXPECT_NE(vertex_shader, CGPU_NULLPTR);
        EXPECT_NE(fragment_shader, CGPU_NULLPTR);

        cgpu_free_shader_library(vertex_shader);
        cgpu_free_shader_library(fragment_shader);
    }

    SUBCASE("CreateRootSignature")
    {
        SKR_DECLARE_ZERO(CGPUShaderLibraryDescriptor, vdesc)
        vdesc.code = vertex_shaders[backend];
        vdesc.code_size = vertex_shader_sizes[backend];
        vdesc.name = u8"VertexShaderLibrary";
        auto vertex_shader = cgpu_create_shader_library(device, &vdesc);

        SKR_DECLARE_ZERO(CGPUShaderLibraryDescriptor, fdesc)
        fdesc.code = frag_shaders[backend];
        fdesc.code_size = frag_shader_sizes[backend];
        fdesc.name = u8"FragmentShaderLibrary";
        auto fragment_shader = cgpu_create_shader_library(device, &fdesc);

        CGPUShaderEntryDescriptor vertex_shader_entry = {};
        vertex_shader_entry.entry = u8"main";
        vertex_shader_entry.stage = ECGPUShaderStage::CGPU_SHADER_STAGE_TESE;
        vertex_shader_entry.library = vertex_shader;
        CGPUShaderEntryDescriptor fragment_shader_entry = {};
        fragment_shader_entry.entry = u8"main";
        fragment_shader_entry.stage = ECGPUShaderStage::CGPU_SHADER_STAGE_FRAG;
        fragment_shader_entry.library = fragment_shader;
        CGPUShaderEntryDescriptor shaders[] = { vertex_shader_entry, fragment_shader_entry };
        CGPURootSignatureDescriptor root_desc = {};
        root_desc.shaders = shaders;
        root_desc.shader_count = 2;
        auto signature = cgpu_create_root_signature(device, &root_desc);

        EXPECT_NE(signature, CGPU_NULLPTR);
        cgpu_free_root_signature(signature);

        cgpu_free_shader_library(vertex_shader);
        cgpu_free_shader_library(fragment_shader);
    }

    SUBCASE("CreateComputePipeline")
    {
        SKR_DECLARE_ZERO(CGPUShaderLibraryDescriptor, csdesc)
        csdesc.code = compute_shaders[backend];
        csdesc.code_size = compute_shader_sizes[backend];
        csdesc.name = u8"ComputeShaderLibrary";
        auto compute_shader = cgpu_create_shader_library(device, &csdesc);
        EXPECT_NE(compute_shader, CGPU_NULLPTR);

        // Create root signature
        SKR_DECLARE_ZERO(CGPUShaderEntryDescriptor, compute_shader_entry)
        compute_shader_entry.entry = u8"main";
        compute_shader_entry.stage = CGPU_SHADER_STAGE_COMPUTE;
        compute_shader_entry.library = compute_shader;
        SKR_DECLARE_ZERO(CGPURootSignatureDescriptor, root_desc)
        root_desc.shaders = &compute_shader_entry;
        root_desc.shader_count = 1;
        CGPURootSignatureId signature = cgpu_create_root_signature(device, &root_desc);
        EXPECT_NE(signature, CGPU_NULLPTR);

        cgpu_free_shader_library(compute_shader);
        cgpu_free_root_signature(signature);
    }

    SUBCASE("CreateRenderPipeline")
    {

    }
}

#ifdef CGPU_USE_D3D12
TEST_CASE_METHOD(ResourceCreation<CGPU_BACKEND_D3D12>, "ResourceCreation-d3d12")
{
    test_all();
}
#endif

#ifdef CGPU_USE_VULKAN
TEST_CASE_METHOD(ResourceCreation<CGPU_BACKEND_VULKAN>, "ResourceCreation-vulkan")
{
    test_all();
}
#endif