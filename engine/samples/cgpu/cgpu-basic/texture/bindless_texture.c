#include "common/utils.h"
#include "common/texture.h"
#include "math.h"
#include "SkrGraphics/api.h"
#include "SkrOS/thread.h"

#define FLIGHT_FRAMES 3
#define BACK_BUFFER_COUNT 3
#define TEXTURE_COUNT 4  // Number of different colored textures

ECGPUBackend backend;
SDL_Window* sdl_window;
CGPUSurfaceId surface;
CGPUSwapChainId swapchain;
uint32_t backbuffer_index;
CGPUInstanceId instance;
CGPUAdapterId adapter;
CGPUDeviceId device;
CGPUSemaphoreId present_semaphore;
CGPUFenceId exec_fences[FLIGHT_FRAMES];
CGPUQueueId gfx_queue;
CGPURootSignatureId root_sig;
CGPUDescriptorSetId desc_set;
CGPUDescriptorBufferId descriptor_buffer;  // For bindless textures
CGPURenderPipelineId pipeline;
CGPUCommandPoolId pools[FLIGHT_FRAMES];
CGPUCommandBufferId cmds[FLIGHT_FRAMES];
CGPUTextureId sampled_textures[TEXTURE_COUNT];
CGPUSamplerId sampler_state;
CGPUTextureViewId views[BACK_BUFFER_COUNT];

// Color tints for each texture
static const float color_tints[TEXTURE_COUNT][4] = {
    {1.0f, 0.5f, 0.5f, 1.0f},  // Red tinted
    {0.5f, 1.0f, 0.5f, 1.0f},  // Green tinted
    {0.5f, 0.5f, 1.0f, 1.0f},  // Blue tinted
    {1.0f, 1.0f, 0.5f, 1.0f}   // Yellow tinted
};

void tint_texture_data(uint8_t* dst, const uint8_t* src, size_t size, const float* tint)
{
    for (size_t i = 0; i < size; i += 4)
    {
        dst[i + 0] = (uint8_t)(src[i + 0] * tint[0]);  // R
        dst[i + 1] = (uint8_t)(src[i + 1] * tint[1]);  // G
        dst[i + 2] = (uint8_t)(src[i + 2] * tint[2]);  // B
        dst[i + 3] = src[i + 3];                       // A (unchanged)
    }
}

void create_bindless_textures()
{
    // Create sampler
    CGPUSamplerDescriptor sampler_desc = {
        .address_u = CGPU_ADDRESS_MODE_REPEAT,
        .address_v = CGPU_ADDRESS_MODE_REPEAT,
        .address_w = CGPU_ADDRESS_MODE_REPEAT,
        .mipmap_mode = CGPU_MIPMAP_MODE_LINEAR,
        .min_filter = CGPU_FILTER_TYPE_LINEAR,
        .mag_filter = CGPU_FILTER_TYPE_LINEAR,
        .compare_func = CGPU_CMP_NEVER
    };
    sampler_state = cgpu_create_sampler(device, &sampler_desc);
    
    // Create upload buffer for all textures
    const size_t texture_data_size = sizeof(TEXTURE_DATA);
    const size_t total_upload_size = texture_data_size * TEXTURE_COUNT;
    CGPUBufferDescriptor upload_buffer_desc = {
        .name = "UploadBuffer",
        .flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT,
        .usages = CGPU_BUFFER_USAGE_NONE,
        .memory_usage = CGPU_MEM_USAGE_CPU_ONLY,
        .size = total_upload_size
    };
    CGPUBufferId upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    uint8_t* mapped_data = (uint8_t*)upload_buffer->info->cpu_mapped_address;
    
    // Create textures with different colors
    for (uint32_t i = 0; i < TEXTURE_COUNT; i++)
    {
        // Create texture
        CGPUTextureDescriptor tex_desc = {
            .usages = CGPU_TEXTURE_USAGE_SHADER_READ,
            .flags = CGPU_TEXTURE_FLAG_HEAP_DEDICATED_BIT,
            .width = TEXTURE_WIDTH,
            .height = TEXTURE_HEIGHT,
            .depth = 1,
            .format = CGPU_FORMAT_R8G8B8A8_UNORM,
            .array_size = 1,
            .owner_queue = gfx_queue,
            .start_state = CGPU_RESOURCE_STATE_COPY_DEST
        };
        sampled_textures[i] = cgpu_create_texture(device, &tex_desc);
        
        // Tint texture data and copy to upload buffer
        tint_texture_data(mapped_data + i * texture_data_size, 
                         TEXTURE_DATA, texture_data_size, color_tints[i]);
    }
    
    // Upload all textures
    cgpu_reset_command_pool(pools[0]);
    cgpu_cmd_begin(cmds[0]);
    
    for (uint32_t i = 0; i < TEXTURE_COUNT; i++)
    {
        CGPUBufferToTextureTransfer b2t = {
            .src = upload_buffer,
            .src_offset = i * texture_data_size,
            .dst = sampled_textures[i],
            .dst_subresource.mip_level = 0,
            .dst_subresource.base_array_layer = 0,
            .dst_subresource.layer_count = 1
        };
        cgpu_cmd_transfer_buffer_to_texture(cmds[0], &b2t);
    }
    
    // Transition all textures to shader resource state
    CGPUTextureBarrier srv_barriers[TEXTURE_COUNT] = { 0 };
    for (uint32_t i = 0; i < TEXTURE_COUNT; i++)
    {
        srv_barriers[i].texture = sampled_textures[i];
        srv_barriers[i].src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        srv_barriers[i].dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
    }
    CGPUResourceBarrierDescriptor barrier_desc = { 
        .texture_barriers = srv_barriers, 
        .texture_barriers_count = TEXTURE_COUNT 
    };
    cgpu_cmd_resource_barrier(cmds[0], &barrier_desc);
    
    cgpu_cmd_end(cmds[0]);
    CGPUQueueSubmitDescriptor cpy_submit = { .cmds = cmds, .cmds_count = 1 };
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
}

