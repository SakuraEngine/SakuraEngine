#include "SkrGraphics/api.h"
#include "common/render_application.h"
#include "common/texture.h"
#include "math.h"
#include "string.h"
#include "SkrOS/thread.h"
#include "SkrCore/memory/memory.h"

#define FLIGHT_FRAMES 3
#define BACK_BUFFER_COUNT 3
CGPUSemaphoreId      present_semaphore;
CGPUFenceId          exec_fences[FLIGHT_FRAMES];
CGPURootSignatureId  root_sig;
CGPUDescriptorSetId  desc_set;
CGPUDescriptorSetId  desc_set2; // We use this for samplers under D3D12
CGPURenderPipelineId pipeline;
CGPUCommandPoolId    pools[FLIGHT_FRAMES];
CGPUCommandBufferId  cmds[FLIGHT_FRAMES];
CGPUTextureId        sampled_texture;
CGPUSamplerId        sampler_state;
bool                 bUseStaticSampler = true;
bool                 bUseTiledCopy     = true;
CGPUTextureViewId    views[BACK_BUFFER_COUNT];

#define TOTAL_MIPS 3
CGPUBufferId      streaming_heap;
const uint32_t    resident_mip = 0;
uint32_t          mapped_mip   = ~0;
CGPUTextureViewId sampled_views[TOTAL_MIPS];

render_application_t App;

static const uint32_t TILE_WIDTH  = 128;
static const uint32_t TILE_HEIGHT = 128;

void create_sampled_texture()
{
    // Sampler
    CGPUSamplerDescriptor sampler_desc = {
        .address_u    = CGPU_ADDRESS_MODE_REPEAT,
        .address_v    = CGPU_ADDRESS_MODE_REPEAT,
        .address_w    = CGPU_ADDRESS_MODE_REPEAT,
        .mipmap_mode  = CGPU_MIPMAP_MODE_LINEAR,
        .min_filter   = CGPU_FILTER_TYPE_LINEAR,
        .mag_filter   = CGPU_FILTER_TYPE_LINEAR,
        .compare_func = CGPU_CMP_NEVER
    };
    sampler_state = cgpu_create_sampler(App.device, &sampler_desc);
    // Texture
    CGPUTextureDescriptor tex_desc = {
        .descriptors = CGPU_RESOURCE_TYPE_TEXTURE,
        .flags       = CGPU_TCF_TILED_RESOURCE,
        .width       = TEXTURE_WIDTH,
        .height      = TEXTURE_HEIGHT,
        .depth       = 1,
        .mip_levels  = TOTAL_MIPS,
        .format      = CGPU_FORMAT_R8G8B8A8_UNORM,
        .array_size  = 1,
        .owner_queue = App.gfx_queue,
        .start_state = CGPU_RESOURCE_STATE_COPY_DEST
    };
    sampled_texture = cgpu_create_texture(App.device, &tex_desc);
    for (uint32_t i = 0; i < TOTAL_MIPS; i++)
    {
        CGPUTextureViewDescriptor sview_desc = {
            .texture           = sampled_texture,
            .format            = tex_desc.format,
            .array_layer_count = 1,
            .base_array_layer  = 0,
            .mip_level_count   = 1,
            .base_mip_level    = i,
            .aspects           = CGPU_TVA_COLOR,
            .dims              = CGPU_TEX_DIMENSION_2D,
            .usages            = CGPU_TVU_SRV
        };
        sampled_views[i] = cgpu_create_texture_view(App.device, &sview_desc);
    }
    CGPUBufferDescriptor upload_buffer_desc = {
        .name           = "UploadBuffer",
        .flags          = CGPU_BCF_PERSISTENT_MAP_BIT,
        .descriptors    = CGPU_RESOURCE_TYPE_NONE,
        .memory_usage   = CGPU_MEM_USAGE_CPU_ONLY,
        .element_stride = sizeof(TEXTURE_DATA),
        .element_count   = 1,
        .size           = sizeof(TEXTURE_DATA)
    };
    streaming_heap = cgpu_create_buffer(App.device, &upload_buffer_desc);
    if (bUseTiledCopy)
    {
        const uint32_t TEXEL_SIZE            = 4 * sizeof(uint8_t);
        uint8_t*       SWIZZLED_TEXTURE_DATA = (uint8_t*)sakura_malloc(sizeof(TEXTURE_DATA));
        // swizzle linear row data to tile blocks
        const uint32_t TILE_COUNT_PER_ROW = TEXTURE_WIDTH / TILE_WIDTH;
        const uint32_t TILE_COUNT         = TILE_WIDTH * TILE_HEIGHT;
        for (uint32_t y = 0; y < TEXTURE_HEIGHT; y += TILE_HEIGHT)
        {
            const uint32_t TILE_Y = y / TILE_HEIGHT;
            for (uint32_t x = 0; x < TEXTURE_WIDTH; x += TILE_WIDTH)
            {
                const uint32_t TILE_X = x / TILE_WIDTH;
                for (uint32_t ty = 0; ty < TILE_HEIGHT; ty++)
                {
                    uint32_t src_index = (y + ty) * TEXTURE_WIDTH + x;
                    uint32_t dst_index = TILE_Y * TILE_COUNT_PER_ROW * TILE_COUNT + TILE_X * TILE_COUNT + ty * TILE_WIDTH;
                    memcpy(SWIZZLED_TEXTURE_DATA + dst_index * TEXEL_SIZE, TEXTURE_DATA + src_index * TEXEL_SIZE, TEXEL_SIZE * TILE_WIDTH);
                }
            }
        }
        memcpy(streaming_heap->info->cpu_mapped_address, SWIZZLED_TEXTURE_DATA, upload_buffer_desc.size);
        sakura_free(SWIZZLED_TEXTURE_DATA);
    }
    else
    {
        memcpy(streaming_heap->info->cpu_mapped_address, TEXTURE_DATA, upload_buffer_desc.size);
    }
}

