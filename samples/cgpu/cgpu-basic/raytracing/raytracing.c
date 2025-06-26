#include "SkrOS/thread.h"
#include "SkrGraphics/api.h"
#include "SkrGraphics/raytracing.h"
#include "lodepng.h"
#include "math.h"

inline static void read_bytes(const char* file_name, char8_t** bytes, uint32_t* length)
{
    FILE* f = fopen(file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (char8_t*)malloc(*length + 1);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

typedef struct SKR_ALIGNAS(16) skr_float4_tt {
    struct 
    {
        float x;
        float y;
        float z;
        float w;
    };
} skr_float4_tt;

inline static void read_shader_bytes(const char* virtual_path, uint32_t** bytes, uint32_t* length, ECGPUBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../resources/shaders/";
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
        case CGPU_BACKEND_METAL:
            strcat(shader_file, ".metallib");
            break;
        default:
            SKR_UNIMPLEMENTED_FUNCTION();
            break;
    }
    read_bytes(shader_file, (char8_t**)bytes, length);
}

#define RAYTRACING_WIDTH 3200
#define RAYTRACING_HEIGHT 2400
static const char8_t* gPNGNames[CGPU_BACKEND_COUNT] = {
    "raytracing-vulkan.png",
    "raytracing-d3d12.png",
    "raytracing-d3d12(xbox).png",
    "raytracing-agc.png",
    "raytracing-metal.png"
};

typedef struct Pixel {
    float r, g, b, a;
} Pixel;


void ComputeFunc(void* usrdata)
{
    ECGPUBackend backend = *(ECGPUBackend*)usrdata;
    // Create instance
    CGPUInstanceDescriptor instance_desc = {
        .backend = backend,
        .enable_debug_layer = true,
        .enable_gpu_based_validation = true,
        .enable_set_name = true
    };
    CGPUInstanceId instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    SKR_DECLARE_ZERO_VLA(CGPUAdapterId, adapters, adapters_count);
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    CGPUAdapterId adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor G = {
        .queue_type = CGPU_QUEUE_TYPE_GRAPHICS,
        .queue_count = 1
    };
    CGPUDeviceDescriptor device_desc = {
        .queue_groups = &G,
        .queue_group_count = 1
    };
    CGPUDeviceId device = cgpu_create_device(adapter, &device_desc);
    skr_thread_sleep(4000);

    // Create compute shader
    uint32_t *shader_bytes, shader_length;
    read_shader_bytes("cgpu-raytracing/raytracing",
    &shader_bytes, &shader_length, backend);
    CGPUShaderLibraryDescriptor shader_desc = {
        .code = shader_bytes,
        .code_size = shader_length,
        .name = "ComputeShaderLibrary",
        .stage = CGPU_SHADER_STAGE_COMPUTE
    };
    CGPUShaderLibraryId compute_shader = cgpu_create_shader_library(device, &shader_desc);
    free(shader_bytes);

    // Create root signature
    CGPUShaderEntryDescriptor compute_shader_entry = {
        .entry = "compute_main",
        .stage = CGPU_SHADER_STAGE_COMPUTE,
        .library = compute_shader
    };
    CGPURootSignatureDescriptor root_desc = {
        .shaders = &compute_shader_entry,
        .shader_count = 1
    };
    CGPURootSignatureId signature = cgpu_create_root_signature(device, &root_desc);

    // Create compute pipeline
    CGPUComputePipelineDescriptor pipeline_desc = {
        .compute_shader = &compute_shader_entry,
        .root_signature = signature
    };
    CGPUComputePipelineId pipeline = cgpu_create_compute_pipeline(device, &pipeline_desc);

    CGPUDescriptorSetDescriptor set_desc = {
        .root_signature = signature,
        .set_index = 0
    };
    CGPUDescriptorSetId set = cgpu_create_descriptor_set(device, &set_desc);

    // Create data buffer
    CGPUBufferDescriptor buffer_desc = {
        .name = "DataBuffer",
        .flags = CGPU_BCF_NONE,
        .descriptors = CGPU_RESOURCE_TYPE_RW_BUFFER,
        .start_state = CGPU_RESOURCE_STATE_UNORDERED_ACCESS,
        .memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
        .element_stride = sizeof(Pixel),
        .element_count = RAYTRACING_WIDTH * RAYTRACING_HEIGHT,
        .size = sizeof(Pixel) * RAYTRACING_WIDTH * RAYTRACING_HEIGHT
    };
    CGPUBufferId data_buffer = cgpu_create_buffer(device, &buffer_desc);

    // Create readback buffer
    CGPUBufferDescriptor rb_desc = {
        .name = "ReadbackBuffer",
        .flags = CGPU_BCF_NONE,
        .descriptors = CGPU_RESOURCE_TYPE_NONE,
        .start_state = CGPU_RESOURCE_STATE_COPY_DEST,
        .memory_usage = CGPU_MEM_USAGE_GPU_TO_CPU,
        .element_stride = buffer_desc.element_stride,
        .element_count = buffer_desc.element_count,
        .size = buffer_desc.size
    };
    CGPUBufferId readback_buffer = cgpu_create_buffer(device, &rb_desc);

    // Create vertex buffer
    float vertices[] = {
        0.75f, 0.75f, 0.f,
        0.25f, 0.25f, 0.f,
        0.75f, 0.25f, 0.f,
        0.25f, 0.75f, 0.f
    };
    CGPUBufferDescriptor vertex_buffer_desc = {
        .name = "VertexBuffer",
        .flags = CGPU_BCF_PERSISTENT_MAP_BIT,
        .descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER,
        .start_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
        .element_stride = sizeof(float) * 3,
        .element_count = 12,
        .size = sizeof(vertices)
    };
    CGPUBufferId vertex_buffer = cgpu_create_buffer(device, &vertex_buffer_desc);
    memcpy(vertex_buffer->info->cpu_mapped_address, vertices, vertex_buffer_desc.size);

    // Create index buffer
    uint16_t indices[] = {
        0, 1, 2,
        0, 3, 1
    };
    CGPUBufferDescriptor index_buffer_desc = {
        .name = "IndexBuffer",
        .flags = CGPU_BCF_PERSISTENT_MAP_BIT,
        .descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER,
        .start_state = CGPU_RESOURCE_STATE_INDEX_BUFFER,
        .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
        .element_stride = sizeof(uint16_t),
        .element_count = 6,
        .size = sizeof(indices)
    };
    CGPUBufferId index_buffer = cgpu_create_buffer(device, &index_buffer_desc);
    memcpy(index_buffer->info->cpu_mapped_address, indices, index_buffer_desc.size);

    CGPUAccelerationStructureGeometryDesc blas_geom = { 0 };
    blas_geom.flags = CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE;
    blas_geom.vertex_buffer = vertex_buffer;
    blas_geom.index_buffer = index_buffer;
    blas_geom.vertex_offset = 0;
    blas_geom.vertex_count = 4;
    blas_geom.vertex_stride = sizeof(float) * 3;
    blas_geom.vertex_format = CGPU_FORMAT_R32G32B32_SFLOAT;
    blas_geom.index_offset = 0;
    blas_geom.index_count = 6;
    blas_geom.index_stride = sizeof(uint16_t);

    CGPUAccelerationStructureDescriptor blas_desc = { 0 };
    blas_desc.type = CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    blas_desc.flags = CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    blas_desc.bottom.count = 1;
    blas_desc.bottom.geometries = &blas_geom;
    CGPUAccelerationStructureId blas = cgpu_create_acceleration_structure(device, &blas_desc);

    CGPUAccelerationStructureInstanceDesc tlas_instance = { 0 };
    tlas_instance.bottom = blas;
    tlas_instance.instance_id = 0;
    tlas_instance.instance_mask = 255;
    tlas_instance.transform[0] = 1.f;
    tlas_instance.transform[5] = 1.f;
    tlas_instance.transform[10] = 1.f;

    CGPUAccelerationStructureDescriptor tlas_desc = { 0 };
    tlas_desc.type = CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlas_desc.flags = CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    tlas_desc.top.count = 1;
    tlas_desc.top.instances = &tlas_instance;
    CGPUAccelerationStructureId tlas = cgpu_create_acceleration_structure(device, &tlas_desc);

    // Update descriptor set
    CGPUDescriptorData descriptor_data[2] = {
        {
            .name = "buf",
            .binding_type = CGPU_RESOURCE_TYPE_RW_BUFFER,
            .buffers = &data_buffer,
            .count = 1
        },
        {
            .name = "AS",
            .binding_type = CGPU_RESOURCE_TYPE_ACCELERATION_STRUCTURE,
            .acceleration_structures = &tlas,
            .count = 1
        }
    };
    cgpu_update_descriptor_set(set, descriptor_data, 2);

    // Create command objects
    CGPUQueueId gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    CGPUCommandPoolId pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
    CGPUCommandBufferDescriptor cmd_desc = { .is_secondary = false };
    CGPUCommandBufferId cmd = cgpu_create_command_buffer(pool, &cmd_desc);

    // Dispatch
    {
        cgpu_cmd_begin(cmd);
        // Build BLAS then TLAS
        CGPUAccelerationStructureBuildDescriptor blas_build = { .type = CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL, .as = &blas, .as_count = 1 };
        cgpu_cmd_build_acceleration_structures(cmd, &blas_build);
        CGPUAccelerationStructureBuildDescriptor tlas_build = { .type = CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL, .as = &tlas, .as_count = 1 };
        cgpu_cmd_build_acceleration_structures(cmd, &tlas_build);
        // Begin dispatch compute pass
        CGPUComputePassDescriptor pass_desc = { .name = "ComputePass" };
        CGPUComputePassEncoderId encoder = cgpu_cmd_begin_compute_pass(cmd, &pass_desc);
        cgpu_compute_encoder_bind_pipeline(encoder, pipeline);
        cgpu_compute_encoder_bind_descriptor_set(encoder, set);
        cgpu_compute_encoder_dispatch(encoder,
            (uint32_t)ceil(RAYTRACING_WIDTH / (float)32),
            (uint32_t)ceil(RAYTRACING_HEIGHT / (float)32),
            1);
        cgpu_cmd_end_compute_pass(cmd, encoder);
        // Barrier UAV buffer to transfer source
        CGPUBufferBarrier buffer_barrier = {
            .buffer = data_buffer,
            .src_state = CGPU_RESOURCE_STATE_UNORDERED_ACCESS,
            .dst_state = CGPU_RESOURCE_STATE_COPY_SOURCE
        };
        CGPUResourceBarrierDescriptor barriers_desc = {
            .buffer_barriers = &buffer_barrier,
            .buffer_barriers_count = 1
        };
        cgpu_cmd_resource_barrier(cmd, &barriers_desc);
        // Copy buffer to readback
        CGPUBufferToBufferTransfer cpy_desc = {
            .src = data_buffer,
            .src_offset = 0,
            .dst = readback_buffer,
            .dst_offset = 0,
            .size = buffer_desc.size
        };
        cgpu_cmd_transfer_buffer_to_buffer(cmd, &cpy_desc);
        cgpu_cmd_end(cmd);
        CGPUQueueSubmitDescriptor submit_desc = {
            .cmds = &cmd,
            .cmds_count = 1
        };
        cgpu_submit_queue(gfx_queue, &submit_desc);
        cgpu_wait_queue_idle(gfx_queue);
    }

    // Map buffer and readback
    unsigned char* image;
    {
        CGPUBufferRange map_range = {
            .offset = 0,
            .size = buffer_desc.size
        };
        cgpu_map_buffer(readback_buffer, &map_range);
        Pixel* mapped_memory = (Pixel*)readback_buffer->info->cpu_mapped_address;
        image = sakura_malloc(RAYTRACING_WIDTH * RAYTRACING_HEIGHT * 4);
        for (int i = 0; i < RAYTRACING_WIDTH * RAYTRACING_HEIGHT; i += 1)
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
        unsigned error = lodepng_encode32_file(gPNGNames[backend], image, RAYTRACING_WIDTH, RAYTRACING_HEIGHT);
        if (error)
            printf("encoder error %d: %s", error, lodepng_error_text(error));
    }
    // Clean up
    cgpu_free_command_buffer(cmd);
    cgpu_free_command_pool(pool);
    cgpu_free_acceleration_structure(blas);
    cgpu_free_acceleration_structure(tlas);
    cgpu_free_buffer(vertex_buffer);
    cgpu_free_buffer(index_buffer);
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

#include "SkrOS/thread.h"

int main(void)
{
    // When we support more add them here
    ECGPUBackend backends[] = {
#if SKR_PLAT_MACOSX
        CGPU_BACKEND_METAL,
#endif

#ifdef CGPU_USE_VULKAN
       //  CGPU_BACKEND_VULKAN,
#endif

#ifdef CGPU_USE_D3D12
        CGPU_BACKEND_D3D12,
#endif
    };
    const uint32_t CGPU_BACKEND_COUNT = sizeof(backends) / sizeof(ECGPUBackend);
    SKR_DECLARE_ZERO_VLA(SThreadHandle, hdls, CGPU_BACKEND_COUNT)
    SKR_DECLARE_ZERO_VLA(SThreadDesc, thread_descs, CGPU_BACKEND_COUNT)
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