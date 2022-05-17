#include "../common/utils.h"
#include "../common/texture.h"
#include "math.h"
#include "cgpu/api.h"
#include "platform/thread.h"

#define FLIGHT_FRAMES 3
#define BACK_BUFFER_COUNT 3
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
CGPUDescriptorSetId desc_set2; // We use this for samplers under D3D12
CGPURenderPipelineId pipeline;
CGPUCommandPoolId pools[FLIGHT_FRAMES];
CGPUCommandBufferId cmds[FLIGHT_FRAMES];
CGPUTextureId sampled_texture;
CGPUSamplerId sampler_state;
bool bUseStaticSampler = true;
CGPUTextureViewId sampled_view;
CGPUTextureViewId views[BACK_BUFFER_COUNT];

void create_sampled_texture()
{
    // Sampler
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
    // Texture
    CGPUTextureDescriptor tex_desc = {
        .descriptors = CGPU_RESOURCE_TYPE_TEXTURE,
        .flags = CGPU_TCF_OWN_MEMORY_BIT,
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .depth = 1,
        .format = CGPU_FORMAT_R8G8B8A8_UNORM,
        .array_size = 1,
        .owner_queue = gfx_queue,
        .start_state = CGPU_RESOURCE_STATE_COPY_DEST
    };
    sampled_texture = cgpu_create_texture(device, &tex_desc);
    CGPUTextureViewDescriptor sview_desc = {
        .texture = sampled_texture,
        .format = tex_desc.format,
        .array_layer_count = 1,
        .base_array_layer = 0,
        .mip_level_count = 1,
        .base_mip_level = 0,
        .aspects = CGPU_TVA_COLOR,
        .dims = CGPU_TEX_DIMENSION_2D,
        .usages = CGPU_TVU_SRV
    };
    sampled_view = cgpu_create_texture_view(device, &sview_desc);
    CGPUBufferDescriptor upload_buffer_desc = {
        .name = "UploadBuffer",
        .flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT,
        .descriptors = CGPU_RESOURCE_TYPE_NONE,
        .memory_usage = CGPU_MEM_USAGE_CPU_ONLY,
        .element_stride = sizeof(TEXTURE_DATA),
        .elemet_count = 1,
        .size = sizeof(TEXTURE_DATA)
    };
    CGPUBufferId upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    {
        memcpy(upload_buffer->cpu_mapped_address, TEXTURE_DATA, upload_buffer_desc.size);
    }
    cgpu_reset_command_pool(pools[0]);
    // record
    cgpu_cmd_begin(cmds[0]);
    CGPUBufferToTextureTransfer b2t = {
        .src = upload_buffer,
        .src_offset = 0,
        .dst = sampled_texture,
        .dst_subresource.mip_level = 0,
        .dst_subresource.base_array_layer = 0,
        .dst_subresource.layer_count = 1
    };
    cgpu_cmd_transfer_buffer_to_texture(cmds[0], &b2t);
    CGPUTextureBarrier srv_barrier = {
        .texture = sampled_texture,
        .src_state = CGPU_RESOURCE_STATE_COPY_DEST,
        .dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE
    };
    CGPUResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &srv_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmds[0], &barrier_desc1);
    cgpu_cmd_end(cmds[0]);
    CGPUQueueSubmitDescriptor cpy_submit = { .cmds = cmds, .cmds_count = 1 };
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
}

