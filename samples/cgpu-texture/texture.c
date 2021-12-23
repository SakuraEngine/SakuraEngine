#include "utils.h"
#include "math.h"
#include "cgpu/api.h"

_Thread_local ECGpuBackend backend;
_Thread_local SDL_Window* sdl_window;
_Thread_local CGpuSurfaceId surface;
_Thread_local CGpuSwapChainId swapchain;
_Thread_local uint32_t backbuffer_index;
_Thread_local CGpuInstanceId instance;
_Thread_local CGpuAdapterId adapter;
_Thread_local CGpuDeviceId device;
_Thread_local CGpuFenceId present_fence;
_Thread_local CGpuQueueId gfx_queue;
_Thread_local CGpuRootSignatureId root_sig;
_Thread_local CGpuDescriptorSetId desc_set;
_Thread_local CGpuRenderPipelineId pipeline;
_Thread_local CGpuCommandPoolId pool;
_Thread_local CGpuCommandBufferId cmd;
_Thread_local CGpuTextureId sampled_texture;
_Thread_local CGpuTextureViewId sampled_view;
_Thread_local CGpuTextureViewId views[3];

const uint32_t* get_vertex_shader()
{
    if (backend == ECGpuBackend_VULKAN) return (const uint32_t*)vertex_shader_spirv;
    return CGPU_NULLPTR;
}
const size_t get_vertex_shader_size()
{
    if (backend == ECGpuBackend_VULKAN) return sizeof(vertex_shader_spirv);
    return 0;
}
const uint32_t* get_fragment_shader()
{
    if (backend == ECGpuBackend_VULKAN) return (const uint32_t*)fragment_shader_spirv;
    return CGPU_NULLPTR;
}
const size_t get_fragment_shader_size()
{
    if (backend == ECGpuBackend_VULKAN) return sizeof(fragment_shader_spirv);
    return 0;
}

