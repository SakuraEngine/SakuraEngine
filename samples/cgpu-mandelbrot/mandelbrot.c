#include "cgpu/cgpu_config.h"
#include "math.h"
#include "embed-shaders.h"
#include "lodepng.h"
#include "platform/configure.h"
#include "cgpu/api.h"

const uint32_t* compute_shaders[ECGpuBackend_COUNT];
uint32_t compute_shader_sizes[ECGpuBackend_COUNT];
static const char8_t* gPNGNames[ECGpuBackend_COUNT] = {
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
    compute_shaders[ECGpuBackend_VULKAN] = (const uint32_t*)mandelbrot_comp_spirv;
    compute_shader_sizes[ECGpuBackend_VULKAN] = sizeof(mandelbrot_comp_spirv);

    compute_shaders[ECGpuBackend_D3D12] = (const uint32_t*)mandelbrot_comp_dxil;
    compute_shader_sizes[ECGpuBackend_D3D12] = sizeof(mandelbrot_comp_dxil);

    ECGpuBackend backend = ECGpuBackend_VULKAN;
    // When we support more add them here
    if (backend == ECGpuBackend_VULKAN)
    {
        // Create instance
        CGpuInstanceDescriptor instance_desc = {
            .backend = backend,
            .enable_debug_layer = true,
            .enable_gpu_based_validation = true,
            .enable_set_name = true
        };
        CGpuInstanceId instance = cgpu_create_instance(&instance_desc);

        // Filter adapters
        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
        DECLARE_ZERO_VLA(CGpuAdapterId, adapters, adapters_count);
        cgpu_enum_adapters(instance, adapters, &adapters_count);
        CGpuAdapterId adapter = adapters[0];

        // Create device
        CGpuQueueGroupDescriptor G = {
            .queueType = ECGpuQueueType_Graphics,
            .queueCount = 1
        };
        CGpuDeviceDescriptor device_desc = {
            .queueGroups = &G,
            .queueGroupCount = 1
        };
        CGpuDeviceId device = cgpu_create_device(adapter, &device_desc);

        // Create compute shader
        CGpuShaderLibraryDescriptor shader_desc = {
            .code = compute_shaders[backend],
            .code_size = compute_shader_sizes[backend],
            .name = "ComputeShaderLibrary",
            .stage = SS_COMPUTE
        };
        CGpuShaderLibraryId compute_shader = cgpu_create_shader_library(device, &shader_desc);

        // Create root signature
        CGpuPipelineShaderDescriptor compute_shader_entry = {
            .entry = "main",
            .stage = SS_COMPUTE,
            .library = compute_shader
        };
        CGpuRootSignatureDescriptor root_desc = {
            .shaders = &compute_shader_entry,
            .shaders_count = 1
        };
        CGpuRootSignatureId signature = cgpu_create_root_signature(device, &root_desc);

        // Create compute pipeline
        CGpuComputePipelineDescriptor pipeline_desc = {
            .compute_shader = &compute_shader_entry,
            .root_signature = signature
        };
        CGpuComputePipelineId pipeline = cgpu_create_compute_pipeline(device, &pipeline_desc);

        CGpuDescriptorSetDescriptor set_desc = {
            .root_signature = signature,
            .set_index = 0
        };
        CGpuDescriptorSetId set = cgpu_create_descriptor_set(device, &set_desc);

        // Create data buffer
        CGpuBufferDescriptor buffer_desc = {
            .name = "DataBuffer",
            .flags = BCF_NONE,
            .descriptors = RT_RW_BUFFER,
            .memory_usage = MU_GPU_ONLY,
            .element_stride = sizeof(Pixel),
            .elemet_count = MANDELBROT_WIDTH * MANDELBROT_HEIGHT,
            .size = sizeof(Pixel) * MANDELBROT_WIDTH * MANDELBROT_HEIGHT
        };
        CGpuBufferId data_buffer = cgpu_create_buffer(device, &buffer_desc);

        // Update descriptor set
        CGpuDescriptorData descriptor_data = {
            .name = "buf",
            .buffers = &data_buffer,
            .count = 1
        };
        cgpu_update_descriptor_set(set, &descriptor_data, 1);

        // Create command objects
        CGpuQueueId gfx_queue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
        CGpuCommandPoolId pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
        CGpuCommandBufferDescriptor cmd_desc = { .is_secondary = false };
        CGpuCommandBufferId cmd = cgpu_create_command_buffer(pool, &cmd_desc);

        // Dispatch
        {
            cgpu_cmd_begin(cmd);
            CGpuComputePassDescriptor pass_desc = { .name = "ComputePass" };
            CGpuComputePassEncoderId encoder = cgpu_cmd_begin_compute_pass(cmd, &pass_desc);
            cgpu_compute_encoder_bind_descriptor_set(encoder, set);
            cgpu_compute_encoder_bind_pipeline(encoder, pipeline);
            cgpu_compute_encoder_dispatch(encoder,
                (uint32_t)ceil(MANDELBROT_WIDTH / (float)WORKGROUP_SIZE),
                (uint32_t)ceil(MANDELBROT_HEIGHT / (float)WORKGROUP_SIZE),
                1);
            cgpu_cmd_end_compute_pass(cmd, encoder);
            cgpu_cmd_end(cmd);
            CGpuQueueSubmitDescriptor submit_desc = {
                .cmds = &cmd,
                .cmds_count = 1
            };
            cgpu_submit_queue(gfx_queue, &submit_desc);
            cgpu_wait_queue_idle(gfx_queue);
        }
        cgpu_reset_command_pool(pool);

        // Create readback buffer
        CGpuBufferDescriptor rb_desc = {
            .name = "ReadbackBuffer",
            .flags = BCF_OWN_MEMORY_BIT,
            .descriptors = RT_NONE,
            .start_state = RS_COPY_DEST,
            .memory_usage = MU_GPU_TO_CPU,
            .element_stride = buffer_desc.element_stride,
            .elemet_count = buffer_desc.elemet_count,
            .size = buffer_desc.size
        };
        CGpuBufferId readback_buffer = cgpu_create_buffer(device, &rb_desc);

        // Copy back
        {
            cgpu_cmd_begin(cmd);
            CGpuBufferUpdateDescriptor cpy_desc = {
                .src = data_buffer,
                .src_offset = 0,
                .dst = readback_buffer,
                .dst_offset = 0,
                .size = buffer_desc.size
            };
            cgpu_cmd_update_buffer(cmd, &cpy_desc);
            cgpu_cmd_end(cmd);
            CGpuQueueSubmitDescriptor submit_desc2 = {
                .cmds = &cmd,
                .cmds_count = 1
            };
            cgpu_submit_queue(gfx_queue, &submit_desc2);
            cgpu_wait_queue_idle(gfx_queue);
        }

        // Map buffer and readback
        unsigned char* image;
        {
            CGpuBufferRange map_range = {
                .offset = 0,
                .size = buffer_desc.size
            };
            cgpu_map_buffer(readback_buffer, &map_range);
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