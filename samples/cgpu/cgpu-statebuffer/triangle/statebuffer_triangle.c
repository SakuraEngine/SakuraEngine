#include "common/utils.h"
#include "platform/thread.h"
#include "math.h"

// Render objects
THREAD_LOCAL SDL_Window* sdl_window;
THREAD_LOCAL ECGPUBackend backend;
THREAD_LOCAL CGPUSurfaceId surface;
THREAD_LOCAL CGPUSwapChainId swapchain;
THREAD_LOCAL uint32_t backbuffer_index;
THREAD_LOCAL CGPUInstanceId instance;
THREAD_LOCAL CGPUAdapterId adapter;
THREAD_LOCAL CGPUDeviceId device;
THREAD_LOCAL CGPUFenceId present_fence;
THREAD_LOCAL CGPUQueueId gfx_queue;
THREAD_LOCAL CGPURootSignatureId root_sig;
THREAD_LOCAL CGPULinkedShaderId linked_shader;
THREAD_LOCAL CGPUCommandPoolId pool;
THREAD_LOCAL CGPUCommandBufferId cmd;
THREAD_LOCAL CGPUStateBufferId sb;
THREAD_LOCAL CGPUTextureViewId views[3];

void create_shaders()
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("statebuffer-triangle/vertex_shader", &vs_bytes, &vs_length, backend);
    read_shader_bytes("statebuffer-triangle/fragment_shader", &fs_bytes, &fs_length, backend);
    CGPUShaderLibraryDescriptor shader_libs[2] = {
        {
            .stage = CGPU_SHADER_STAGE_VERT,
            .name = "VertexShaderLibrary",
            .code = vs_bytes,
            .code_size = vs_length,
            .reflection_only = true
        },
        {
            .name = "FragmentShaderLibrary",
            .stage = CGPU_SHADER_STAGE_FRAG,
            .code = fs_bytes,
            .code_size = fs_length,
            .reflection_only = true
        } 
    };
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &shader_libs[0]);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &shader_libs[1]);
    CGPUShaderEntryDescriptor ppl_shaders[2] = {
        {
            .stage = CGPU_SHADER_STAGE_VERT,
            .entry = "main",
            .library = vertex_shader,
        },
        {
            .stage = CGPU_SHADER_STAGE_FRAG,
            .entry = "main",
            .library = fragment_shader,
        }
    };
    CGPURootSignatureDescriptor rs_desc = {
        .shaders = ppl_shaders,
        .shader_count = 2
    };
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGPUCompiledShaderDescriptor compile_shaders[2]  = {
        {
            .entry = ppl_shaders[0],
            .shader_code = vs_bytes,
            .code_size = vs_length
        },
        {
            .entry = ppl_shaders[1],
            .shader_code = fs_bytes,
            .code_size = fs_length
        }
    };
    linked_shader = cgpu_compile_and_link_shaders(root_sig, compile_shaders, 2);

    free(vs_bytes);
    free(fs_bytes);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

void initialize(void* usrdata)
{
    // Create window
    SDL_SysWMinfo wmInfo;
    backend = *(ECGPUBackend*)usrdata;

    sdl_window = SDL_CreateWindow(gCGPUBackendNames[backend],
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);

    // Create instance
    CGPUInstanceDescriptor instance_desc = {
        .backend = backend,
        .enable_debug_layer = false,
        .enable_gpu_based_validation = false,
        .enable_set_name = true
    };
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    DECLARE_ZERO_VLA(CGPUAdapterId, adapters, adapters_count);
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
    gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);

    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    surface = cgpu_surface_from_hwnd(device, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGPUNSView* ns_view = (struct CGPUNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGPUSwapChainDescriptor descriptor = {
        .present_queues = &gfx_queue,
        .present_queues_count = 1,
        .width = BACK_BUFFER_WIDTH,
        .height = BACK_BUFFER_HEIGHT,
        .surface = surface,
        .imageCount = 3,
        .format = CGPU_FORMAT_R8G8B8A8_UNORM,
        .enable_vsync = true
    };
    swapchain = cgpu_create_swapchain(device, &descriptor);
    // Create views
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        CGPUTextureViewDescriptor view_desc = {
            .texture = swapchain->back_buffers[i],
            .aspects = CGPU_TVA_COLOR,
            .array_layer_count = 1,
            .dims = CGPU_TEX_DIMENSION_2D,
            .format = swapchain->back_buffers[i]->format,
            .usages = CGPU_TVU_RTV_DSV
        };
        views[i] = cgpu_create_texture_view(device, &view_desc);
    }
    create_shaders();
}

