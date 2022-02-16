#include "utils.h"
#include "math.h"
#include "cgpu/api.h"
#include "platform/thread.h"

#define FLIGHT_FRAMES 3
#define BACK_BUFFER_COUNT 3
THREAD_LOCAL ECGpuBackend backend;
THREAD_LOCAL SDL_Window* sdl_window;
THREAD_LOCAL CGpuSurfaceId surface;
THREAD_LOCAL CGpuSwapChainId swapchain;
THREAD_LOCAL uint32_t backbuffer_index;
THREAD_LOCAL CGpuInstanceId instance;
THREAD_LOCAL CGpuAdapterId adapter;
THREAD_LOCAL CGpuDeviceId device;
THREAD_LOCAL CGpuSemaphoreId present_semaphore;
THREAD_LOCAL CGpuFenceId exec_fences[FLIGHT_FRAMES];
THREAD_LOCAL CGpuQueueId gfx_queue;
THREAD_LOCAL CGpuRootSignatureId root_sig;
THREAD_LOCAL CGpuDescriptorSetId desc_set;
THREAD_LOCAL CGpuDescriptorSetId desc_set2; // We use this for samplers under D3D12
THREAD_LOCAL CGpuRenderPipelineId pipeline;
THREAD_LOCAL CGpuCommandPoolId pools[FLIGHT_FRAMES];
THREAD_LOCAL CGpuCommandBufferId cmds[FLIGHT_FRAMES];
THREAD_LOCAL CGpuTextureId sampled_texture;
THREAD_LOCAL CGpuSamplerId sampler_state;
THREAD_LOCAL bool bUseStaticSampler = true;
THREAD_LOCAL CGpuTextureViewId sampled_view;
THREAD_LOCAL CGpuTextureViewId views[BACK_BUFFER_COUNT];
THREAD_LOCAL CGpuBufferId vertex_buffer;
THREAD_LOCAL CGpuBufferId index_buffer;

const uint32_t* get_vertex_shader()
{
    if (backend == CGPU_BACKEND_VULKAN) return (const uint32_t*)vertex_shader_spirv;
    if (backend == CGPU_BACKEND_D3D12) return (const uint32_t*)vertex_shader_dxil;
    return CGPU_NULLPTR;
}
const uint32_t get_vertex_shader_size()
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
const uint32_t get_fragment_shader_size()
{
    if (backend == CGPU_BACKEND_VULKAN) return sizeof(fragment_shader_spirv);
    if (backend == CGPU_BACKEND_D3D12) return sizeof(fragment_shader_dxil);
    return 0;
}

void create_sampled_texture()
{
    // Sampler
    CGpuSamplerDescriptor sampler_desc = {
        .address_u = ADDRESS_MODE_REPEAT,
        .address_v = ADDRESS_MODE_REPEAT,
        .address_w = ADDRESS_MODE_REPEAT,
        .mipmap_mode = MIPMAP_MODE_LINEAR,
        .min_filter = FILTER_TYPE_LINEAR,
        .mag_filter = FILTER_TYPE_LINEAR,
        .compare_func = CMP_NEVER
    };
    sampler_state = cgpu_create_sampler(device, &sampler_desc);
    // Texture
    CGpuTextureDescriptor tex_desc = {
        .descriptors = RT_TEXTURE,
        .flags = TCF_OWN_MEMORY_BIT,
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .depth = 1,
        .format = PF_R8G8B8A8_UNORM,
        .array_size = 1,
        .owner_queue = gfx_queue,
        .start_state = RESOURCE_STATE_COPY_DEST
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
        .dims = TEX_DIMENSION_2D,
        .usages = TVU_SRV
    };
    sampled_view = cgpu_create_texture_view(device, &sview_desc);
}

typedef struct Vertex {
    float position[3];
    float color[3];
    float uv[2];
} Vertex;

const Vertex vertices[] = {
    { { 0.5f, 0.5f, 0.f }, { 1.0f, 1.0f, 1.0f }, { 1.f, 1.f } },
    { { -0.5f, -0.5f, 0.f }, { 0.0f, 1.0f, 0.0f }, { 0.f, 0.f } },
    { { 0.5f, -0.5f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 1.f, 0.f } },
    { { -0.5f, 0.5f, 0.f }, { 0.0f, 1.0f, 0.0f }, { 0.f, 1.f } },
};

const uint16_t indices[] = {
    0, 1, 2, 0, 3, 1
};
typedef uint16_t Index;

void create_vertex_buffer()
{
    CGpuBufferDescriptor vertex_buffer_desc = {
        .flags = BCF_OWN_MEMORY_BIT,
        .descriptors = RT_VERTEX_BUFFER,
        .memory_usage = MEM_USAGE_GPU_ONLY,
        .element_stride = sizeof(Vertex),
        .elemet_count = sizeof(vertices) / sizeof(Vertex),
        .size = sizeof(vertices),
        .name = "VertexBuffer"
    };
    vertex_buffer = cgpu_create_buffer(device, &vertex_buffer_desc);
}