void create_render_pipeline()
{
    // Shaders
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("cgpu-texture/vertex_shader", &vs_bytes, &vs_length, backend);
    read_shader_bytes("cgpu-texture/fragment_shader", &fs_bytes, &fs_length, backend);
    CGPUShaderLibraryDescriptor vs_desc = {
        .name = "VertexShaderLibrary",
        .stage = CGPU_SHADER_STAGE_VERT,
        .code = vs_bytes,
        .code_size = vs_length
    };
    CGPUShaderLibraryDescriptor ps_desc = {
        .name = "FragmentShaderLibrary",
        .stage = CGPU_SHADER_STAGE_FRAG,
        .code = fs_bytes,
        .code_size = fs_length
    };
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    // Create RS
    CGPUPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    const char8_t* sampler_name = "texture_sampler";
    const char8_t* root_constant_name = "root_constants";
    CGPURootSignatureDescriptor rs_desc = {
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
    CGPUDescriptorSetDescriptor desc_set_desc = {
        .root_signature = root_sig,
        .set_index = 0
    };
    desc_set = cgpu_create_descriptor_set(device, &desc_set_desc);
    if (!bUseStaticSampler)
    {
        desc_set_desc.set_index = 1;
        desc_set2 = cgpu_create_descriptor_set(device, &desc_set_desc);
    }
    CGPUVertexLayout vertex_layout = { .attribute_count = 0 };
    CGPURenderPipelineDescriptor rp_desc = {
        .root_signature = root_sig,
        .prim_topology = CGPU_PRIM_TOPO_TRI_LIST,
        .vertex_layout = &vertex_layout,
        .vertex_shader = &ppl_shaders[0],
        .fragment_shader = &ppl_shaders[1],
        .render_target_count = 1,
        .color_formats = &views[0]->info.format
    };
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    // Update descriptor set for once
    CGPUDescriptorData arguments[2];
    arguments[0].name = "sampled_texture";
    // via binding: arguments[0].binding = 0;
    arguments[0].count = 1;
    arguments[0].textures = &sampled_view;
    arguments[1].name = sampler_name;
    // via binding: arguments[1].binding = 1;
    arguments[1].count = 1;
    arguments[1].samplers = &sampler_state;
    {
        cgpu_update_descriptor_set(desc_set, arguments, 1);
        if (!bUseStaticSampler) cgpu_update_descriptor_set(desc_set2, arguments + 1, 1);
    }
}

void initialize(void* usrdata)
{
    backend = *(ECGPUBackend*)usrdata;

    // Create window
    SDL_SysWMinfo wmInfo;
    sdl_window = SDL_CreateWindow(gCGPUBackendNames[backend],
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);

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
    DECLARE_ZERO_VLA(CGPUAdapterId, adapters, adapters_count);
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor G = {
        .queueType = CGPU_QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    CGPUDeviceDescriptor device_desc = {
        .queueGroups = &G,
        .queueGroupCount = 1
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
    surface = cgpu_surface_from_hwnd(device, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGPUNSView* ns_view = (struct CGPUNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGPUSwapChainDescriptor descriptor = {
        .presentQueues = &gfx_queue,
        .presentQueuesCount = 1,
        .width = BACK_BUFFER_WIDTH,
        .height = BACK_BUFFER_HEIGHT,
        .surface = surface,
        .imageCount = BACK_BUFFER_COUNT,
        .format = CGPU_FORMAT_R8G8B8A8_UNORM,
        .enableVsync = true
    };
    swapchain = cgpu_create_swapchain(device, &descriptor);
    // Create views
    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        CGPUTextureViewDescriptor view_desc = {
            .texture = swapchain->back_buffers[i],
            .aspects = CGPU_TVA_COLOR,
            .dims = CGPU_TEX_DIMENSION_2D,
            .format = swapchain->back_buffers[i]->format,
            .usages = CGPU_TVU_RTV_DSV
        };
        views[i] = cgpu_create_texture_view(device, &view_desc);
    }
    create_sampled_texture();
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
    CGPUAcquireNextDescriptor acquire_desc = {
        .signal_semaphore = present_semaphore
    };
    backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    CGPUCommandPoolId pool = pools[backbuffer_index];
    CGPUCommandBufferId cmd = cmds[backbuffer_index];
    const CGPUTextureId back_buffer = swapchain->back_buffers[backbuffer_index];
    cgpu_wait_fences(exec_fences + backbuffer_index, 1);
    cgpu_reset_command_pool(pool);
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
        cgpu_render_encoder_set_viewport(rp_encoder,
        0.0f, 0.0f,
        (float)back_buffer->width, (float)back_buffer->height,
        0.f, 1.f);
        cgpu_render_encoder_set_scissor(rp_encoder, 0, 0, back_buffer->width, back_buffer->height);
        cgpu_render_encoder_bind_pipeline(rp_encoder, pipeline);
        cgpu_render_encoder_bind_descriptor_set(rp_encoder, desc_set);
        cgpu_render_encoder_push_constants(rp_encoder, root_sig, "root_constants", &data);
        if (desc_set2) cgpu_render_encoder_bind_descriptor_set(rp_encoder, desc_set2);
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
#ifdef CGPU_USE_D3D12
    ECGPUBackend backend = CGPU_BACKEND_D3D12;
#else
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
#endif
    ProgramMain(&backend);
    SDL_Quit();

    return 0;
}