void create_descriptor_buffer()
{
    // Create descriptor buffer for bindless texture array
    CGPUDescriptorBufferDescriptor db_desc = {
        .count = TEXTURE_COUNT
    };
    descriptor_buffer = cgpu_create_descriptor_buffer(device, &db_desc);
    
    // Update descriptor buffer with texture views
    for (uint32_t i = 0; i < TEXTURE_COUNT; i++)
    {
        CGPUTextureViewDescriptor sview_desc = {
            .texture = sampled_textures[i],
            .format = CGPU_FORMAT_R8G8B8A8_UNORM,
            .array_layer_count = 1,
            .base_array_layer = 0,
            .mip_level_count = 1,
            .base_mip_level = 0,
            .aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR,
            .dims = CGPU_TEXTURE_DIMENSION_2D,
            .view_usages = CGPU_TEXTURE_VIEW_USAGE_SRV
        };
        CGPUDescriptorBufferElement elem = {
            .index = i,
            .resource_type = CGPU_RESOURCE_TYPE2_TEXTURE,
            .texture = sview_desc
        };
        cgpu_update_descriptor_buffer(descriptor_buffer, &elem, 1);
    }
}

void create_render_pipeline()
{
    // Shaders
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("cgpu-texture/bindless_texture_card.vs", &vs_bytes, &vs_length, backend);
    read_shader_bytes("cgpu-texture/bindless_texture_card.fs", &fs_bytes, &fs_length, backend);
    CGPUShaderLibraryDescriptor vs_desc = {
        .name = "VertexShaderLibrary",
        .code = vs_bytes,
        .code_size = vs_length
    };
    CGPUShaderLibraryDescriptor ps_desc = {
        .name = "FragmentShaderLibrary",
        .code = fs_bytes,
        .code_size = fs_length
    };
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    
    // Create RS with unbounded array for bindless
    CGPUShaderEntryDescriptor ppl_shaders[2];
    ppl_shaders[0].entry = "vs";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].entry = "fs";
    ppl_shaders[1].library = fragment_shader;
    
    const char8_t* sampler_name = "texture_sampler";
    const char8_t* push_constant_name = "push_constants";
    CGPURootSignatureDescriptor rs_desc = {
        .shaders = ppl_shaders,
        .shader_count = 2,
        .push_constant_names = &push_constant_name,
        .push_constant_count = 1,
        .static_samplers = &sampler_state,
        .static_sampler_count = 1,
        .static_sampler_names = &sampler_name
    };
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    
    // Create descriptor set for bindless
    CGPUDescriptorSetDescriptor desc_set_desc = {
        .root_signature = root_sig,
        .set_index = 0
    };
    desc_set = cgpu_create_descriptor_set(device, &desc_set_desc);
    
    CGPUVertexLayout vertex_layout = { .attribute_count = 0 };
    CGPURenderPipelineDescriptor rp_desc = {
        .root_signature = root_sig,
        .prim_topology = CGPU_PRIM_TOPO_TRI_LIST,
        .vertex_layout = &vertex_layout,
        .vertex_shader = &ppl_shaders[0],
        .fragment_shader = &ppl_shaders[1],
        .render_target_count = 1,
        .color_formats = &views[0]->info->format
    };
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    
    // Update descriptor set with descriptor buffer (bindless array)
    CGPUDescriptorData arguments = {
        .by_name.name = "sampled_textures",
        .count = 1,
        .descriptor_buffers = &descriptor_buffer
    };
    cgpu_update_descriptor_set(desc_set, &arguments, 1);
}

