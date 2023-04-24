#include "cgpu/cgpu_config.h"
#include "cgpu/flags.h"
#include "platform/configure.h"
#include "gtest/gtest.h"
#include <containers/string.hpp>
#include <EASTL/vector.h>
#include "cgpu/api.h"
#include "spirv.h"
#include "dxil.h"

class ResourceCreation : public ::testing::TestWithParam<ECGPUBackend>
{
protected:
    void SetUp() override
    {
        ECGPUBackend backend = GetParam();
        DECLARE_ZERO(CGPUInstanceDescriptor, desc)
        desc.backend = backend;
        desc.enable_debug_layer = true;
        desc.enable_gpu_based_validation = true;
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
        DECLARE_ZERO(CGPUDeviceDescriptor, descriptor)
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

    void TearDown() override
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

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

TEST_P(ResourceCreation, CreateDStorageQueue)
{
    DECLARE_ZERO(CGPUDStorageQueueDescriptor, desc)
    desc.name = "DStorageQueue";
    desc.capacity = 1024;
    desc.priority = CGPU_DSTORAGE_PRIORITY_NORMAL;
    desc.source = CGPU_DSTORAGE_SOURCE_FILE;
#ifdef _WIN32
    if (GetParam() == CGPU_BACKEND_D3D12)
    {
        const auto support = cgpu_query_dstorage_availability(device);
        EXPECT_EQ(support, CGPU_DSTORAGE_AVAILABILITY_HARDWARE);
        auto dstorageQueue = cgpu_create_dstorage_queue(device, &desc);
        EXPECT_NE(dstorageQueue, nullptr);
        cgpu_free_dstorage_queue(dstorageQueue);
    }
#endif
}

TEST_P(ResourceCreation, CreateIndexBuffer)
{
    DECLARE_ZERO(CGPUBufferDescriptor, desc)
    desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
    desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = u8"IndexBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    EXPECT_EQ(buffer->cpu_mapped_address, CGPU_NULLPTR);
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateTexture)
{
    DECLARE_ZERO(CGPUTextureDescriptor, desc)
    desc.name = u8"Texture";
    desc.flags = CGPU_TCF_OWN_MEMORY_BIT;
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

TEST_P(ResourceCreation, CreateUploadBuffer)
{
    DECLARE_ZERO(CGPUBufferDescriptor, desc)
    desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
    desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER | CGPU_RESOURCE_TYPE_BUFFER;
    desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = u8"UploadBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    DECLARE_ZERO(CGPUBufferRange, range);
    range.offset = 0;
    range.size = desc.size;
    {
        cgpu_map_buffer(buffer, &range);
        uint16_t* indices = (uint16_t*)buffer->cpu_mapped_address;
        indices[0] = 2;
        indices[1] = 3;
        indices[2] = 3;
        cgpu_unmap_buffer(buffer);
    }
    {
        cgpu_map_buffer(buffer, &range);
        uint16_t* read_indices = (uint16_t*)buffer->cpu_mapped_address;
        EXPECT_EQ(read_indices[0], 2);
        EXPECT_EQ(read_indices[1], 3);
        EXPECT_EQ(read_indices[2], 3);
        cgpu_unmap_buffer(buffer);
    }
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateUploadBufferPersistent)
{
    DECLARE_ZERO(CGPUBufferDescriptor, desc)
    desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
    desc.descriptors = CGPU_RESOURCE_TYPE_BUFFER;
    desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = u8"UploadBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    EXPECT_NE(buffer->cpu_mapped_address, CGPU_NULLPTR);
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateHostVisibleDeviceMemory)
{
    DECLARE_ZERO(CGPUBufferDescriptor, desc)
    desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT | CGPU_BCF_HOST_VISIBLE;
    desc.descriptors = CGPU_RESOURCE_TYPE_BUFFER;
    desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = u8"UploadBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    auto detail = cgpu_query_adapter_detail(device->adapter);
    if (detail->support_host_visible_vram)
    {
        EXPECT_NE(buffer, CGPU_NULLPTR);
        EXPECT_NE(buffer->cpu_mapped_address, CGPU_NULLPTR);
    }
    else
    {
        if (buffer)
        {
            EXPECT_EQ(buffer->cpu_mapped_address, CGPU_NULLPTR);
        }
    }
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateConstantBufferX)
{
    auto buffer = cgpux_create_mapped_constant_buffer(device,
        sizeof(uint16_t) * 3, u8"ConstantBuffer", true);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    EXPECT_NE(buffer->cpu_mapped_address, CGPU_NULLPTR);
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateModules)
{
    ECGPUBackend backend = GetParam();
    DECLARE_ZERO(CGPUShaderLibraryDescriptor, vdesc)
    vdesc.code = vertex_shaders[backend];
    vdesc.code_size = vertex_shader_sizes[backend];
    vdesc.name = u8"VertexShaderLibrary";
    vdesc.stage = ECGPUShaderStage::CGPU_SHADER_STAGE_VERT;
    auto vertex_shader = cgpu_create_shader_library(device, &vdesc);

    DECLARE_ZERO(CGPUShaderLibraryDescriptor, fdesc)
    fdesc.code = frag_shaders[backend];
    fdesc.code_size = frag_shader_sizes[backend];
    fdesc.name = u8"FragmentShaderLibrary";
    fdesc.stage = ECGPUShaderStage::CGPU_SHADER_STAGE_FRAG;
    auto fragment_shader = cgpu_create_shader_library(device, &fdesc);

    skr::string cbName = (const char*)fragment_shader->entry_reflections[0].shader_resources[0].name;
    EXPECT_EQ(cbName, "perDrawCBuffer");

    EXPECT_NE(vertex_shader, CGPU_NULLPTR);
    EXPECT_NE(fragment_shader, CGPU_NULLPTR);

    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

TEST_P(ResourceCreation, CreateRootSignature)
{
    ECGPUBackend backend = GetParam();
    DECLARE_ZERO(CGPUShaderLibraryDescriptor, vdesc)
    vdesc.code = vertex_shaders[backend];
    vdesc.code_size = vertex_shader_sizes[backend];
    vdesc.name = u8"VertexShaderLibrary";
    vdesc.stage = ECGPUShaderStage::CGPU_SHADER_STAGE_VERT;
    auto vertex_shader = cgpu_create_shader_library(device, &vdesc);

    DECLARE_ZERO(CGPUShaderLibraryDescriptor, fdesc)
    fdesc.code = frag_shaders[backend];
    fdesc.code_size = frag_shader_sizes[backend];
    fdesc.name = u8"FragmentShaderLibrary";
    fdesc.stage = ECGPUShaderStage::CGPU_SHADER_STAGE_FRAG;
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

TEST_P(ResourceCreation, CreateComputePipeline)
{
    // Create compute shader
    ECGPUBackend backend = GetParam();
    DECLARE_ZERO(CGPUShaderLibraryDescriptor, csdesc)
    csdesc.code = compute_shaders[backend];
    csdesc.code_size = compute_shader_sizes[backend];
    csdesc.name = u8"ComputeShaderLibrary";
    csdesc.stage = ECGPUShaderStage::CGPU_SHADER_STAGE_COMPUTE;
    auto compute_shader = cgpu_create_shader_library(device, &csdesc);
    EXPECT_NE(compute_shader, CGPU_NULLPTR);

    // Create root signature
    DECLARE_ZERO(CGPUShaderEntryDescriptor, compute_shader_entry)
    compute_shader_entry.entry = u8"main";
    compute_shader_entry.stage = CGPU_SHADER_STAGE_COMPUTE;
    compute_shader_entry.library = compute_shader;
    DECLARE_ZERO(CGPURootSignatureDescriptor, root_desc)
    root_desc.shaders = &compute_shader_entry;
    root_desc.shader_count = 1;
    CGPURootSignatureId signature = cgpu_create_root_signature(device, &root_desc);
    EXPECT_NE(signature, CGPU_NULLPTR);

    cgpu_free_shader_library(compute_shader);
    cgpu_free_root_signature(signature);
}

TEST_P(ResourceCreation, CreateRenderPipeline)
{
}

static const auto allPlatforms = testing::Values(
#ifdef CGPU_USE_VULKAN
CGPU_BACKEND_VULKAN
#endif
#ifdef CGPU_USE_D3D12
,
CGPU_BACKEND_D3D12
#endif
);

INSTANTIATE_TEST_SUITE_P(ResourceCreation, ResourceCreation, allPlatforms);

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}