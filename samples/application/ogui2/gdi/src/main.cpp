#include "../../../../common/render_application.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrGui/gdi.h"
#include "SkrGuiRenderer/renderer.hpp"
#include "platform/filesystem.hpp"
#include "platform/vfs.h"

render_application_t App;
skr_vfs_t* resource_vfs = nullptr;
skr_io_ram_service_t* ram_service = nullptr;
skr_io_vram_service_t* vram_service = nullptr;
skr_threaded_service_t* aux_service = nullptr;

skr::gdi::SGDIDevice* gdi_device = nullptr;
skr::gdi::SGDICanvas* gdi_canvas = nullptr;
skr::gdi::SGDICanvasGroup* gdi_canvas_group = nullptr;
skr::gdi::SGDIRenderer_RenderGraph* gdi_renderer = nullptr;

skr::gdi::SGDICanvas* background_canvas = nullptr;
skr::gdi::SGDIElement* background_element = nullptr;

void initialize_renderer()
{
    {
        std::error_code ec = {};
        auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources");
        auto u8ResourceRoot = resourceRoot.u8string();
        skr_vfs_desc_t vfs_desc = {};
        vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
        vfs_desc.override_mount_dir = u8ResourceRoot.c_str();
        resource_vfs = skr_create_vfs(&vfs_desc);
    }
    {
        auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
        ioServiceDesc.name = "GUI-RAMService";
        ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_COND_VAR;
        ioServiceDesc.sleep_time = 1000 / 60;
        ioServiceDesc.lockless = true;
        ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
        ram_service = skr_io_ram_service_t::create(&ioServiceDesc);
    }
    {
        auto ioServiceDesc = make_zeroed<skr_vram_io_service_desc_t>();
        ioServiceDesc.name = "GUI-VRAMService";
        ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_COND_VAR;
        ioServiceDesc.sleep_time = 1000 / 60;
        ioServiceDesc.lockless = true;
        ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
        vram_service = skr_io_vram_service_t::create(&ioServiceDesc);
    }
    {
        auto ioServiceDesc = make_zeroed<skr_threaded_service_desc_t>();
        ioServiceDesc.name = "GUI-AuxService";
        ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_COND_VAR;
        ioServiceDesc.sleep_time = 1000 / 60;
        ioServiceDesc.lockless = true;
        ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
        aux_service = skr_threaded_service_t::create(&ioServiceDesc);
    }
    skr::gdi::SGDIRendererDescriptor gdir_desc = {};
    skr::gdi::SGDIRendererDescriptor_RenderGraph gdir_desc2 = {};
    gdir_desc2.target_format = (ECGPUFormat)App.swapchain->back_buffers[0]->format;
    gdir_desc2.device = App.device;
    gdir_desc2.transfer_queue = App.gfx_queue;
    gdir_desc2.vfs = resource_vfs;
    gdir_desc2.ram_service = ram_service;
    gdir_desc2.vram_service = vram_service;
    gdir_desc2.aux_service = aux_service;
    gdi_renderer = SkrNew<skr::gdi::SGDIRenderer_RenderGraph>();
    gdir_desc.usr_data = &gdir_desc2;
    gdi_renderer->initialize(&gdir_desc);
}