void initialize(void* usrdata)
{
    backend = *(ECGPUBackend*)usrdata;

    // Create window
    sdl_window = SDL_CreateWindow(
        gCGPUBackendNames[backend],
        BACK_BUFFER_WIDTH,
        BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN);

    // Create instance
    CGPUInstanceDescriptor instance_desc = {
        .backend = backend,
        .enable_debug_layer = true,
        .enable_gpu_based_validation = true,
        .enable_set_name = true
    };
    instance = cgpu_create_instance(&instance_desc);
    
    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    SKR_DECLARE_ZERO_VLA(CGPUAdapterId, adapters, adapters_count);
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor G = {
        .queue_type = CGPU_QUEUE_TYPE_GRAPHICS,
        .queue_count = 1
    };
    CGPUDeviceDescriptor device_desc = {
        .queue_groups = &G,
        .queue_group_count = 1
    };
    device = cgpu_create_device(adapter, &device_desc);
    
    // Create command objects
    gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    present_semaphore = cgpu_create_semaphore(device);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        pools[i] = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
        CGPUCommandBufferDescriptor cmd_desc = { .is_secondary = false };
        cmds[i] = cgpu_create_command_buffer(pools[i], &cmd_desc);
        exec_fences[i] = cgpu_create_fence(device);
    }
    
    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    surface = cgpu_surface_from_hwnd(device, (HWND)SDLGetNativeWindowHandle(sdl_window));
#elif defined(__APPLE__)
    struct CGPUNSView* ns_view = (struct CGPUNSView*)nswindow_get_content_view(SDLGetNativeWindowHandle(sdl_window));
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGPUSwapChainDescriptor descriptor = {
        .present_queues = &gfx_queue,
        .present_queues_count = 1,
        .width = BACK_BUFFER_WIDTH,
        .height = BACK_BUFFER_HEIGHT,
        .surface = surface,
        .image_count = BACK_BUFFER_COUNT,
        .format = CGPU_FORMAT_R8G8B8A8_UNORM,
        .enable_vsync = true
    };
    swapchain = cgpu_create_swapchain(device, &descriptor);
    
    // Create views
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        CGPUTextureViewDescriptor view_desc = {
            .texture = swapchain->back_buffers[i],
            .aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR,
            .dims = CGPU_TEXTURE_DIMENSION_2D,
            .format = swapchain->back_buffers[i]->info->format,
            .view_usages = CGPU_TEXTURE_VIEW_USAGE_RTV_DSV,
            .array_layer_count = 1
        };
        views[i] = cgpu_create_texture_view(device, &view_desc);
    }
    
    create_bindless_textures();
    create_descriptor_buffer();
    create_render_pipeline();
}

typedef struct PushConstants
{
    uint32_t TextureIndex;  // Which texture to use from the bindless array
    float ColorMultiplier;
} PushConstants;

static uint32_t frame_count = 0;