void create_index_buffer()
{
    CGpuBufferDescriptor index_buffer_desc = {
        .flags = BCF_OWN_MEMORY_BIT,
        .descriptors = RT_INDEX_BUFFER,
        .memory_usage = MEM_USAGE_GPU_ONLY,
        .element_stride = sizeof(Index),
        .elemet_count = sizeof(indices) / sizeof(Index),
        .size = sizeof(indices),
        .name = "IndexBuffer"
    };
    index_buffer = cgpu_create_buffer(device, &index_buffer_desc);
}

void upload_resources()
{
    CGpuBufferDescriptor upload_buffer_desc = {
        .name = "UploadBuffer",
        .flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT,
        .descriptors = RT_NONE,
        .memory_usage = MEM_USAGE_CPU_ONLY,
        .element_stride = sizeof(TEXTURE_DATA),
        .elemet_count = 1,
        .size = sizeof(TEXTURE_DATA)
    };
    CGpuBufferId upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    // upload texture
    {
        memcpy(upload_buffer->cpu_mapped_address, TEXTURE_DATA, upload_buffer_desc.size);
    }
    cgpu_reset_command_pool(pools[0]);
    cgpu_cmd_begin(cmds[0]);
    CGpuBufferToTextureTransfer b2t = {
        .src = upload_buffer,
        .src_offset = 0,
        .dst = sampled_texture,
        .elems_per_row = TEXTURE_WIDTH,
        .rows_per_image = TEXTURE_HEIGHT,
        .base_array_layer = 0,
        .layer_count = 1
    };
    cgpu_cmd_transfer_buffer_to_texture(cmds[0], &b2t);
    CGpuTextureBarrier srv_barrier = {
        .texture = sampled_texture,
        .src_state = RESOURCE_STATE_COPY_DEST,
        .dst_state = RESOURCE_STATE_SHADER_RESOURCE
    };
    CGpuResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &srv_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmds[0], &barrier_desc1);
    cgpu_cmd_end(cmds[0]);
    CGpuQueueSubmitDescriptor cpy_submit = { .cmds = cmds, .cmds_count = 1 };
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    // upload vertex buffer
    {
        memcpy(upload_buffer->cpu_mapped_address, vertices, sizeof(vertices));
    }
    cgpu_reset_command_pool(pools[0]);
    cgpu_cmd_begin(cmds[0]);
    CGpuBufferToBufferTransfer b2v = {
        .src = upload_buffer,
        .src_offset = 0,
        .dst = vertex_buffer,
        .dst_offset = 0,
        .size = sizeof(vertices)
    };
    cgpu_cmd_transfer_buffer_to_buffer(cmds[0], &b2v);
    CGpuBufferBarrier vb_barrier = {
        .buffer = vertex_buffer,
        .src_state = RESOURCE_STATE_COPY_DEST,
        .dst_state = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    };
    CGpuResourceBarrierDescriptor barrier_desc2 = {
        .buffer_barriers = &vb_barrier, .buffer_barriers_count = 1
    };
    cgpu_cmd_resource_barrier(cmds[0], &barrier_desc2);
    cgpu_cmd_end(cmds[0]);
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    // upload index buffer
    {
        memcpy(upload_buffer->cpu_mapped_address, indices, sizeof(indices));
    }
    cgpu_reset_command_pool(pools[0]);
    cgpu_cmd_begin(cmds[0]);
    CGpuBufferToBufferTransfer b2i = {
        .src = upload_buffer,
        .src_offset = 0,
        .dst = index_buffer,
        .dst_offset = 0,
        .size = sizeof(indices)
    };
    cgpu_cmd_transfer_buffer_to_buffer(cmds[0], &b2i);
    CGpuBufferBarrier ib_barrier = {
        .buffer = index_buffer,
        .src_state = RESOURCE_STATE_COPY_DEST,
        .dst_state = RESOURCE_STATE_INDEX_BUFFER
    };
    CGpuResourceBarrierDescriptor barrier_desc3 = {
        .buffer_barriers = &ib_barrier, .buffer_barriers_count = 1
    };
    cgpu_cmd_resource_barrier(cmds[0], &barrier_desc3);
    cgpu_cmd_end(cmds[0]);
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    // free
    cgpu_free_buffer(upload_buffer);
}

void create_render_resources()
{
    create_sampled_texture();
    create_vertex_buffer();
    create_index_buffer();
    upload_resources();
}

