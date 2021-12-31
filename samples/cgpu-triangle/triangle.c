#include "triangle_module.c"
#include "../common/utils.h"
#include "platform/thread.h"
#include "utils.h"
#include "math.h"

// WA Engine
THREAD_LOCAL SWAInstanceId wa_instance;
THREAD_LOCAL SWARuntimeId wa_runtime;
THREAD_LOCAL SWAModuleId wa_module;

// Render objects
THREAD_LOCAL ECGpuBackend backend;
THREAD_LOCAL SDL_Window* sdl_window;
THREAD_LOCAL CGpuSurfaceId surface;
THREAD_LOCAL CGpuSwapChainId swapchain;
THREAD_LOCAL uint32_t backbuffer_index;
THREAD_LOCAL CGpuInstanceId instance;
THREAD_LOCAL CGpuAdapterId adapter;
THREAD_LOCAL CGpuDeviceId device;
THREAD_LOCAL CGpuFenceId present_fence;
THREAD_LOCAL CGpuQueueId gfx_queue;
THREAD_LOCAL CGpuRootSignatureId root_sig;
THREAD_LOCAL CGpuRenderPipelineId pipeline;
THREAD_LOCAL CGpuCommandPoolId pool;
THREAD_LOCAL CGpuCommandBufferId cmd;
THREAD_LOCAL CGpuTextureViewId views[3];

const uint32_t* get_vertex_shader()
{
    if (backend == CGPU_BACKEND_VULKAN) return (const uint32_t*)vertex_shader_spirv;
    if (backend == CGPU_BACKEND_D3D12) return (const uint32_t*)vertex_shader_dxil;
    return CGPU_NULLPTR;
}
const size_t get_vertex_shader_size()
{
    if (backend == CGPU_BACKEND_VULKAN) return sizeof(vertex_shader_spirv);
    if (backend == CGPU_BACKEND_D3D12) return sizeof(vertex_shader_dxil);
    return 0;
}
const uint32_t* get_fragment_shader()
{
    if (backend == CGPU_BACKEND_VULKAN) return (const uint32_t*)fragment_shader_spirv;
    if (backend == CGPU_BACKEND_D3D12) return (const uint32_t*)fragment_shader_dxil;
    return CGPU_NULLPTR;
}
const size_t get_fragment_shader_size()
{
    if (backend == CGPU_BACKEND_VULKAN) return sizeof(fragment_shader_spirv);
    if (backend == CGPU_BACKEND_D3D12) return sizeof(fragment_shader_dxil);
    return 0;
}