void create_render_pipeline()
{
    // Texture
    CGpuTextureDescriptor tex_desc = {
        .descriptors = RT_TEXTURE,
        .flags = TCF_OWN_MEMORY_BIT,
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .depth = 1,
        .format = PF_R8G8B8A8_UNORM,
        .array_size = 1,
        .start_state = RS_COPY_DEST
    };
    sampled_texture = cgpu_create_texture(device, &tex_desc);
    CGpuTextureViewDescriptor sview_desc = {
        .texture = sampled_texture,
        .format = tex_desc.format,
        .array_layer_count = 1,
        .base_array_layer = 0,
        .mip_level_count = 1,
        .base_mip_level = 0,
        .aspects = TVA_COLOR,
        .dims = TD_2D,
        .usages = TVU_SRV
    };
    sampled_view = cgpu_create_texture_view(device, &sview_desc);
    CGpuBufferDescriptor upload_buffer_desc = {
        .flags = BCF_OWN_MEMORY_BIT,
        .descriptors = RT_BUFFER,
        .memory_usage = MU_CPU_TO_GPU,
        .element_stride = sizeof(uint8_t),
        .elemet_count = sizeof(TEXTURE_DATA) / sizeof(uint8_t),
        .size = sizeof(TEXTURE_DATA),
        .name = "UploadBuffer"
    };
    CGpuBufferId upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGpuBufferRange map_range = {
        .offset = 0, .size = upload_buffer_desc.size
    };
    {
        cgpu_map_buffer(upload_buffer, &map_range);
        uint8_t* data = (uint8_t*)upload_buffer->cpu_mapped_address;
        memcpy(data, TEXTURE_DATA, upload_buffer_desc.size);
        cgpu_unmap_buffer(upload_buffer);
    }
    cgpu_reset_command_pool(pool);
    // record
    cgpu_cmd_begin(cmd);
    CGpuTextureBarrier cpy_barrier = {
        .texture = sampled_texture,
        .src_state = RS_UNDEFINED,
        .dst_state = RS_COPY_DEST
    };
    CGpuResourceBarrierDescriptor barrier_desc = { .texture_barriers = &cpy_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc);
    CGpuBufferToTextureTransfer b2t = {
        .src = upload_buffer,
        .src_offset = 0,
        .dst = sampled_texture,
        .bytes_per_row = FormatUtil_WidthOfBlock(PF_R8G8B8A8_UNORM) * TEXTURE_WIDTH,
        .rows_per_image = TEXTURE_HEIGHT,
        .base_array_layer = 0,
        .layer_count = 1
    };
    cgpu_cmd_transfer_buffer_to_texture(cmd, &b2t);
    cgpu_cmd_end(cmd);
    CGpuQueueSubmitDescriptor cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
    // Shaders
    CGpuShaderLibraryDescriptor vs_desc = { .stage = SS_VERT, .name = "VertexShaderLibrary", .code = get_vertex_shader(), .code_size = get_vertex_shader_size() };
    CGpuShaderLibraryDescriptor ps_desc = {
        .name = "FragmentShaderLibrary",
        .stage = SS_FRAG,
        .code = get_fragment_shader(),
        .code_size = get_fragment_shader_size()
    };
    CGpuShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SS_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = SS_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    CGpuRootSignatureDescriptor rs_desc = {
        .shaders = ppl_shaders,
        .shaders_count = 2
    };
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuDescriptorSetDescriptor desc_set_desc = {
        .root_signature = root_sig,
        .set_index = 0
    };
    desc_set = cgpu_create_descriptor_set(device, &desc_set_desc);
    CGpuDescriptorData arguments[2];
    arguments[0].count = 1;
    arguments[0].textures = &sampled_texture;
    arguments[1].count = 1;
    arguments[1].samplers = NULL;
    cgpu_update_descriptor_set(desc_set, arguments, 2);
    CGpuVertexLayout vertex_layout = { .attribute_count = 0 };
    CGpuRenderPipelineDescriptor rp_desc = {
        .root_signature = root_sig,
        .prim_topology = TOPO_TRI_LIST,
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
    // Create window
    SDL_SysWMinfo wmInfo;
    sdl_window = SDL_CreateWindow("title",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);

    backend = *(ECGpuBackend*)usrdata;
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
        .queueType = ECGpuQueueType_Graphics,
        .queueCount = 1
    };
    CGpuDeviceDescriptor device_desc = {
        .queueGroups = &G,
        .queueGroupCount = 1
    };
    device = cgpu_create_device(adapter, &device_desc);
    present_fence = cgpu_create_fence(device);
    // Create command objects
    gfx_queue = cgpu_get_queue(device, ECGpuQueueType_Graphics, 0);
    pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
    CGpuCommandBufferDescriptor cmd_desc = { .is_secondary = false };
    cmd = cgpu_create_command_buffer(pool, &cmd_desc);

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
            .dims = TD_2D,
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
    cgpu_reset_command_pool(pool);
    // record
    cgpu_cmd_begin(cmd);
    CGpuColorAttachment screen_attachment = {
        .view = views[backbuffer_index],
        .load_action = LA_CLEAR,
        .store_action = SA_Store,
        .clear_color = fastclear_0000
    };
    CGpuRenderPassDescriptor rp_desc = {
        .render_target_count = 1,
        .sample_count = SC_1,
        .color_attachments = &screen_attachment,
        .depth_stencil = CGPU_NULLPTR
    };
    CGpuTextureBarrier draw_barrier = {
        .texture = back_buffer,
        .src_state = RS_UNDEFINED,
        .dst_state = RS_RENDER_TARGET
    };
    CGpuResourceBarrierDescriptor barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc0);
    CGpuRenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);
    {
        cgpu_render_encoder_set_viewport(rp_encoder, 0.0f, 0.0f, back_buffer->width, back_buffer->height, 0.f, 1.f);
        cgpu_render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->width, back_buffer->height);
        cgpu_render_encoder_bind_pipeline(rp_encoder, pipeline);
        cgpu_render_encoder_draw(rp_encoder, 3, 0);
        cgpu_render_encoder_draw(rp_encoder, 3, 3);
    }
    cgpu_cmd_end_render_pass(cmd, rp_encoder);
    CGpuTextureBarrier present_barrier = {
        .texture = back_buffer,
        .src_state = RS_RENDER_TARGET,
        .dst_state = RS_PRESENT
    };
    CGpuResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);
    cgpu_cmd_end(cmd);
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
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    cgpu_free_descriptor_set(desc_set);
    cgpu_free_texture(sampled_texture);
    cgpu_free_texture_view(sampled_view);
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

int main()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;

    ECGpuBackend backend = ECGpuBackend_VULKAN;
    initialize(&backend);
    raster_program();
    finalize();

    SDL_Quit();

    return 0;
}