#include "../../../../common/render_application.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrGui/gdi.h"
#include "SkrGuiRenderer/renderer.hpp"

thread_local render_application_t App;

thread_local skr::gdi::SGDIDevice* gdi_device = nullptr;
thread_local skr::gdi::SGDICanvas* gdi_canvas = nullptr;
thread_local skr::gdi::SGDICanvasGroup* gdi_canvas_group = nullptr;
thread_local skr::gdi::SGDIRenderer_RenderGraph* gdi_renderer = nullptr;

int main(int argc, char* argv[])
{
    App = make_zeroed<render_application_t>();
    App.backend = platform_default_backend;
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
    skr::gdi::SGDIElement* test_element = nullptr;
    gdi_device = skr::gdi::SGDIDevice::Create(skr::gdi::EGDIBackend::NANOVG);
    gdi_canvas_group = gdi_device->create_canvas_group();
    gdi_canvas = gdi_device->create_canvas();
    test_element = gdi_device->create_element();
    gdi_canvas_group->add_canvas(gdi_canvas);
    gdi_canvas->add_element(test_element, {0.f, 0.f, 0.f, 0.f});
    // create GDI renderer
    skr::gdi::SGDIRendererDescriptor gdir_desc = {};
    skr::gdi::SGDIRendererDescriptor_RenderGraph gdir_desc2 = {};
    gdir_desc2.device = App.device;
    gdi_renderer = SkrNew<skr::gdi::SGDIRenderer_RenderGraph>();
    gdir_desc.usr_data = &gdir_desc2;
    gdi_renderer->initialize(&gdir_desc);
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
                }
            }
        }
        // acquire frame
        cgpu_wait_fences(&App.present_fence, 1);
        CGPUAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = App.present_fence;
        App.backbuffer_index = cgpu_acquire_next_image(App.swapchain, &acquire_desc);
        // GDI
        {
            test_element->begin_frame(1.f);
            test_element->begin_path();
            test_element->rect(100, 100, 300, 300);
            test_element->fill_color(155u, 30u, 120u, 255u);
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
    gdi_device->free_element(test_element);
    gdi_device->free_canvas(gdi_canvas);
    gdi_device->free_canvas_group(gdi_canvas_group);
    skr::gdi::SGDIDevice::Free(gdi_device);
    gdi_renderer->finalize();
    SkrDelete(gdi_renderer);
    // free gfx objects
    app_wait_gpu_idle(&App);
    app_finalize(&App);
    return 0;
}