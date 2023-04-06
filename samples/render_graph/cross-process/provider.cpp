#include "platform/memory.h"
#include "platform/process.h"
#include "mdb_utils.h"
#include <platform/filesystem.hpp>
#include <containers/text.hpp>
#include "common/utils.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"

struct ProviderRenderer
{
    void create_window();
    void create_api_objects();
    void create_render_pipeline();
    void create_blit_pipeline();
    void finalize();

    SDL_Window* sdl_window;
    SDL_SysWMinfo wmInfo;
    CGPUSurfaceId surface;
    CGPUSwapChainId swapchain;
    uint32_t backbuffer_index;
    CGPUFenceId present_fence;

    #if _WIN32
    ECGPUBackend backend = CGPU_BACKEND_D3D12;
    #else
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
    #endif

    CGPUInstanceId instance;
    CGPUAdapterId adapter;
    CGPUDeviceId device;
    CGPUQueueId gfx_queue;
    CGPUSamplerId static_sampler;

    CGPURootSignatureId root_sig;
    CGPURootSignatureId blit_root_sig;
    CGPURenderPipelineId pipeline;
    CGPURenderPipelineId blit_pipeline;
};


void ProviderRenderer::create_window()
{
    auto title = skr::text::text::from_utf8(SKR_UTF8("Cross-Process Provider ["));
    title += gCGPUBackendNames[backend];
    title += SKR_UTF8("]");
    title += SKR_UTF8(" PID: ");
    title += skr::text::format(SKR_UTF8("{}"), skr_get_current_process_id());
    sdl_window = SDL_CreateWindow(title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);
}

