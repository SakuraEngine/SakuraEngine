#include "cgpu/cgpu_config.h"
#include "math.h"
#include "lodepng.h"
#include "platform/configure.h"
#include "cgpu/api.h"
#include "stdio.h"
#include <stdint.h>

inline static void read_bytes(const char* file_name, char8_t** bytes, uint32_t* length)
{
    FILE* f = fopen(file_name, "r");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (char8_t*)malloc(*length + 1);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void read_shader_bytes(
    const char* virtual_path, uint32_t** bytes, uint32_t* length,
    ECGpuBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../Resources/shaders/";
    strcpy(shader_file, shader_path);
    strcat(shader_file, virtual_path);
    switch (backend)
    {
        case CGPU_BACKEND_VULKAN:
            strcat(shader_file, ".spv");
            break;
        case CGPU_BACKEND_D3D12:
        case CGPU_BACKEND_XBOX_D3D12:
            strcat(shader_file, ".dxil");
            break;
    }
    read_bytes(shader_file, (char8_t**)bytes, length);
}

#define MANDELBROT_WIDTH 3200
#define MANDELBROT_HEIGHT 2400
static const char8_t* gPNGNames[CGPU_BACKEND_COUNT] = {
    "mandelbrot-vulkan.png",
    "mandelbrot-d3d12.png",
    "mandelbrot-d3d12(xbox).png",
    "mandelbrot-agc.png",
    "mandelbrot-metal.png"
};

typedef struct Pixel {
    float r, g, b, a;
} Pixel;

void ComputeFunc(void* usrdata)
{
    ECGpuBackend backend = *(ECGpuBackend*)usrdata;
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
        .queueType = QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    CGpuDeviceDescriptor device_desc = {
        .queueGroups = &G,
        .queueGroupCount = 1
    };
    CGpuDeviceId device = cgpu_create_device(adapter, &device_desc);

    // Create compute shader
    uint32_t *shader_bytes, shader_length;
    read_shader_bytes("cgpu-mandelbrot/mandelbrot",
        &shader_bytes, &shader_length, backend);
    CGpuShaderLibraryDescriptor shader_desc = {
        .code = shader_bytes,
        .code_size = shader_length,
        .name = "ComputeShaderLibrary",
        .stage = SHADER_STAGE_COMPUTE
    };
    CGpuShaderLibraryId compute_shader = cgpu_create_shader_library(device, &shader_desc);
    free(shader_bytes);

    // Create root signature
    CGpuPipelineShaderDescriptor compute_shader_entry = {
        .entry = "main",
        .stage = SHADER_STAGE_COMPUTE,
        .library = compute_shader
    };
    CGpuShaderReflection* entry_reflection = &compute_shader->entry_reflections[0];
    CGpuRootSignatureDescriptor root_desc = {
        .shaders = &compute_shader_entry,
        .shader_count = 1
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
        .start_state = RESOURCE_STATE_UNORDERED_ACCESS,
        .memory_usage = MEM_USAGE_GPU_ONLY,
        .element_stride = sizeof(Pixel),
        .elemet_count = MANDELBROT_WIDTH * MANDELBROT_HEIGHT,
        .size = sizeof(Pixel) * MANDELBROT_WIDTH * MANDELBROT_HEIGHT
    };
    CGpuBufferId data_buffer = cgpu_create_buffer(device, &buffer_desc);

    // Create readback buffer
    CGpuBufferDescriptor rb_desc = {
        .name = "ReadbackBuffer",
        .flags = BCF_OWN_MEMORY_BIT,
        .descriptors = RT_NONE,
        .start_state = RESOURCE_STATE_COPY_DEST,
        .memory_usage = MEM_USAGE_GPU_TO_CPU,
        .element_stride = buffer_desc.element_stride,
        .elemet_count = buffer_desc.elemet_count,
        .size = buffer_desc.size
    };
    CGpuBufferId readback_buffer = cgpu_create_buffer(device, &rb_desc);

    // Update descriptor set
    CGpuDescriptorData descriptor_data = {
        .name = "buf",
        .buffers = &data_buffer,
        .count = 1
    };
    cgpu_update_descriptor_set(set, &descriptor_data, 1);

    // Create command objects
    CGpuQueueId gfx_queue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
    CGpuCommandPoolId pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
    CGpuCommandBufferDescriptor cmd_desc = { .is_secondary = false };
    CGpuCommandBufferId cmd = cgpu_create_command_buffer(pool, &cmd_desc);

    // Dispatch
    {
        cgpu_cmd_begin(cmd);
        // Begin dispatch compute pass
        CGpuComputePassDescriptor pass_desc = { .name = "ComputePass" };
        CGpuComputePassEncoderId encoder = cgpu_cmd_begin_compute_pass(cmd, &pass_desc);
        cgpu_compute_encoder_bind_pipeline(encoder, pipeline);
        cgpu_compute_encoder_bind_descriptor_set(encoder, set);
        cgpu_compute_encoder_dispatch(encoder,
            (uint32_t)ceil(MANDELBROT_WIDTH / (float)entry_reflection->thread_group_sizes[0]),
            (uint32_t)ceil(MANDELBROT_HEIGHT / (float)entry_reflection->thread_group_sizes[1]),
            1);
        cgpu_cmd_end_compute_pass(cmd, encoder);
        // Barrier UAV buffer to transfer source
        CGpuBufferBarrier buffer_barrier = {
            .buffer = data_buffer,
            .src_state = RESOURCE_STATE_UNORDERED_ACCESS,
            .dst_state = RESOURCE_STATE_COPY_SOURCE
        };
        CGpuResourceBarrierDescriptor barriers_desc = {
            .buffer_barriers = &buffer_barrier,
            .buffer_barriers_count = 1
        };
        cgpu_cmd_resource_barrier(cmd, &barriers_desc);
        // Copy buffer to readback
        CGpuBufferToBufferTransfer cpy_desc = {
            .src = data_buffer,
            .src_offset = 0,
            .dst = readback_buffer,
            .dst_offset = 0,
            .size = buffer_desc.size
        };
        cgpu_cmd_transfer_buffer_to_buffer(cmd, &cpy_desc);
        cgpu_cmd_end(cmd);
        CGpuQueueSubmitDescriptor submit_desc = {
            .cmds = &cmd,
            .cmds_count = 1
        };
        cgpu_submit_queue(gfx_queue, &submit_desc);
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
            image[i * 4] = (uint8_t)(255.0f * mapped_memory[i].r);
            image[i * 4 + 1] = (uint8_t)(255.0f * mapped_memory[i].g);
            image[i * 4 + 2] = (uint8_t)(255.0f * mapped_memory[i].b);
            image[i * 4 + 3] = (uint8_t)(255.0f * mapped_memory[i].a);
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
    sakura_free(image);
}

#include "platform/thread.h"

int main(void)
{
    // When we support more add them here
    ECGpuBackend backends[] = {
        CGPU_BACKEND_VULKAN
#ifdef CGPU_USE_D3D12
        ,
        CGPU_BACKEND_D3D12
#endif
    };
    const uint32_t CGPU_BACKEND_COUNT = sizeof(backends) / sizeof(ECGpuBackend);
    DECLARE_ZERO_VLA(SThreadHandle, hdls, CGPU_BACKEND_COUNT)
    DECLARE_ZERO_VLA(SThreadDesc, thread_descs, CGPU_BACKEND_COUNT)
    for (uint32_t i = 0; i < CGPU_BACKEND_COUNT; i++)
    {
        thread_descs[i].pFunc = &ComputeFunc;
        thread_descs[i].pData = &backends[i];
        skr_init_thread(&thread_descs[i], &hdls[i]);
    }
    for (uint32_t i = 0; i < CGPU_BACKEND_COUNT; i++)
    {
        skr_join_thread(hdls[i]);
        skr_destroy_thread(hdls[i]);
    }
    return 0;
}