void create_render_pipeline()
{
    // Shaders
    CGpuShaderLibraryDescriptor vs_desc = { .name = "VertexShaderLibrary", .stage = SHADER_STAGE_VERT, .code = get_vertex_shader(), .code_size = get_vertex_shader_size() };
    CGpuShaderLibraryDescriptor ps_desc = { .name = "FragmentShaderLibrary", .stage = SHADER_STAGE_FRAG, .code = get_fragment_shader(), .code_size = get_fragment_shader_size() };
    CGpuShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    // Create RS
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    const char8_t* sampler_name = "texture_sampler";
    const char8_t* root_constant_name = "root_constants";
    CGpuRootSignatureDescriptor rs_desc = {
        .shaders = ppl_shaders,
        .shader_count = 2,
        .root_constant_names = &root_constant_name,
        .root_constant_count = 1
    };
    if (bUseStaticSampler)
    {
        rs_desc.static_samplers = &sampler_state;
        rs_desc.static_sampler_count = 1;
        rs_desc.static_sampler_names = &sampler_name;
    }
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    // Create descriptor set
    CGpuDescriptorSetDescriptor desc_set_desc = {
        .root_signature = root_sig,
        .set_index = 0
    };
    desc_set = cgpu_create_descriptor_set(device, &desc_set_desc);
    if (!bUseStaticSampler)
    {
        desc_set_desc.set_index = 1;
        desc_set2 = cgpu_create_descriptor_set(device, &desc_set_desc);
    }
    CGpuVertexLayout vertex_layout = {
        .attributes = {
            { "POSITION", PF_R32G32B32_SFLOAT, 0, 0, 0, INPUT_RATE_VERTEX },
            { "COLOR", PF_R32G32B32_SFLOAT, 0, 1, 12, INPUT_RATE_VERTEX },
            { "TEXCOORD", PF_R32G32_SFLOAT, 0, 2, 24, INPUT_RATE_VERTEX },
        },
        .attribute_count = 3
    };
    CGpuRasterizerStateDescriptor rs_state = {
        .cull_mode = CULL_MODE_BACK,
        .fill_mode = FILL_MODE_SOLID,
        .front_face = FRONT_FACE_CCW,
        .slope_scaled_depth_bias = 0.f,
        .enable_depth_clamp = false,
        .enable_scissor = false,
        .enable_multi_sample = false,
        .depth_bias = 0
    };
    CGpuRenderPipelineDescriptor rp_desc = {
        .root_signature = root_sig,
        .prim_topology = PRIM_TOPO_TRI_LIST,
        .vertex_layout = &vertex_layout,
        .vertex_shader = &ppl_shaders[0],
        .fragment_shader = &ppl_shaders[1],
        .render_target_count = 1,
        .rasterizer_state = &rs_state,
        .color_formats = &views[0]->info.format
    };
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    // Update descriptor set for once
    CGpuDescriptorData arguments[2];
    arguments[0].name = "sampled_texture";
    arguments[0].count = 1;
    arguments[0].textures = &sampled_view;
    arguments[1].name = sampler_name;
    arguments[1].count = 1;
    arguments[1].samplers = &sampler_state;
    {
        cgpu_update_descriptor_set(desc_set, arguments, 1);
        if (!bUseStaticSampler) cgpu_update_descriptor_set(desc_set2, arguments + 1, 1);
    }
}

void initialize(void* usrdata)
{
    backend = *(ECGpuBackend*)usrdata;

    // Create window
    SDL_SysWMinfo wmInfo;
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
    // Create command objects
    gfx_queue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
    present_semaphore = cgpu_create_semaphore(device);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        pools[i] = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
        CGpuCommandBufferDescriptor cmd_desc = { .is_secondary = false };
        cmds[i] = cgpu_create_command_buffer(pools[i], &cmd_desc);
        exec_fences[i] = cgpu_create_fence(device);
    }
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
        .imageCount = BACK_BUFFER_COUNT,
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
            .usages = TVU_RTV_DSV
        };
        views[i] = cgpu_create_texture_view(device, &view_desc);
    }
    create_render_resources();
    create_render_pipeline();
}

typedef struct PushConstants {
    float ColorMultiplier;
    uint32_t bFlipUVX;
    uint32_t bFlipUVY;
} PushConstants;

const static PushConstants data = {
    .ColorMultiplier = 0.5f,
    .bFlipUVX = 0,
    .bFlipUVY = 1
};