void update_streaming_unmap(CGPUCommandBufferId cmd, uint32_t MipLevel)
{
    if (mapped_mip == MipLevel)
        return;
    // cgpu_queue_unmap_tiled_texture(cmd, mapped_mip);
}

void update_streaming_map(CGPUCommandBufferId cmd, uint32_t MipLevel)
{
    if (mapped_mip == MipLevel)
        return;
    const CGPUTiledSubresourceInfo subres = sampled_texture->tiled_resource->subresources[MipLevel];

    CGPUTiledTexturePackedMip packed_update = {
        .texture = sampled_texture,
        .layer   = 0
    };
    CGPUTiledTexturePackedMips packed_updates = {
        .packed_mips      = &packed_update,
        .packed_mip_count = 1
    };
    cgpu_queue_map_packed_mips(App.gfx_queue, &packed_updates);

    // map tiled texture
    if (MipLevel == 1)
    {
        CGPUTextureCoordinateRegion coordinate = {
            .start     = { 0, 0, 0 },
            .end       = { 1, 1, 1 },
            .mip_level = MipLevel,
            .layer     = 0
        };
        CGPUTiledTextureRegions mapping = {
            .texture      = sampled_texture,
            .regions      = &coordinate,
            .region_count = 1
        };
        cgpu_queue_map_tiled_texture(App.gfx_queue, &mapping);
    }
    else
    {
        CGPUTextureCoordinateRegion coordinates[3] = {
            { .start     = { 0, 0, 0 },
              .end       = { subres.width_in_tiles / 2, subres.height_in_tiles, subres.depth_in_tiles },
              .mip_level = MipLevel,
              .layer     = 0 },
            { .start     = { 0, 0, 0 },
              .end       = { subres.width_in_tiles / 2, subres.height_in_tiles / 2, subres.depth_in_tiles },
              .mip_level = MipLevel,
              .layer     = 0 },
            { .start     = { subres.width_in_tiles / 2, 0, 0 },
              .end       = { subres.width_in_tiles, subres.height_in_tiles / 2, subres.depth_in_tiles },
              .mip_level = MipLevel,
              .layer     = 0 }
        };
        {
            SkrCZone(z, 1);
            SkrCZoneName(z, "map", strlen("map"));
            CGPUTiledTextureRegions mapping = {
                .texture      = sampled_texture,
                .regions      = coordinates,
                .region_count = 1
            };
            cgpu_queue_map_tiled_texture(App.gfx_queue, &mapping);
            SkrCZoneEnd(z);
        }
        {
            SkrCZone(z, 1);
            SkrCZoneName(z, "unmap", strlen("unmap"));
            CGPUTiledTextureRegions unmapping = {
                .texture      = sampled_texture,
                .regions      = coordinates + 1,
                .region_count = 1
            };
            cgpu_queue_unmap_tiled_texture(App.gfx_queue, &unmapping);
            SkrCZoneEnd(z);
        }
        {
            SkrCZone(z, 1);
            SkrCZoneName(z, "map", strlen("map"));
            CGPUTiledTextureRegions mapping1 = {
                .texture      = sampled_texture,
                .regions      = coordinates + 2,
                .region_count = 1
            };
            cgpu_queue_map_tiled_texture(App.gfx_queue, &mapping1);
            SkrCZoneEnd(z);
        }
    }
    mapped_mip = MipLevel;
    // record
    if (bUseTiledCopy)
    {
        CGPUBufferToTilesTransfer b2t = {
            .src        = streaming_heap,
            .src_offset = 0,
            .dst        = sampled_texture,
            .region     = {
                    .start     = { 0, 0, 0 },
                    .end       = { subres.width_in_tiles, subres.height_in_tiles, subres.depth_in_tiles },
                    .mip_level = MipLevel,
                    .layer     = 0 }
        };
        cgpu_cmd_transfer_buffer_to_tiles(cmd, &b2t);
    }
    else
    {
        CGPUBufferToTextureTransfer b2t = {
            .src                              = streaming_heap,
            .src_offset                       = 0,
            .dst                              = sampled_texture,
            .dst_subresource.mip_level        = MipLevel,
            .dst_subresource.base_array_layer = 0,
            .dst_subresource.layer_count      = 1
        };
        cgpu_cmd_transfer_buffer_to_texture(cmd, &b2t);
    }
    CGPUTextureBarrier srv_barrier = {
        .texture   = sampled_texture,
        .src_state = CGPU_RESOURCE_STATE_COPY_DEST,
        .dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE
    };
    CGPUResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &srv_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);
}