void draw_background_canvas()
{
    const bool bDrawRelativeXMesh = false;
    const bool bDrawRelativeYMesh = false;
    const float window_width = static_cast<float>(App.window_width);
    const float window_height = static_cast<float>(App.window_height);

    background_element->begin_frame(1.f);
    // draw background
    {            
        background_element->begin_path();
        background_element->rect(0, 0, window_width, window_height);
        background_element->fill_color(235u, 235u, 235u, 255u);
        background_element->fill();
    }
    const auto epsillon = 3.f;
    const auto absUnitX = 10.f;
    const auto absUnitY = absUnitX;
    const auto unitX = App.window_width / 100.f;
    const auto unitY = App.window_height / 100.f;

    // draw relative main-meshes
    if (bDrawRelativeXMesh)
    {
        background_element->begin_path();
        background_element->stroke_width(2.f);
        background_element->stroke_color(0u, 200u, 64u, 200u);
        for (uint32_t i = 0; i < 10; ++i)
        {
            const auto pos = eastl::max(i * unitY * 10.f, epsillon);
            background_element->move_to(0.f, pos);
            background_element->line_to(window_width, pos);
        }
        background_element->stroke();
    }
    if (bDrawRelativeYMesh)
    {
        background_element->begin_path();
        background_element->stroke_width(2.f);
        background_element->stroke_color(200u, 0u, 64u, 200u);
        for (uint32_t i = 0; i < 10; ++i)
        {
            const auto pos = eastl::max(i * unitX * 10.f, epsillon);
            background_element->move_to(pos, 0.f);
            background_element->line_to(pos, window_height);
        }
        background_element->stroke();
    }

    // draw absolute main-meshes
    background_element->begin_path();
    background_element->stroke_width(2.f);
    background_element->stroke_color(125u, 125u, 255u, 200u);
    for (uint32_t i = 0; i < App.window_height / absUnitY / 10; ++i)
    {
        const auto pos = eastl::max(i * absUnitY * 10.f, epsillon);
        background_element->move_to(0.f, pos);
        background_element->line_to(window_width, pos);
    }
    for (uint32_t i = 0; i < App.window_width / absUnitX / 10; ++i)
    {
        const auto pos = eastl::max(i * absUnitX * 10.f, epsillon);
        background_element->move_to(pos, 0.f);
        background_element->line_to(pos, window_height);
    }
    background_element->stroke();
    
    // draw absolute sub-meshes
    background_element->begin_path();
    background_element->stroke_width(1.f);
    background_element->stroke_color(88u, 88u, 222u, 180u);
    for (uint32_t i = 0; i < App.window_height / absUnitY; ++i)
    {
        const auto pos = eastl::max(i * absUnitY, epsillon);
        background_element->move_to(0.f, pos);
        background_element->line_to(window_width, pos);
    }
    for (uint32_t i = 0; i < App.window_width / absUnitX; ++i)
    {
        const auto pos = eastl::max(i * absUnitX, epsillon);
        background_element->move_to(pos, 0.f);
        background_element->line_to(pos, window_height);
    }
    background_element->stroke();
}

