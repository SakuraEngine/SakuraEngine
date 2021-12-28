#include "cgpu/cgpu_config.h"
#include "cgpu/flags.h"
#include "platform/configure.h"
#include "gtest/gtest.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include "cgpu/api.h"
#include "spirv.h"
#include "dxil.h"

class ResourceCreation : public ::testing::TestWithParam<ECGpuBackend>
{
protected:
    void SetUp() override
    {
        ECGpuBackend backend = GetParam();
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

        CGpuQueueGroupDescriptor G = { QUEUE_TYPE_GRAPHICS, 1 };
        DECLARE_ZERO(CGpuDeviceDescriptor, descriptor)
        descriptor.queueGroups = &G;
        descriptor.queueGroupCount = 1;
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

    CGpuInstanceId instance;
    CGpuAdapterId adapter;
    CGpuDeviceId device;
    const uint32_t* vertex_shaders[ECGpuBackend::CGPU_BACKEND_COUNT];
    uint32_t vertex_shader_sizes[ECGpuBackend::CGPU_BACKEND_COUNT];
    const uint32_t* frag_shaders[ECGpuBackend::CGPU_BACKEND_COUNT];
    uint32_t frag_shader_sizes[ECGpuBackend::CGPU_BACKEND_COUNT];
    const uint32_t* compute_shaders[ECGpuBackend::CGPU_BACKEND_COUNT];
    uint32_t compute_shader_sizes[ECGpuBackend::CGPU_BACKEND_COUNT];
};

TEST_P(ResourceCreation, CreateIndexBuffer)
{
    DECLARE_ZERO(CGpuBufferDescriptor, desc)
    desc.flags = BCF_OWN_MEMORY_BIT;
    desc.descriptors = RT_INDEX_BUFFER;
    desc.memory_usage = MEM_USAGE_GPU_ONLY;
    desc.element_stride = sizeof(uint16_t);
    desc.elemet_count = 3;
    desc.size = sizeof(uint16_t) * 3;
    desc.name = "IndexBuffer";
    auto buffer = cgpu_create_buffer(device, &desc);
    EXPECT_NE(buffer, CGPU_NULLPTR);
    EXPECT_EQ(buffer->cpu_mapped_address, CGPU_NULLPTR);
    cgpu_free_buffer(buffer);
}

TEST_P(ResourceCreation, CreateTexture)
{
    DECLARE_ZERO(CGpuTextureDescriptor, desc)
    desc.name = "Texture";
    desc.flags = TCF_OWN_MEMORY_BIT;
    desc.format = PF_R8G8B8A8_UNORM;
    desc.start_state = RESOURCE_STATE_COMMON;
    desc.descriptors = RT_TEXTURE;
    desc.width = 512;
    desc.height = 512;
    desc.depth = 1;
    auto texture = cgpu_create_texture(device, &desc);
    EXPECT_NE(texture, CGPU_NULLPTR);
    cgpu_free_texture(texture);
}

TEST_P(ResourceCreation, CreateUploadBuffer)
{
    DECLARE_ZERO(CGpuBufferDescriptor, desc)
    desc.flags = BCF_OWN_MEMORY_BIT;
    desc.descriptors = RT_INDEX_BUFFER | RT_BUFFER;
    desc.memory_usage = MEM_USAGE_CPU_TO_GPU;
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
    desc.memory_usage = MEM_USAGE_CPU_TO_GPU;
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
    ECGpuBackend backend = GetParam();
    DECLARE_ZERO(CGpuShaderLibraryDescriptor, vdesc)
    vdesc.code = vertex_shaders[backend];
    vdesc.code_size = vertex_shader_sizes[backend];
    vdesc.name = "VertexShaderLibrary";
    vdesc.stage = ECGpuShaderStage::SHADER_STAGE_VERT;
    auto vertex_shader = cgpu_create_shader_library(device, &vdesc);

    DECLARE_ZERO(CGpuShaderLibraryDescriptor, fdesc)
    fdesc.code = frag_shaders[backend];
    fdesc.code_size = frag_shader_sizes[backend];
    fdesc.name = "FragmentShaderLibrary";
    fdesc.stage = ECGpuShaderStage::SHADER_STAGE_FRAG;
    auto fragment_shader = cgpu_create_shader_library(device, &fdesc);

    eastl::string cbName = fragment_shader->entry_reflections[0].shader_resources[0].name;
    EXPECT_EQ(cbName, "perDrawCBuffer");

    EXPECT_NE(vertex_shader, CGPU_NULLPTR);
    EXPECT_NE(fragment_shader, CGPU_NULLPTR);

    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

TEST_P(ResourceCreation, CreateRootSignature)
{
    ECGpuBackend backend = GetParam();
    DECLARE_ZERO(CGpuShaderLibraryDescriptor, vdesc)
    vdesc.code = vertex_shaders[backend];
    vdesc.code_size = vertex_shader_sizes[backend];
    vdesc.name = "VertexShaderLibrary";
    vdesc.stage = ECGpuShaderStage::SHADER_STAGE_VERT;
    auto vertex_shader = cgpu_create_shader_library(device, &vdesc);

    DECLARE_ZERO(CGpuShaderLibraryDescriptor, fdesc)
    fdesc.code = frag_shaders[backend];
    fdesc.code_size = frag_shader_sizes[backend];
    fdesc.name = "FragmentShaderLibrary";
    fdesc.stage = ECGpuShaderStage::SHADER_STAGE_FRAG;
    auto fragment_shader = cgpu_create_shader_library(device, &fdesc);

    CGpuPipelineShaderDescriptor vertex_shader_entry = {};
    vertex_shader_entry.entry = "main";
    vertex_shader_entry.stage = ECGpuShaderStage::SHADER_STAGE_TESE;
    vertex_shader_entry.library = vertex_shader;
    CGpuPipelineShaderDescriptor fragment_shader_entry = {};
    fragment_shader_entry.entry = "main";
    fragment_shader_entry.stage = ECGpuShaderStage::SHADER_STAGE_FRAG;
    fragment_shader_entry.library = fragment_shader;
    CGpuPipelineShaderDescriptor shaders[] = { vertex_shader_entry, fragment_shader_entry };
    CGpuRootSignatureDescriptor root_desc = {};
    root_desc.shaders = shaders;
    root_desc.shaders_count = 2;
    auto signature = cgpu_create_root_signature(device, &root_desc);

    EXPECT_NE(signature, CGPU_NULLPTR);
    cgpu_free_root_signature(signature);

    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

TEST_P(ResourceCreation, CreateComputePipeline)
{
    // Create compute shader
    ECGpuBackend backend = GetParam();
    DECLARE_ZERO(CGpuShaderLibraryDescriptor, csdesc)
    csdesc.code = compute_shaders[backend];
    csdesc.code_size = compute_shader_sizes[backend];
    csdesc.name = "ComputeShaderLibrary";
    csdesc.stage = ECGpuShaderStage::SHADER_STAGE_COMPUTE;
    auto compute_shader = cgpu_create_shader_library(device, &csdesc);
    EXPECT_NE(compute_shader, CGPU_NULLPTR);

    // Create root signature
    DECLARE_ZERO(CGpuPipelineShaderDescriptor, compute_shader_entry)
    compute_shader_entry.entry = "main";
    compute_shader_entry.stage = SHADER_STAGE_COMPUTE;
    compute_shader_entry.library = compute_shader;
    DECLARE_ZERO(CGpuRootSignatureDescriptor, root_desc)
    root_desc.shaders = &compute_shader_entry;
    root_desc.shaders_count = 1;
    CGpuRootSignatureId signature = cgpu_create_root_signature(device, &root_desc);
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