void ProviderRenderer::create_api_objects()
{
    // Create instance
    CGPUInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = false;
    instance_desc.enable_set_name = true;
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    auto adapter_detail = cgpu_query_adapter_detail(adapter);
    SKR_LOG_TRACE("Adapter: %s", adapter_detail->vendor_preset.gpu_name);

    // Create device
    CGPUQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count = 1;
    CGPUDeviceDescriptor device_desc = {};
    device_desc.queue_groups = &queue_group_desc;
    device_desc.queue_group_count = 1;
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);
    // Sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    static_sampler = cgpu_create_sampler(device, &sampler_desc);

    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    surface = cgpu_surface_from_hwnd(device, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGPUNSView* ns_view = (struct CGPUNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.present_queues = &gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 3;
    chain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    chain_desc.enable_vsync = true;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
}

void ProviderRenderer::create_render_pipeline()
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes(u8"cross-process/vertex_shader", &vs_bytes, &vs_length, backend);
    read_shader_bytes(u8"cross-process/fragment_shader", &fs_bytes, &fs_length, backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.name = u8"VertexShaderLibrary";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = u8"FragmentShaderLibrary";
    ps_desc.stage = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUShaderEntryDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = u8"main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = u8"main";
    ppl_shaders[1].library = fragment_shader;
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    auto backend_format = (ECGPUFormat)swapchain->back_buffers[0]->format;
    rp_desc.color_formats = &backend_format;
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

void ProviderRenderer::create_blit_pipeline()
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes(u8"cross-process/screen_vs", &vs_bytes, &vs_length,
    device->adapter->instance->backend);
    read_shader_bytes(u8"cross-process/blit_fs", &fs_bytes, &fs_length,
    device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.name = u8"ScreenVertexShader";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = u8"BlitFragmentShader";
    ps_desc.stage = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    auto screen_vs = cgpu_create_shader_library(device, &vs_desc);
    auto blit_fs = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUShaderEntryDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = u8"main";
    ppl_shaders[0].library = screen_vs;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = u8"main";
    ppl_shaders[1].library = blit_fs;
    const char8_t* static_sampler_name = u8"texture_sampler";
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    blit_root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = blit_root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    auto backend_format = (ECGPUFormat)swapchain->back_buffers[0]->format;
    rp_desc.color_formats = &backend_format;
    blit_pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(screen_vs);
    cgpu_free_shader_library(blit_fs);
}

void ProviderRenderer::finalize()
{
    // Free cgpu objects
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_render_pipeline(blit_pipeline);
    cgpu_free_root_signature(root_sig);
    cgpu_free_root_signature(blit_root_sig);
    cgpu_free_sampler(static_sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

int provider_set_shared_handle(MDB_env* env, MDB_dbi dbi, SProcessId provider_id, CGPUImportTextureDescriptor info)
{
    // Open txn
    {
        // ZoneScopedN("MDBTransaction");
        MDB_txn* txn;
        if (const int rc = mdb_txn_begin(env, nullptr, 0, &txn)) 
        {
            SKR_LOG_ERROR("mdb_txn_begin failed: %d", rc);
        }
        // Txn body: write db
        {
            //Initialize the key with the key we're looking for
            skr::string keyString = skr::to_string(provider_id);
            MDB_val key = { (size_t)keyString.size(), (void*)keyString.data() };
            MDB_val data = { sizeof(info), (void*)&info };

            if (int rc = mdb_put(txn, dbi, &key, &data, 0))
            {
                SKR_LOG_ERROR("mdb_put failed: %d", rc);
            }
            else
            {
                SKR_LOG_TRACE("provider_set_shared_handle succeed: %d, proc: %s, shared_handle: %lld", rc, keyString.c_str(), info.shared_handle);
            }
            if (int rc = mdb_txn_commit(txn))
            {
                SKR_LOG_ERROR("mdb_txn_commit failed: %d", rc);
            }
        }
    }
    return 0;
}

int provider_main(int argc, char* argv[])
{
    // report process information
    auto id = skr_get_current_process_id();
    SKR_LOG_DEBUG("exec_mode: %s, process id: %lld", argv[1], id);
    const SProcessId provider_id = skr_get_current_process_id();
    SKR_LOG_TRACE("provider id: %d", provider_id);

    // initialize db
    MDB_env* env = nullptr;
    env_create(&env);
    MDB_dbi dbi;
    dbi_create(env, &dbi, false);
    SKR_LOG_TRACE("db id: %u", dbi);

    // initialize renderer
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    auto renderer = SkrNew<ProviderRenderer>();
    if (argc >= 3)
    {
        renderer->backend = (strcmp(argv[2], "--d3d12") == 0) ? CGPU_BACKEND_D3D12 : renderer->backend;
        renderer->backend = (strcmp(argv[2], "--vulkan") == 0) ? CGPU_BACKEND_VULKAN : renderer->backend;
    }
    renderer->create_window();
    renderer->create_api_objects();
    renderer->create_render_pipeline();
    renderer->create_blit_pipeline();

    // initialize render graph
    namespace render_graph = skr::render_graph;
    auto graph = render_graph::RenderGraph::create(
    [=](render_graph::RenderGraphBuilder& builder) {
        builder.with_device(renderer->device)
            .with_gfx_queue(renderer->gfx_queue);
    });

    // loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(renderer->sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, renderer->sdl_window))
                {
                    quit = true;
                }
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                Uint8 window_event = event.window.event;
                if (window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    cgpu_wait_queue_idle(renderer->gfx_queue);
                    cgpu_free_swapchain(renderer->swapchain);
                    int width = 0, height = 0;
                    SDL_GetWindowSize(renderer->sdl_window, &width, &height);
                    CGPUSwapChainDescriptor chain_desc = {};
                    chain_desc.present_queues = &renderer->gfx_queue;
                    chain_desc.present_queues_count = 1;
                    chain_desc.width = width;
                    chain_desc.height = height;
                    chain_desc.surface = renderer->surface;
                    chain_desc.imageCount = 3;
                    chain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
                    chain_desc.enable_vsync = true;
                    renderer->swapchain = cgpu_create_swapchain(renderer->device, &chain_desc);
                }
            }
        }
        // acquire frame
        cgpu_wait_fences(&renderer->present_fence, 1);
        CGPUAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = renderer->present_fence;
        renderer->backbuffer_index = cgpu_acquire_next_image(renderer->swapchain, &acquire_desc);
        // render graph setup & compile & exec
        CGPUTextureId to_import = renderer->swapchain->back_buffers[renderer->backbuffer_index];
        auto back_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"backbuffer")
                .import(to_import, CGPU_RESOURCE_STATE_UNDEFINED)
                .allow_render_target();
            });
        auto target_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(u8"target_buffer")
                .extent(to_import->width, to_import->height)
                .format((ECGPUFormat)to_import->format)
                .with_flags(CGPU_TCF_EXPORT_BIT)
                .owns_memory()
                .allow_render_target();
            });
        graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name(u8"color_pass")
                    .set_pipeline(renderer->pipeline)
                    .write(0, target_buffer, CGPU_LOAD_ACTION_CLEAR);
            },
            [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)to_import->width, (float)to_import->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, to_import->width, to_import->height);
                cgpu_render_encoder_draw(stack.encoder, 3, 0);
            });
        graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name(u8"final_blit")
                    .set_pipeline(renderer->blit_pipeline)
                    .read(u8"input_color", target_buffer)
                    .write(0, back_buffer, CGPU_LOAD_ACTION_CLEAR);
            },
            [=](render_graph::RenderGraph& g, render_graph::RenderPassContext& stack) {
                static CGPUTextureId cached_shared_texture = nullptr;
                if (auto shared_texture = stack.resolve(target_buffer);shared_texture != cached_shared_texture)
                {
                    CGPUExportTextureDescriptor export_desc = {};
                    export_desc.texture = shared_texture;
                    auto shared_handle = cgpu_export_shared_texture_handle(renderer->device, &export_desc);
                    SKR_LOG_TRACE("shared texture handle exported: %p", shared_handle);
                    cached_shared_texture = shared_texture;
                    CGPUImportTextureDescriptor import_info = {};
                    import_info.backend = renderer->backend;
                    import_info.shared_handle = shared_handle;
                    import_info.width = shared_texture->width;
                    import_info.height = shared_texture->height;
                    import_info.depth = shared_texture->depth;
                    import_info.is_dedicated = shared_texture->is_dedicated;
                    import_info.format = (ECGPUFormat)shared_texture->format;
                    import_info.mip_levels = shared_texture->mip_levels;
                    import_info.size_in_bytes = shared_texture->size_in_bytes;
                    provider_set_shared_handle(env, dbi, provider_id, import_info);
                }

                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)to_import->width, (float)to_import->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, to_import->width, to_import->height);
                cgpu_render_encoder_draw(stack.encoder, 6, 0);
            });
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name(u8"present")
                .swapchain(renderer->swapchain, renderer->backbuffer_index)
                .texture(back_buffer, true);
            });
        graph->compile();
        graph->execute();
        // present
        cgpu_wait_queue_idle(renderer->gfx_queue);
        CGPUQueuePresentDescriptor present_desc = {};
        present_desc.index = renderer->backbuffer_index;
        present_desc.swapchain = renderer->swapchain;
        cgpu_queue_present(renderer->gfx_queue, &present_desc);
    }
    render_graph::RenderGraph::destroy(graph);
    // clean up
    renderer->finalize();
    SDL_DestroyWindow(renderer->sdl_window);
    SkrDelete(renderer);
    SDL_Quit();
    // quit db
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    return 0;
}