void triangle_pass(uint32_t w, uint32_t h)
{
    const CGPUTextureViewId back_buffer_view = views[backbuffer_index];
    CGPUColorAttachment screen_attachment = {
        .view = back_buffer_view,
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
    CGPURenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);
    {
        cgpu_render_encoder_bind_state_buffer(rp_encoder, sb);
        {
            CGPURasterStateEncoderId cx = cgpu_open_raster_state_encoder(sb, rp_encoder);
            cgpu_raster_state_encoder_set_primitive_topology(cx, CGPU_PRIM_TOPO_TRI_LIST);
            cgpu_raster_state_encoder_set_fill_mode(cx, CGPU_FILL_MODE_SOLID);
            cgpu_raster_state_encoder_set_front_face(cx, CGPU_FRONT_FACE_CCW);
            cgpu_raster_state_encoder_set_scissor(cx, 0, 0, w, h);
            cgpu_raster_state_encoder_set_viewport(cx, 0.0f, 0.0f, (float)w, (float)h, 0.f, 1.f);
            cgpu_close_raster_state_encoder(cx);
        }
        {
            CGPUShaderStateEncoderId sh = cgpu_open_shader_state_encoder_r(sb, rp_encoder);
            cgpu_shader_state_encoder_bind_linked_shader(sh, linked_shader);
            cgpu_close_shader_state_encoder(sh);
        }
        cgpu_render_encoder_draw(rp_encoder, 3, 0);
    }
    cgpu_cmd_end_render_pass(cmd, rp_encoder);
}

void raster_redraw()
{
    // sync & reset
    cgpu_wait_fences(&present_fence, 1);
    CGPUAcquireNextDescriptor acquire_desc = { .fence = present_fence };
    backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    const CGPUTextureId back_buffer = swapchain->back_buffers[backbuffer_index];
    cgpu_reset_command_pool(pool);
    cgpu_cmd_begin(cmd);
    
    // barrier swapchain to drawable
    CGPUTextureBarrier draw_barrier = {
        .texture = back_buffer,
        .src_state = CGPU_RESOURCE_STATE_UNDEFINED,
        .dst_state = CGPU_RESOURCE_STATE_RENDER_TARGET
    };
    CGPUResourceBarrierDescriptor barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc0);

    // record pass & draws
    triangle_pass(back_buffer->width, back_buffer->height);

    // barrier swapchain to present
    CGPUTextureBarrier present_barrier = {
        .texture = back_buffer,
        .src_state = CGPU_RESOURCE_STATE_RENDER_TARGET,
        .dst_state = CGPU_RESOURCE_STATE_PRESENT
    };
    CGPUResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);

    // end & submit cmd
    cgpu_cmd_end(cmd);
    CGPUQueueSubmitDescriptor submit_desc = { .cmds = &cmd, .cmds_count = 1, };
    cgpu_submit_queue(gfx_queue, &submit_desc);
    
    // present swapchain
    cgpu_wait_queue_idle(gfx_queue);
    CGPUQueuePresentDescriptor present_desc = {
        .index = backbuffer_index,
        .swapchain = swapchain,
        .wait_semaphore_count = 0,
        .wait_semaphores = CGPU_NULLPTR
    };
    cgpu_queue_present(gfx_queue, &present_desc);
}

void raster_program()
{
    // Create command objects
    pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
    CGPUCommandBufferDescriptor cmd_desc = { .is_secondary = false };
    cmd = cgpu_create_command_buffer(pool, &cmd_desc);
    sb = cgpu_create_state_buffer(cmd, NULL);

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
            }
        }
        raster_redraw();
    }
}

void finalize()
{
    SDL_DestroyWindow(sdl_window);
    // Free cgpu objects
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        cgpu_free_texture_view(views[i]);
    }
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_command_buffer(cmd);
    cgpu_free_command_pool(pool);
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
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    // When we support more add them here
    ECGPUBackend backends[] = {
        CGPU_BACKEND_VULKAN
#ifdef CGPU_USE_D3D12
        , CGPU_BACKEND_D3D12
#endif
    };

    ProgramMain(backends);

    SDL_Quit();

    return 0;
}