void update_streaming(CGPUCommandBufferId cmd, uint32_t MipLevel)
{
    update_streaming_unmap(cmd, mapped_mip);
    update_streaming_map(cmd, MipLevel);
}

void create_render_pipeline()
{
    // Shaders
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("cgpu-texture/vertex_shader", &vs_bytes, &vs_length, App.backend);
    read_shader_bytes("cgpu-texture/fragment_shader", &fs_bytes, &fs_length, App.backend);
    CGPUShaderLibraryDescriptor vs_desc = {
        .name      = "VertexShaderLibrary",
        .stage     = CGPU_SHADER_STAGE_VERT,
        .code      = vs_bytes,
        .code_size = vs_length
    };
    CGPUShaderLibraryDescriptor ps_desc = {
        .name      = "FragmentShaderLibrary",
        .stage     = CGPU_SHADER_STAGE_FRAG,
        .code      = fs_bytes,
        .code_size = fs_length
    };
    CGPUShaderLibraryId vertex_shader   = cgpu_create_shader_library(App.device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(App.device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    // Create RS
    CGPUShaderEntryDescriptor ppl_shaders[2];
    ppl_shaders[0].stage                           = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry                           = "main";
    ppl_shaders[0].library                         = vertex_shader;
    ppl_shaders[1].stage                           = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry                           = "main";
    ppl_shaders[1].library                         = fragment_shader;
    const char8_t*              sampler_name       = "texture_sampler";
    const char8_t*              push_constant_name = "push_constants";
    CGPURootSignatureDescriptor rs_desc            = {
                   .shaders             = ppl_shaders,
                   .shader_count        = 2,
                   .push_constant_names = &push_constant_name,
                   .push_constant_count = 1
    };
    if (bUseStaticSampler)
    {
        rs_desc.static_samplers      = &sampler_state;
        rs_desc.static_sampler_count = 1;
        rs_desc.static_sampler_names = &sampler_name;
    }
    root_sig = cgpu_create_root_signature(App.device, &rs_desc);
    // Create descriptor set
    CGPUDescriptorSetDescriptor desc_set_desc = {
        .root_signature = root_sig,
        .set_index      = 0
    };
    desc_set = cgpu_create_descriptor_set(App.device, &desc_set_desc);
    if (!bUseStaticSampler)
    {
        desc_set_desc.set_index = 1;
        desc_set2               = cgpu_create_descriptor_set(App.device, &desc_set_desc);
    }
    CGPUVertexLayout             vertex_layout = { .attribute_count = 0 };
    CGPURenderPipelineDescriptor rp_desc       = {
              .root_signature      = root_sig,
              .prim_topology       = CGPU_PRIM_TOPO_TRI_LIST,
              .vertex_layout       = &vertex_layout,
              .vertex_shader       = &ppl_shaders[0],
              .fragment_shader     = &ppl_shaders[1],
              .render_target_count = 1,
              .color_formats       = &views[0]->info.format
    };
    pipeline = cgpu_create_render_pipeline(App.device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    // Update descriptor set for once
    CGPUDescriptorData arguments[2];
    arguments[0].name = "sampled_texture";
    // via binding: arguments[0].binding = 0;
    arguments[0].count    = 1;
    arguments[0].textures = &sampled_views[resident_mip];
    arguments[1].name     = sampler_name;
    // via binding: arguments[1].binding = 1;
    arguments[1].count    = 1;
    arguments[1].samplers = &sampler_state;
    {
        cgpu_update_descriptor_set(desc_set, arguments, 1);
        if (!bUseStaticSampler) cgpu_update_descriptor_set(desc_set2, arguments + 1, 1);
    }
}

void initialize(void* usrdata)
{
    App.backend      = *(ECGPUBackend*)usrdata;
    App.window_title = gCGPUBackendNames[App.backend];
    app_create_window(&App, BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT);
    app_create_gfx_objects(&App);

    present_semaphore = cgpu_create_semaphore(App.device);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        pools[i]                             = cgpu_create_command_pool(App.gfx_queue, CGPU_NULLPTR);
        CGPUCommandBufferDescriptor cmd_desc = { .is_secondary = false };
        cmds[i]                              = cgpu_create_command_buffer(pools[i], &cmd_desc);
        exec_fences[i]                       = cgpu_create_fence(App.device);
    }
    // Create views
    for (uint32_t i = 0; i < App.swapchain->buffer_count; i++)
    {
        CGPUTextureViewDescriptor view_desc = {
            .texture           = App.swapchain->back_buffers[i],
            .aspects           = CGPU_TVA_COLOR,
            .dims              = CGPU_TEX_DIMENSION_2D,
            .format            = App.swapchain->back_buffers[i]->info->format,
            .usages            = CGPU_TVU_RTV_DSV,
            .array_layer_count = 1
        };
        views[i] = cgpu_create_texture_view(App.device, &view_desc);
    }
    create_sampled_texture();
    create_render_pipeline();
}

typedef struct PushConstants {
    float    ColorMultiplier;
    uint32_t bFlipUVX;
    uint32_t bFlipUVY;
} PushConstants;

const static PushConstants data = {
    .ColorMultiplier = 0.5f,
    .bFlipUVX        = 0,
    .bFlipUVY        = 1
};

void raster_redraw()
{
    // sync & reset
    CGPUAcquireNextDescriptor acquire_desc = {
        .signal_semaphore = present_semaphore
    };
    App.backbuffer_index            = cgpu_acquire_next_image(App.swapchain, &acquire_desc);
    CGPUCommandPoolId   pool        = pools[App.backbuffer_index];
    CGPUCommandBufferId cmd         = cmds[App.backbuffer_index];
    const CGPUTextureId back_buffer = App.swapchain->back_buffers[App.backbuffer_index];
    cgpu_wait_fences(exec_fences + App.backbuffer_index, 1);
    cgpu_reset_command_pool(pool);
    // record
    cgpu_cmd_begin(cmd);
    update_streaming_map(cmd, resident_mip);
    CGPUColorAttachment screen_attachment = {
        .view         = views[App.backbuffer_index],
        .load_action  = CGPU_LOAD_ACTION_CLEAR,
        .store_action = CGPU_STORE_ACTION_STORE,
        .clear_color  = fastclear_0000
    };
    CGPURenderPassDescriptor rp_desc = {
        .render_target_count = 1,
        .sample_count        = CGPU_SAMPLE_COUNT_1,
        .color_attachments   = &screen_attachment,
        .depth_stencil       = CGPU_NULLPTR
    };
    CGPUTextureBarrier draw_barrier = {
        .texture   = back_buffer,
        .src_state = CGPU_RESOURCE_STATE_UNDEFINED,
        .dst_state = CGPU_RESOURCE_STATE_RENDER_TARGET
    };
    CGPUResourceBarrierDescriptor barrier_desc0 = { .texture_barriers = &draw_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc0);
    CGPURenderPassEncoderId rp_encoder = cgpu_cmd_begin_render_pass(cmd, &rp_desc);
    {
        cgpu_render_encoder_set_viewport(rp_encoder, 0.0f, 0.0f, (float)back_buffer->info->width, (float)back_buffer->info->height, 0.f, 1.f);
        cgpu_render_encoder_set_scissor(rp_encoder, 0, 0, (uint32_t)back_buffer->info->width, (uint32_t)back_buffer->info->height);
        cgpu_render_encoder_bind_pipeline(rp_encoder, pipeline);
        cgpu_render_encoder_bind_descriptor_set(rp_encoder, desc_set);
        cgpu_render_encoder_push_constants(rp_encoder, root_sig, "push_constants", &data);
        if (desc_set2) cgpu_render_encoder_bind_descriptor_set(rp_encoder, desc_set2);
        cgpu_render_encoder_draw(rp_encoder, 6, 0);
    }
    cgpu_cmd_end_render_pass(cmd, rp_encoder);
    CGPUTextureBarrier present_barrier = {
        .texture   = back_buffer,
        .src_state = CGPU_RESOURCE_STATE_RENDER_TARGET,
        .dst_state = CGPU_RESOURCE_STATE_PRESENT
    };
    CGPUResourceBarrierDescriptor barrier_desc1 = { .texture_barriers = &present_barrier, .texture_barriers_count = 1 };
    cgpu_cmd_resource_barrier(cmd, &barrier_desc1);
    cgpu_cmd_end(cmd);
    // submit
    CGPUQueueSubmitDescriptor submit_desc = {
        .cmds         = &cmd,
        .cmds_count   = 1,
        .signal_fence = exec_fences[App.backbuffer_index]
    };
    cgpu_submit_queue(App.gfx_queue, &submit_desc);
    // present
    CGPUQueuePresentDescriptor present_desc = {
        .index                = App.backbuffer_index,
        .swapchain            = App.swapchain,
        .wait_semaphore_count = 1,
        .wait_semaphores      = &present_semaphore
    };
    cgpu_queue_present(App.gfx_queue, &present_desc);
}

void raster_program()
{
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            SDL_Window* sdl_window = (SDL_Window*)App.sdl_window;
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    quit = true;
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED)
                {
                    cgpu_wait_queue_idle(App.gfx_queue);
                    int width = 0, height = 0;
                    SDL_GetWindowSize(sdl_window, &width, &height);
                    CGPUSwapChainDescriptor descriptor = {
                        .present_queues       = &App.gfx_queue,
                        .present_queues_count = 1,
                        .width                = width,
                        .height               = height,
                        .surface              = App.surface,
                        .image_count          = BACK_BUFFER_COUNT,
                        .format               = CGPU_FORMAT_R8G8B8A8_UNORM,
                        .enable_vsync         = true
                    };
                    cgpu_free_swapchain(App.swapchain);
                    App.swapchain = cgpu_create_swapchain(App.device, &descriptor);
                    // Create views
                    for (uint32_t i = 0; i < App.swapchain->buffer_count; i++)
                    {
                        cgpu_free_texture_view(views[i]);
                        CGPUTextureViewDescriptor view_desc = {
                            .texture           = App.swapchain->back_buffers[i],
                            .aspects           = CGPU_TVA_COLOR,
                            .dims              = CGPU_TEX_DIMENSION_2D,
                            .format            = App.swapchain->back_buffers[i]->info->format,
                            .usages            = CGPU_TVU_RTV_DSV,
                            .array_layer_count = 1
                        };
                        views[i] = cgpu_create_texture_view(App.device, &view_desc);
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
    cgpu_wait_fences(exec_fences, FLIGHT_FRAMES);
    cgpu_free_buffer(streaming_heap);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        cgpu_free_command_buffer(cmds[i]);
        cgpu_free_command_pool(pools[i]);
        cgpu_free_fence(exec_fences[i]);
    }
    cgpu_wait_queue_idle(App.gfx_queue);
    cgpu_free_semaphore(present_semaphore);
    cgpu_free_descriptor_set(desc_set);
    if (desc_set2) cgpu_free_descriptor_set(desc_set2);
    cgpu_free_sampler(sampler_state);
    cgpu_free_texture(sampled_texture);
    for (uint32_t i = 0; i < TOTAL_MIPS; i++)
        cgpu_free_texture_view(sampled_views[i]);
    for (uint32_t i = 0; i < App.swapchain->buffer_count; i++)
        cgpu_free_texture_view(views[i]);
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(root_sig);
    app_finalize(&App);
}

void ProgramMain(void* usrdata)
{
    initialize(usrdata);
    raster_program();
    finalize();
}

int main(int argc, char* argv[])
{
    // When we support more add them here
#ifdef CGPU_USE_D3D12
    ECGPUBackend backend = CGPU_BACKEND_D3D12;
#else
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
#endif
    ProgramMain(&backend);

    return 0;
}