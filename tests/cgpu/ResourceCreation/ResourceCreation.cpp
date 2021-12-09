#include "cgpu/cgpu_config.h"
#include "cgpu/flags.h"
#include "platform/configure.h"
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
        compute_shaders[ECGPUBackEnd_VULKAN] = (const uint32_t*)mandelbrot_comp_spirv;
        compute_shader_sizes[ECGPUBackEnd_VULKAN] = sizeof(mandelbrot_comp_spirv);

        vertex_shaders[ECGPUBackEnd_D3D12] = (const uint32_t*)triangle_vert_dxil;
        vertex_shader_sizes[ECGPUBackEnd_D3D12] = sizeof(triangle_vert_dxil);
        frag_shaders[ECGPUBackEnd_D3D12] = (const uint32_t*)triangle_frag_dxil;
        frag_shader_sizes[ECGPUBackEnd_D3D12] = sizeof(triangle_frag_dxil);
        compute_shaders[ECGPUBackEnd_D3D12] = (const uint32_t*)mandelbrot_comp_dxil;
        compute_shader_sizes[ECGPUBackEnd_D3D12] = sizeof(mandelbrot_comp_dxil);
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
    const uint32_t* compute_shaders[ECGPUBackEnd::ECGPUBackEnd_COUNT];
    uint32_t compute_shader_sizes[ECGPUBackEnd::ECGPUBackEnd_COUNT];
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

    eastl::string cbName = fragment_shader->entry_reflections[0].shader_resources[0].name;
    EXPECT_EQ(cbName, "perDrawCBuffer");

    EXPECT_NE(vertex_shader, CGPU_NULLPTR);
    EXPECT_NE(fragment_shader, CGPU_NULLPTR);

    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

TEST_P(ResourceCreation, CreateRootSignature)
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

    CGpuPipelineShaderDescriptor vertex_shader_entry = {};
    vertex_shader_entry.entry = "main";
    vertex_shader_entry.stage = ECGpuShaderStage::SS_TESE;
    vertex_shader_entry.library = vertex_shader;
    CGpuPipelineShaderDescriptor fragment_shader_entry = {};
    fragment_shader_entry.entry = "main";
    fragment_shader_entry.stage = ECGpuShaderStage::SS_FRAG;
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

struct Pixel {
    float r, g, b, a;
};

TEST_P(ResourceCreation, CreateComputePipeline)
{
    ECGPUBackEnd backend = GetParam();
    // When we support more add them here
    if (backend == ECGPUBackEnd::ECGPUBackEnd_VULKAN)
    {
        DECLARE_ZERO(CGpuShaderLibraryDescriptor, cdesc)
        cdesc.code = compute_shaders[backend];
        cdesc.code_size = compute_shader_sizes[backend];
        cdesc.name = "ComputeShaderLibrary";
        cdesc.stage = ECGpuShaderStage::SS_COMPUTE;
        auto compute_shader = cgpu_create_shader_library(device, &cdesc);
        EXPECT_NE(compute_shader, CGPU_NULLPTR);

        DECLARE_ZERO(CGpuPipelineShaderDescriptor, compute_shader_entry)
        compute_shader_entry.entry = "main";
        compute_shader_entry.stage = ECGpuShaderStage::SS_COMPUTE;
        compute_shader_entry.library = compute_shader;

        DECLARE_ZERO(CGpuRootSignatureDescriptor, root_desc)
        root_desc.shaders = &compute_shader_entry;
        root_desc.shaders_count = 1;
        auto signature = cgpu_create_root_signature(device, &root_desc);
        EXPECT_NE(signature, CGPU_NULLPTR);

        DECLARE_ZERO(CGpuComputePipelineDescriptor, pipeline_desc)
        pipeline_desc.compute_shader = &compute_shader_entry;
        pipeline_desc.root_signature = signature;
        auto pipeline = cgpu_create_compute_pipeline(device, &pipeline_desc);
        EXPECT_NE(pipeline, CGPU_NULLPTR);

        DECLARE_ZERO(CGpuDescriptorSetDescriptor, set_desc)
        set_desc.root_signature = signature;
        set_desc.set_index = 0;
        auto set = cgpu_create_descriptor_set(device, &set_desc);
        EXPECT_NE(set, CGPU_NULLPTR);

        // Create Buffer
        DECLARE_ZERO(CGpuBufferDescriptor, buffer_desc)
        buffer_desc.flags = BCF_NONE;
        buffer_desc.descriptors = RT_RW_BUFFER;
        buffer_desc.memory_usage = MU_GPU_ONLY;
        buffer_desc.element_stride = sizeof(Pixel);
        buffer_desc.elemet_count = MANDELBROT_WIDTH * MANDELBROT_HEIGHT;
        buffer_desc.size = sizeof(Pixel) * MANDELBROT_WIDTH * MANDELBROT_HEIGHT;
        buffer_desc.name = "DataBuffer";
        auto data_buffer = cgpu_create_buffer(device, &buffer_desc);

        // Update Descriptor Set
        DECLARE_ZERO(CGpuDescriptorData, data)
        data.name = "buf";
        data.buffers = &data_buffer;
        data.count = 1;
        cgpu_update_descriptor_set(set, &data, 1);

        // Dispatch
        auto gfx_queue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
        EXPECT_NE(gfx_queue, CGPU_NULLPTR);
        auto pool = cgpu_create_command_pool(gfx_queue, nullptr);

        DECLARE_ZERO(CGpuCommandBufferDescriptor, cmd_desc);
        cmd_desc.is_secondary = false;
        auto cmd = cgpu_create_command_buffer(pool, &cmd_desc);
        cgpu_cmd_begin(cmd);
        DECLARE_ZERO(CGpuComputePassDescriptor, pass_desc);
        pass_desc.name = "ComputePass";
        auto encoder = cgpu_cmd_begin_compute_pass(cmd, &pass_desc);
        cgpu_compute_encoder_bind_descriptor_set(encoder, set);
        cgpu_compute_encoder_bind_pipeline(encoder, pipeline);
        cgpu_compute_encoder_dispatch(encoder, 32, 32, 1);
        cgpu_cmd_end_compute_pass(cmd, encoder);
        cgpu_cmd_end(cmd);
        DECLARE_ZERO(CGpuQueueSubmitDescriptor, submit_desc);
        submit_desc.cmds = &cmd;
        submit_desc.cmds_count = 1;
        cgpu_submit_queue(gfx_queue, &submit_desc);
        cgpu_wait_queue_idle(gfx_queue);

        // clean up
        cgpu_free_command_buffer(cmd);
        cgpu_free_command_pool(pool);
        cgpu_free_buffer(data_buffer);
        cgpu_free_queue(gfx_queue);
        cgpu_free_descriptor_set(set);
        cgpu_free_shader_library(compute_shader);
        cgpu_free_root_signature(signature);
        cgpu_free_compute_pipeline(pipeline);
    }
}

static const auto allPlatforms = testing::Values(
#ifdef CGPU_USE_VULKAN
    ECGPUBackEnd_VULKAN
#endif
#ifdef CGPU_USE_D3D12
//,
// ECGPUBackEnd_D3D12
#endif
);

INSTANTIATE_TEST_SUITE_P(ResourceCreation, ResourceCreation, allPlatforms);