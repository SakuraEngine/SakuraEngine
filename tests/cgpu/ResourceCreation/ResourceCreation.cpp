#define RUNTIME_DLL
#include "gtest/gtest.h"
#include <EASTL/string.h>
#include "cgpu/api.h"
#include "spirv.h"
#include "dxil.h"

class ResourceCreation : public ::testing::TestWithParam<ECGPUBackEnd>
{
protected:
    void SetUp() override
    {
        ECGPUBackEnd backend = GetParam();
        DECLARE_ZERO(CGpuInstanceDescriptor, desc)
        desc.backend = backend;
        desc.enable_debug_layer = true;
        desc.enable_gpu_based_validation = true;
        desc.enable_set_name = true;
        instance = cgpu_create_instance(&desc);

        EXPECT_NE(instance, CGPU_NULLPTR);
        EXPECT_NE(instance, nullptr);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, nullptr, &adapters_count);
        std::vector<CGpuAdapterId> adapters;
        adapters.resize(adapters_count);
        cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
        adapter = adapters[0];

        CGpuQueueGroupDescriptor G = { ECGpuQueueType_Graphics, 1 };
        DECLARE_ZERO(CGpuDeviceDescriptor, descriptor)
        descriptor.queueGroups = &G;
        descriptor.queueGroupCount = 1;
        device = cgpu_create_device(adapter, &descriptor);
        EXPECT_NE(device, nullptr);
        EXPECT_NE(device, CGPU_NULLPTR);

        vertex_shaders[ECGPUBackEnd_VULKAN] = (const uint32_t*)triangle_vert_spirv;
        vertex_shader_sizes[ECGPUBackEnd_VULKAN] = sizeof(triangle_vert_spirv);
        frag_shaders[ECGPUBackEnd_VULKAN] = (const uint32_t*)triangle_frag_spirv;
        frag_shader_sizes[ECGPUBackEnd_VULKAN] = sizeof(triangle_frag_spirv);

        vertex_shaders[ECGPUBackEnd_D3D12] = (const uint32_t*)triangle_vert_dxil;
        vertex_shader_sizes[ECGPUBackEnd_D3D12] = sizeof(triangle_vert_dxil);
        frag_shaders[ECGPUBackEnd_D3D12] = (const uint32_t*)triangle_frag_dxil;
        frag_shader_sizes[ECGPUBackEnd_D3D12] = sizeof(triangle_frag_dxil);
    }

    void TearDown() override
    {
        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }

    CGpuInstanceId instance;
    CGpuAdapterId adapter;
    CGpuDeviceId device;
    const uint32_t* vertex_shaders[ECGPUBackEnd::ECGPUBackEnd_COUNT];
    uint32_t vertex_shader_sizes[ECGPUBackEnd::ECGPUBackEnd_COUNT];
    const uint32_t* frag_shaders[ECGPUBackEnd::ECGPUBackEnd_COUNT];
    uint32_t frag_shader_sizes[ECGPUBackEnd::ECGPUBackEnd_COUNT];
};

TEST_P(ResourceCreation, CreateIndexBuffer)
{
    DECLARE_ZERO(CGpuBufferDescriptor, desc)
    desc.flags = BCF_OWN_MEMORY_BIT;
    desc.descriptors = RT_INDEX_BUFFER;
    desc.memory_usage = MU_GPU_ONLY;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = "IndexBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    EXPECT_EQ(buffer->cpu_mapped_address, CGPU_NULLPTR);
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateUploadBuffer)
{
    DECLARE_ZERO(CGpuBufferDescriptor, desc)
    desc.flags = BCF_OWN_MEMORY_BIT;
    desc.descriptors = RT_INDEX_BUFFER | RT_BUFFER;
    desc.memory_usage = MU_CPU_TO_GPU;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = "UploadBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    DECLARE_ZERO(CGpuBufferRange, range);
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
    DECLARE_ZERO(CGpuBufferDescriptor, desc)
    desc.flags = BCF_PERSISTENT_MAP_BIT;
    desc.descriptors = RT_BUFFER;
    desc.memory_usage = MU_CPU_TO_GPU;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = "UploadBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    EXPECT_NE(buffer->cpu_mapped_address, CGPU_NULLPTR);
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateModules)
{
    ECGPUBackEnd backend = GetParam();
    DECLARE_ZERO(CGpuShaderLibraryDescriptor, vdesc)
    vdesc.code = vertex_shaders[backend];
    vdesc.code_size = vertex_shader_sizes[backend];
    vdesc.name = "VertexShaderLibrary";
    vdesc.stage = ECGpuShaderStage::SS_VERT;
    auto vertex_shader = cgpu_create_shader_library(device, &vdesc);

    DECLARE_ZERO(CGpuShaderLibraryDescriptor, fdesc)
    fdesc.code = frag_shaders[backend];
    fdesc.code_size = frag_shader_sizes[backend];
    fdesc.name = "FragmentShaderLibrary";
    fdesc.stage = ECGpuShaderStage::SS_FRAG;
    auto fragment_shader = cgpu_create_shader_library(device, &fdesc);

    eastl::string cbName = vertex_shader->entry_reflections[0].shader_resources[0].name;
    EXPECT_EQ(cbName, "AssholeDXC");

    EXPECT_NE(vertex_shader, CGPU_NULLPTR);
    EXPECT_NE(fragment_shader, CGPU_NULLPTR);

    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

static const auto allPlatforms = testing::Values(
#ifdef CGPU_USE_VULKAN
    ECGPUBackEnd_VULKAN
#endif
#ifdef CGPU_USE_D3D12
    ,
    ECGPUBackEnd_D3D12
#endif
);

INSTANTIATE_TEST_SUITE_P(ResourceCreation, ResourceCreation, allPlatforms);