void raster_redraw()
{
    // sync & reset
    CGPUAcquireNextDescriptor acquire_desc = {
        .signal_semaphore = present_semaphore
    };
    backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    CGPUCommandPoolId pool = pools[backbuffer_index];
    CGPUCommandBufferId cmd = cmds[backbuffer_index];
    const CGPUTextureId back_buffer = swapchain->back_buffers[backbuffer_index];
    cgpu_wait_fences(exec_fences + backbuffer_index, 1);
    cgpu_reset_command_pool(pool);
    
    // Cycle through textures every 300 frames
    PushConstants push_data = {
        .TextureIndex = (frame_count / 300) % TEXTURE_COUNT,
        .ColorMultiplier = 1.0f
    };
    frame_count++;
    
    // record
    cgpu_cmd_begin(cmd);
    CGPUColorAttachment screen_attachment = {
        .view = views[backbuffer_index],
        .load_action = CGPU_LOAD_ACTION_CLEAR,
        .store_action = CGPU_STORE_ACTION_STORE,
        .clear_color = fastclear_0000
    };
    CGPURenderPassDescriptor rp_desc = {
        .render_target_count = 1,
        .sample_count = CGPU_SAMPLE_COUNT_1,
        .color_attachments = &screen_attachment,
        .depth_stencil = CGPU_NULLPTR
    };
    CGPUTextureBarrier draw_barrier = {
        .texture = back_buffer,
        .src_state = CGPU_RESOURCE_STATE_UNDEFINED,
        .dst_state = CGPU_RESOURCE_STATE_RENDER_TARGET
    };
    CGPUResourceBarrierDescriptor barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc0);
    CGPURenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);
    {
        cgpu_render_encoder_set_viewport(rp_encoder, 0.0f, 0.0f, (float)back_buffer->info->width, (float)back_buffer->info->height, 0.f, 1.f);
        cgpu_render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->info->width, back_buffer->info->height);
        cgpu_render_encoder_bind_pipeline(rp_encoder, pipeline);
        cgpu_render_encoder_bind_descriptor_set(rp_encoder, desc_set);
        cgpu_render_encoder_push_constants(rp_encoder, root_sig, "push_constants", &push_data);
        cgpu_render_encoder_draw(rp_encoder, 6, 0);
    }
    cgpu_cmd_end_render_pass(cmd, rp_encoder);
    CGPUTextureBarrier present_barrier = {
        .texture = back_buffer,
        .src_state = CGPU_RESOURCE_STATE_RENDER_TARGET,
        .dst_state = CGPU_RESOURCE_STATE_PRESENT
    };
    CGPUResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);
    cgpu_cmd_end(cmd);
    
    // submit
    CGPUQueueSubmitDescriptor submit_desc = {
        .cmds = &cmd,
        .cmds_count = 1,
        .signal_fence = exec_fences[backbuffer_index]
    };
    cgpu_submit_queue(gfx_queue, &submit_desc);
    
    // present
    CGPUQueuePresentDescriptor present_desc = {
        .index = backbuffer_index,
        .swapchain = swapchain,
        .wait_semaphore_count = 1,
        .wait_semaphores = &present_semaphore
    };
    cgpu_queue_present(gfx_queue, &present_desc);
}

void raster_program()
{
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    quit = true;
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED)
                {
                    cgpu_wait_queue_idle(gfx_queue);
                    int width = 0, height = 0;
                    SDL_GetWindowSize(sdl_window, &width, &height);
                    CGPUSwapChainDescriptor descriptor = {
                        .present_queues = &gfx_queue,
                        .present_queues_count = 1,
                        .width = width,
                        .height = height,
                        .surface = surface,
                        .image_count = BACK_BUFFER_COUNT,
                        .format = CGPU_FORMAT_R8G8B8A8_UNORM,
                        .enable_vsync = true
                    };
                    cgpu_free_swapchain(swapchain);
                    swapchain = cgpu_create_swapchain(device, &descriptor);
                    // Create views
                    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
                    {
                        cgpu_free_texture_view(views[i]);
                        CGPUTextureViewDescriptor view_desc = {
                            .texture = swapchain->back_buffers[i],
                            .aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR,
                            .dims = CGPU_TEXTURE_DIMENSION_2D,
                            .format = swapchain->back_buffers[i]->info->format,
                            .view_usages = CGPU_TEXTURE_VIEW_USAGE_RTV_DSV,
                            .array_layer_count = 1
                        };
                        views[i] = cgpu_create_texture_view(device, &view_desc);
                    }
                    skr_thread_sleep(100);
                }
            }
        }
        raster_redraw();
    }
}

void finalize()
{
    SDL_DestroyWindow(sdl_window);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(exec_fences, FLIGHT_FRAMES);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        cgpu_free_command_buffer(cmds[i]);
        cgpu_free_command_pool(pools[i]);
        cgpu_free_fence(exec_fences[i]);
    }
    cgpu_free_semaphore(present_semaphore);
    cgpu_free_descriptor_set(desc_set);
    cgpu_free_descriptor_buffer(descriptor_buffer);
    cgpu_free_sampler(sampler_state);
    for (uint32_t i = 0; i < TEXTURE_COUNT; i++)
    {
        cgpu_free_texture(sampled_textures[i]);
    }
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        cgpu_free_texture_view(views[i]);
    }
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(root_sig);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

void ProgramMain(void* usrdata)
{
    initialize(usrdata);
    raster_program();
    finalize();
}

int main(int argc, char* argv[])
{
#ifdef CGPU_USE_D3D12
    ECGPUBackend backend = CGPU_BACKEND_D3D12;
#else
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
#endif
    ProgramMain(&backend);

    return 0;
}