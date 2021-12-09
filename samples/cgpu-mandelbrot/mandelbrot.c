#include "cgpu/cgpu_config.h"
#include "math.h"
#include "embed-shaders.h"
#include "lodepng.h"
#include "platform/configure.h"
#include "cgpu/api.h"

const uint32_t* compute_shaders[ECGPUBackEnd_COUNT];
uint32_t compute_shader_sizes[ECGPUBackEnd_COUNT];
static const char8_t* gPNGNames[ECGPUBackEnd_COUNT] = {
    "mandelbrot-vulkan.png",
    "mandelbrot-d3d12.png",
    "mandelbrot-d3d12(xbox).png",
    "mandelbrot-agc.png",
    "mandelbrot-metal.png"
};

typedef struct Pixel {
    float r, g, b, a;
} Pixel;

int main(void)
{
    compute_shaders[ECGPUBackEnd_VULKAN] = (const uint32_t*)mandelbrot_comp_spirv;
    compute_shader_sizes[ECGPUBackEnd_VULKAN] = sizeof(mandelbrot_comp_spirv);

    compute_shaders[ECGPUBackEnd_D3D12] = (const uint32_t*)mandelbrot_comp_dxil;
    compute_shader_sizes[ECGPUBackEnd_D3D12] = sizeof(mandelbrot_comp_dxil);

    ECGPUBackEnd backend = ECGPUBackEnd_VULKAN;
    // When we support more add them here
    if (backend == ECGPUBackEnd_VULKAN)
    {
        // Create Device
        DECLARE_ZERO(CGpuInstanceDescriptor, desc)
        desc.backend = backend;
        desc.enable_debug_layer = true;
        desc.enable_gpu_based_validation = true;
        desc.enable_set_name = true;
        CGpuInstanceId instance = cgpu_create_instance(&desc);

        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
        DECLARE_ZERO_VLA(CGpuAdapterId, adapters, adapters_count);
        cgpu_enum_adapters(instance, adapters, &adapters_count);
        CGpuAdapterId adapter = adapters[0];

        CGpuQueueGroupDescriptor G = { ECGpuQueueType_Graphics, 1 };
        DECLARE_ZERO(CGpuDeviceDescriptor, descriptor)
        descriptor.queueGroups = &G;
        descriptor.queueGroupCount = 1;
        CGpuDeviceId device = cgpu_create_device(adapter, &descriptor);

        // Prepare compute pipeline
        DECLARE_ZERO(CGpuShaderLibraryDescriptor, cdesc)
        cdesc.code = compute_shaders[backend];
        cdesc.code_size = compute_shader_sizes[backend];
        cdesc.name = "ComputeShaderLibrary";
        cdesc.stage = SS_COMPUTE;
        CGpuShaderLibraryId compute_shader = cgpu_create_shader_library(device, &cdesc);

        DECLARE_ZERO(CGpuPipelineShaderDescriptor, compute_shader_entry)
        compute_shader_entry.entry = "main";
        compute_shader_entry.stage = SS_COMPUTE;
        compute_shader_entry.library = compute_shader;

        DECLARE_ZERO(CGpuRootSignatureDescriptor, root_desc)
        root_desc.shaders = &compute_shader_entry;
        root_desc.shaders_count = 1;
        CGpuRootSignatureId signature = cgpu_create_root_signature(device, &root_desc);

        DECLARE_ZERO(CGpuComputePipelineDescriptor, pipeline_desc)
        pipeline_desc.compute_shader = &compute_shader_entry;
        pipeline_desc.root_signature = signature;
        CGpuComputePipelineId pipeline = cgpu_create_compute_pipeline(device, &pipeline_desc);

        DECLARE_ZERO(CGpuDescriptorSetDescriptor, set_desc)
        set_desc.root_signature = signature;
        set_desc.set_index = 0;
        CGpuDescriptorSetId set = cgpu_create_descriptor_set(device, &set_desc);

        // Create data buffer
        DECLARE_ZERO(CGpuBufferDescriptor, buffer_desc)
        buffer_desc.flags = BCF_NONE;
        buffer_desc.descriptors = RT_RW_BUFFER;
        buffer_desc.memory_usage = MU_GPU_ONLY;
        buffer_desc.element_stride = sizeof(Pixel);
        buffer_desc.elemet_count = MANDELBROT_WIDTH * MANDELBROT_HEIGHT;
        buffer_desc.size = sizeof(Pixel) * MANDELBROT_WIDTH * MANDELBROT_HEIGHT;
        buffer_desc.name = "DataBuffer";
        CGpuBufferId data_buffer = cgpu_create_buffer(device, &buffer_desc);

        // Update descriptor set
        DECLARE_ZERO(CGpuDescriptorData, data)
        data.name = "buf";
        data.buffers = &data_buffer;
        data.count = 1;
        cgpu_update_descriptor_set(set, &data, 1);

        // Create command objects
        CGpuQueueId gfx_queue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
        CGpuCommandPoolId pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
        DECLARE_ZERO(CGpuCommandBufferDescriptor, cmd_desc);
        cmd_desc.is_secondary = false;
        CGpuCommandBufferId cmd = cgpu_create_command_buffer(pool, &cmd_desc);
        CGpuCommandBufferId cmd2 = cgpu_create_command_buffer(pool, &cmd_desc);

        // Dispatch
        {
            cgpu_cmd_begin(cmd);
            DECLARE_ZERO(CGpuComputePassDescriptor, pass_desc);
            pass_desc.name = "ComputePass";
            CGpuComputePassEncoderId encoder = cgpu_cmd_begin_compute_pass(cmd, &pass_desc);
            cgpu_compute_encoder_bind_descriptor_set(encoder, set);
            cgpu_compute_encoder_bind_pipeline(encoder, pipeline);
            cgpu_compute_encoder_dispatch(encoder,
                (uint32_t)ceil(MANDELBROT_WIDTH / (float)WORKGROUP_SIZE),
                (uint32_t)ceil(MANDELBROT_HEIGHT / (float)WORKGROUP_SIZE),
                1);
            cgpu_cmd_end_compute_pass(cmd, encoder);
            cgpu_cmd_end(cmd);
            DECLARE_ZERO(CGpuQueueSubmitDescriptor, submit_desc);
            submit_desc.cmds = &cmd;
            submit_desc.cmds_count = 1;
            cgpu_submit_queue(gfx_queue, &submit_desc);
            cgpu_wait_queue_idle(gfx_queue);
        }

        // Create readback buffer
        DECLARE_ZERO(CGpuBufferDescriptor, rb_desc)
        rb_desc.flags = BCF_OWN_MEMORY_BIT;
        rb_desc.descriptors = RT_NONE;
        rb_desc.start_state = RS_COPY_DEST;
        rb_desc.memory_usage = MU_GPU_TO_CPU;
        rb_desc.element_stride = buffer_desc.element_stride;
        rb_desc.elemet_count = buffer_desc.elemet_count;
        rb_desc.size = buffer_desc.size;
        rb_desc.name = "ReadbackBuffer";
        CGpuBufferId readback_buffer = cgpu_create_buffer(device, &rb_desc);

        // Copy back
        {
            cgpu_cmd_begin(cmd2);
            DECLARE_ZERO(CGpuBufferUpdateDescriptor, cpy_desc);
            cpy_desc.src = data_buffer;
            cpy_desc.src_offset = 0;
            cpy_desc.dst = readback_buffer;
            cpy_desc.dst_offset = 0;
            cpy_desc.size = buffer_desc.size;
            cgpu_cmd_update_buffer(cmd2, &cpy_desc);
            cgpu_cmd_end(cmd2);
            CGpuQueueSubmitDescriptor submit_desc2 = {};
            submit_desc2.cmds = &cmd2;
            submit_desc2.cmds_count = 1;
            cgpu_submit_queue(gfx_queue, &submit_desc2);
            cgpu_wait_queue_idle(gfx_queue);
        }

        // Map buffer and readback
        unsigned char* image;
        {
            DECLARE_ZERO(CGpuBufferRange, range);
            range.offset = 0;
            range.size = buffer_desc.size;
            cgpu_map_buffer(readback_buffer, &range);
            Pixel* mapped_memory = (Pixel*)readback_buffer->cpu_mapped_address;
            image = sakura_malloc(MANDELBROT_WIDTH * MANDELBROT_HEIGHT * 4);
            for (int i = 0; i < MANDELBROT_WIDTH * MANDELBROT_HEIGHT; i += 1)
            {
                image[i * 4] = 255.0f * mapped_memory[i].r;
                image[i * 4 + 1] = 255.0f * mapped_memory[i].g;
                image[i * 4 + 2] = 255.0f * mapped_memory[i].b;
                image[i * 4 + 3] = 255.0f * mapped_memory[i].a;
            }
            cgpu_unmap_buffer(readback_buffer);
        }

        // Now we save the acquired color data to a .png.
        {
            unsigned error = lodepng_encode32_file(gPNGNames[backend], image, MANDELBROT_WIDTH, MANDELBROT_HEIGHT);
            if (error)
                printf("encoder error %d: %s", error, lodepng_error_text(error));
        }

        // Clean up
        cgpu_free_command_buffer(cmd);
        cgpu_free_command_buffer(cmd2);
        cgpu_free_command_pool(pool);
        cgpu_free_buffer(data_buffer);
        cgpu_free_buffer(readback_buffer);
        cgpu_free_queue(gfx_queue);
        cgpu_free_descriptor_set(set);
        cgpu_free_shader_library(compute_shader);
        cgpu_free_root_signature(signature);
        cgpu_free_compute_pipeline(pipeline);

        cgpu_free_device(device);
        cgpu_free_instance(instance);
    }
}