void raster_redraw()
{
    // sync & reset
    CGpuAcquireNextDescriptor acquire_desc = {
        .signal_semaphore = present_semaphore
    };
    backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    CGpuCommandPoolId pool = pools[backbuffer_index];
    CGpuCommandBufferId cmd = cmds[backbuffer_index];
    const CGpuTextureId back_buffer = swapchain->back_buffers[backbuffer_index];
    cgpu_wait_fences(exec_fences + backbuffer_index, 1);
    cgpu_reset_command_pool(pool);
    // record
    cgpu_cmd_begin(cmd);
    CGpuColorAttachment screen_attachment = {
        .view = views[backbuffer_index],
        .load_action = LOAD_ACTION_CLEAR,
        .store_action = STORE_ACTION_STORE,
        .clear_color = fastclear_0000
    };
    CGpuDepthStencilAttachment ds_attachment = {
        .view = NULL,
        .write_depth = false,
        .clear_depth = false,
        .write_stencil = false
    };
    CGpuRenderPassDescriptor rp_desc = {
        .render_target_count = 1,
        .sample_count = SAMPLE_COUNT_1,
        .color_attachments = &screen_attachment,
        .depth_stencil = &ds_attachment
    };
    CGpuTextureBarrier draw_barrier = {
        .texture = back_buffer,
        .src_state = RESOURCE_STATE_UNDEFINED,
        .dst_state = RESOURCE_STATE_RENDER_TARGET
    };
    CGpuResourceBarrierDescriptor barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc0);
    CGpuRenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);
    {
        cgpu_render_encoder_set_viewport(rp_encoder,
            0.0f, 0.0f,
            (float)back_buffer->width, (float)back_buffer->height,
            0.f, 1.f);
        cgpu_render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->width, back_buffer->height);
        cgpu_render_encoder_bind_pipeline(rp_encoder, pipeline);
        const uint32_t stride = sizeof(Vertex);
        cgpu_render_encoder_bind_vertex_buffers(rp_encoder, 1, &vertex_buffer, &stride, CGPU_NULLPTR);
        cgpu_render_encoder_bind_index_buffer(rp_encoder, index_buffer, 16, 0);
        cgpu_render_encoder_bind_descriptor_set(rp_encoder, desc_set);
        cgpu_render_encoder_push_constants(rp_encoder, root_sig, "root_constants", &data);
        if (desc_set2) cgpu_render_encoder_bind_descriptor_set(rp_encoder, desc_set2);
        cgpu_render_encoder_draw_indexed_instanced(rp_encoder, 6, 0, 1, 0, 0);
    }
    cgpu_cmd_end_render_pass(cmd, rp_encoder);
    CGpuTextureBarrier present_barrier = {
        .texture = back_buffer,
        .src_state = RESOURCE_STATE_RENDER_TARGET,
        .dst_state = RESOURCE_STATE_PRESENT
    };
    CGpuResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);
    cgpu_cmd_end(cmd);
    // submit
    CGpuQueueSubmitDescriptor submit_desc = {
        .cmds = &cmd,
        .cmds_count = 1,
        .signal_fence = exec_fences[backbuffer_index]
    };
    cgpu_submit_queue(gfx_queue, &submit_desc);
    // present
    CGpuQueuePresentDescriptor present_desc = {
        .index = backbuffer_index,
        .swapchain = swapchain,
        .wait_semaphore_count = 1,
        .wait_semaphores = &present_semaphore
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
    cgpu_wait_fences(exec_fences, FLIGHT_FRAMES);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        cgpu_free_command_buffer(cmds[i]);
        cgpu_free_command_pool(pools[i]);
        cgpu_free_fence(exec_fences[i]);
    }
    cgpu_free_semaphore(present_semaphore);
    cgpu_free_descriptor_set(desc_set);
    if (desc_set2) cgpu_free_descriptor_set(desc_set2);
    cgpu_free_buffer(vertex_buffer);
    cgpu_free_buffer(index_buffer);
    cgpu_free_sampler(sampler_state);
    cgpu_free_texture(sampled_texture);
    cgpu_free_texture_view(sampled_view);
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
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    // When we support more add them here
    ECGpuBackend backends[] = {
        CGPU_BACKEND_VULKAN
#ifdef CGPU_USE_D3D12
        ,
        CGPU_BACKEND_D3D12
#endif
    };
#if defined(__APPLE__) || defined(__EMSCRIPTEN__) || defined(__wasi__)
    ProgramMain(backends);
#else
    const uint32_t TEST_BACKEND_COUNT = sizeof(backends) / sizeof(ECGpuBackend);
    DECLARE_ZERO_VLA(SThreadHandle, hdls, TEST_BACKEND_COUNT)
    DECLARE_ZERO_VLA(SThreadDesc, thread_descs, TEST_BACKEND_COUNT)
    for (uint32_t i = 0; i < TEST_BACKEND_COUNT; i++)
    {
        thread_descs[i].pFunc = &ProgramMain;
        thread_descs[i].pData = &backends[i];
        skr_init_thread(&thread_descs[i], &hdls[i]);
    }
    for (uint32_t i = 0; i < TEST_BACKEND_COUNT; i++)
    {
        skr_join_thread(hdls[i]);
        skr_destroy_thread(hdls[i]);
    }
#endif
    SDL_Quit();

    return 0;
}