void create_render_pipeline()
{
    CGpuShaderLibraryDescriptor vs_desc = {
        .stage = SHADER_STAGE_VERT,
        .name = "VertexShaderLibrary",
        .code = get_vertex_shader(),
        .code_size = get_vertex_shader_size()
    };
    CGpuShaderLibraryDescriptor ps_desc = {
        .name = "FragmentShaderLibrary",
        .stage = SHADER_STAGE_FRAG,
        .code = get_fragment_shader(),
        .code_size = get_fragment_shader_size()
    };
    CGpuShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    CGpuRootSignatureDescriptor rs_desc = {
        .shaders = ppl_shaders,
        .shaders_count = 2
    };
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = { .attribute_count = 0 };
    CGpuRenderPipelineDescriptor rp_desc = {
        .root_signature = root_sig,
        .prim_topology = PRIM_TOPO_TRI_LIST,
        .vertex_layout = &vertex_layout,
        .vertex_shader = &ppl_shaders[0],
        .fragment_shader = &ppl_shaders[1],
        .render_target_count = 1,
        .color_formats = &views[0]->info.format
    };
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

void initialize(void* usrdata)
{
    // WASM
    setup_wasm(&wa_instance, &wa_runtime, &wa_module);
    // Create window
    SDL_SysWMinfo wmInfo;
    backend = *(ECGpuBackend*)usrdata;

    sdl_window = SDL_CreateWindow(gCGpuBackendNames[backend],
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);

    // Create instance
    CGpuInstanceDescriptor instance_desc = {
        .backend = backend,
        .enable_debug_layer = true,
        .enable_gpu_based_validation = true,
        .enable_set_name = true
    };
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    DECLARE_ZERO_VLA(CGpuAdapterId, adapters, adapters_count);
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGpuQueueGroupDescriptor G = {
        .queueType = QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    CGpuDeviceDescriptor device_desc = {
        .queueGroups = &G,
        .queueGroupCount = 1
    };
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);

    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    surface = cgpu_surface_from_hwnd(device, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGpuNSView* ns_view = (struct CGpuNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGpuSwapChainDescriptor descriptor = {
        .presentQueues = &gfx_queue,
        .presentQueuesCount = 1,
        .width = BACK_BUFFER_WIDTH,
        .height = BACK_BUFFER_HEIGHT,
        .surface = surface,
        .imageCount = 3,
        .format = PF_R8G8B8A8_UNORM,
        .enableVsync = true
    };
    swapchain = cgpu_create_swapchain(device, &descriptor);
    // Create views
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        CGpuTextureViewDescriptor view_desc = {
            .texture = swapchain->back_buffers[i],
            .aspects = TVA_COLOR,
            .dims = TEX_DIMENSION_2D,
            .format = swapchain->back_buffers[i]->format,
            .usages = TVU_RTV
        };
        views[i] = cgpu_create_texture_view(device, &view_desc);
    }
    create_render_pipeline();
}

void raster_redraw()
{
    // sync & reset
    cgpu_wait_fences(&present_fence, 1);
    CGpuAcquireNextDescriptor acquire_desc = {
        .fence = present_fence
    };
    backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    const CGpuTextureId back_buffer = swapchain->back_buffers[backbuffer_index];
    const CGpuTextureViewId back_buffer_view = views[backbuffer_index];
    cgpu_reset_command_pool(pool);
    // record
    if (wa_module != NULL)
    {
        SWAValue params[6];
        params[0].I = (int64_t)cmd;
        params[0].type = SWA_VAL_I64;
        params[1].I = (int64_t)pipeline;
        params[1].type = SWA_VAL_I64;
        params[2].I = (int64_t)back_buffer;
        params[2].type = SWA_VAL_I64;
        params[3].I = (int64_t)back_buffer_view;
        params[3].type = SWA_VAL_I64;
        params[4].i = back_buffer->width;
        params[4].type = SWA_VAL_I32;
        params[5].i = back_buffer->height;
        params[5].type = SWA_VAL_I32;
        SWAExecDescriptor exec_desc = {
            6, params,
            0, NULL
        };
        const char* res = swa_exec(wa_module, "raster_cmd_record", &exec_desc);
        if (res) printf("[fatal]: %s", res);
    }
    else
    {
        raster_cmd_record(cmd, pipeline,
            back_buffer, back_buffer_view,
            back_buffer->width, back_buffer->height);
    }
    // submit
    CGpuQueueSubmitDescriptor submit_desc = {
        .cmds = &cmd,
        .cmds_count = 1,
    };
    cgpu_submit_queue(gfx_queue, &submit_desc);
    // present
    cgpu_wait_queue_idle(gfx_queue);
    CGpuQueuePresentDescriptor present_desc = {
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
    CGpuCommandBufferDescriptor cmd_desc = { .is_secondary = false };
    cmd = cgpu_create_command_buffer(pool, &cmd_desc);

    while (sdl_window)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // olog::Info(u"event type: {}  windowID: {}"_o, (int)event.type, (int)event.window.windowID);
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    sdl_window = CGPU_NULLPTR;
                }
            }
        }
        raster_redraw();
    }
}

void finalize()
{
    SDL_DestroyWindow(sdl_window);
    // Free wasm engine
    finalize_wasm(wa_instance, wa_runtime, wa_module);
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
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    // When we support more add them here
    ECGpuBackend backends[] = {
        CGPU_BACKEND_VULKAN
#ifdef CGPU_USE_D3D12
        ,
        CGPU_BACKEND_D3D12
#endif
    };
    ProgramMain(backends);

    SDL_Quit();
    return 0;
}