int main(int argc, char* argv[])
{
    App = make_zeroed<render_application_t>();
    App.backend = platform_default_backend;
    skr::string app_name = "GDI [backend:";
    app_name += gCGPUBackendNames[App.backend];
    app_name += "]";
    App.window_title = app_name.c_str();
    if (app_create_window(&App, 900, 900)) return -1;
    if (app_create_gfx_objects(&App)) return -1;
    // initialize
    namespace render_graph = skr::render_graph;
    auto graph = render_graph::RenderGraph::create(
    [=](render_graph::RenderGraphBuilder& builder) {
        builder.with_device(App.device)
        .with_gfx_queue(App.gfx_queue);
    });
    // create GDI objects
    skr::gdi::SGDITextureId test_texture = nullptr;
    gdi_device = skr::gdi::SGDIDevice::Create(skr::gdi::EGDIBackend::NANOVG);
    gdi_canvas_group = gdi_device->create_canvas_group();
    gdi_canvas = gdi_device->create_canvas();
    background_canvas = gdi_device->create_canvas();
    gdi_canvas_group->add_canvas(background_canvas);
    gdi_canvas_group->add_canvas(gdi_canvas);
    gdi_canvas->size = { (float)App.window_width, (float)App.window_height };
    background_canvas->size = { (float)App.window_width, (float)App.window_height };
    
    skr::gdi::SGDIElement* test_element = gdi_device->create_element();
    skr::gdi::SGDIElement* debug_element = gdi_device->create_element();
    skr::gdi::SGDIPaint* test_paint = gdi_device->create_paint();
    gdi_canvas->add_element(debug_element);
    gdi_canvas->add_element(test_element);

    background_element = gdi_device->create_element();
    background_canvas->add_element(background_element);

    // create GDI renderer
    initialize_renderer();

    // create GDI texture
    {
        skr::gdi::SGDITextureDescriptor tex_desc = {};
        skr::gdi::SGDITextureDescriptor_RenderGraph tex_desc2 = {};
        tex_desc.u8Uri = "OpenGUI/rubduck.png";
        tex_desc2.useImageCoder = true;
        tex_desc.usr_data = &tex_desc2;
        test_texture = gdi_renderer->create_texture(&tex_desc);
    }
    // loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(App.sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, App.sdl_window))
                {
                    quit = true;
                }
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                uint8_t window_event = event.window.event;
                if (window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    app_resize_window(&App, event.window.data1, event.window.data2);
                    gdi_canvas->size = { (float)App.window_width, (float)App.window_height };
                    background_canvas->size = { (float)App.window_width, (float)App.window_height };
                }
            }
        }
        // acquire frame
        cgpu_wait_fences(&App.present_fence, 1);
        CGPUAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = App.present_fence;
        App.backbuffer_index = cgpu_acquire_next_image(App.swapchain, &acquire_desc);
        // GDI
        draw_background_canvas();
        {
            debug_element->begin_frame(1.f);
            debug_element->begin_path();
            debug_element->rect(120, 120, 300, 300);
            debug_element->fill_color(128u, 0u, 0u, 128u);
            debug_element->fill();

            test_element->begin_frame(1.f);
            test_element->begin_path();
            test_element->rect(120, 120, 300, 300);
            skr_float4_t color = {1.f, 1.f, 1.f, 1.f};
            if (test_texture->get_state() != skr::gdi::EGDIResourceState::Okay)
            {
                color.w = 0.f; // set transparent if texture is not ready
            }
            test_paint->set_pattern(120, 120, 300, 300, 0, test_texture, color);
            test_element->fill_paint(test_paint);
            test_element->fill();
        }
        // render graph setup & compile & exec
        CGPUTextureId to_import = App.swapchain->back_buffers[App.backbuffer_index];
        auto back_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name("backbuffer")
                .import(to_import, CGPU_RESOURCE_STATE_PRESENT)
                .allow_render_target();
            });
        // render GDI canvas group
        skr::gdi::SGDIRenderParams gdir_params = {};
        skr::gdi::SGDIRenderParams_RenderGraph gdir_params2 = {};
        gdir_params2.render_graph = graph;
        gdir_params.usr_data = &gdir_params2;
        gdi_renderer->render(gdi_canvas_group, &gdir_params);
        // do present
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name("present")
                .swapchain(App.swapchain, App.backbuffer_index)
                .texture(back_buffer, true);
            });
        graph->compile();
        const auto frame_index = graph->execute();
        // present
        cgpu_wait_queue_idle(App.gfx_queue);
        CGPUQueuePresentDescriptor present_desc = {};
        present_desc.index = App.backbuffer_index;
        present_desc.swapchain = App.swapchain;
        cgpu_queue_present(App.gfx_queue, &present_desc);
        if (frame_index == 0)
            render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph_demo.gv");
    }
    render_graph::RenderGraph::destroy(graph);
    // clean up
    app_wait_gpu_idle(&App);
    // free GDI objects
    if (test_element) gdi_device->free_element(test_element);
    if (debug_element) gdi_device->free_element(debug_element);
    if (test_paint) gdi_device->free_paint(test_paint);
    if (test_texture) gdi_renderer->free_texture(test_texture);
    if (background_element) gdi_device->free_element(background_element);
    gdi_device->free_canvas(background_canvas);
    gdi_device->free_canvas(gdi_canvas);
    gdi_device->free_canvas_group(gdi_canvas_group);
    skr::gdi::SGDIDevice::Free(gdi_device);
    gdi_renderer->finalize();
    SkrDelete(gdi_renderer);
    // free gfx objects
    app_wait_gpu_idle(&App);
    app_finalize(&App);
    // free services
    if (aux_service) skr_threaded_service_t::destroy(aux_service);
    if (vram_service) skr_io_vram_service_t::destroy(vram_service);
    if (ram_service) skr_io_ram_service_t::destroy(ram_service);
    if (resource_vfs) skr_free_vfs(resource_vfs);
    return 0;
}