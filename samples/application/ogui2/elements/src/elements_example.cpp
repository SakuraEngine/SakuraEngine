#include "elem_application.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrGui/interface/gdi_renderer.hpp"
#include "SkrGui/window_context.hpp"

#include "SkrGui/render_elements/render_window.hpp"
#include "SkrGui/render_elements/render_canvas.hpp"
#include "SkrGui/render_elements/render_grid_paper.hpp"

#include "SkrGuiRenderer/gdi_renderer.hpp"

struct elements_example_application : public elements_application_t
{
    bool initialize()
    {
        // initialize base app
        if (!initialize_elem_application(this)) return false;

        // add elements
        canvas = SkrNew<skr::gui::RenderCanvas>(gdi.device);
        grid_paper = SkrNew<skr::gui::RenderGridPaper>(gdi.device);
        root_window->add_child(canvas);
        canvas->add_child(grid_paper);

        // initialize render graph
        namespace render_graph = skr::render_graph;
        graph = render_graph::RenderGraph::create(
        [=](render_graph::RenderGraphBuilder& builder) {
            builder.with_device(gdi.gfx.device)
            .with_gfx_queue(gdi.gfx.gfx_queue);
        });
        return true;
    }

    ECGPUSampleCount sample_count = CGPU_SAMPLE_COUNT_1;
    void declare_render_resources()
    {
        namespace render_graph = skr::render_graph;
        // acquire frame
        cgpu_wait_fences(&gdi.gfx.present_fence, 1);
        CGPUAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = gdi.gfx.present_fence;
        gdi.gfx.backbuffer_index = cgpu_acquire_next_image(gdi.gfx.swapchain, &acquire_desc);

        // declare resources
        CGPUTextureId imported_backbuffer = gdi.gfx.swapchain->back_buffers[gdi.gfx.backbuffer_index];
        if (sample_count != CGPU_SAMPLE_COUNT_1)
        {
            back_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("presentbuffer")
                    .import(imported_backbuffer, CGPU_RESOURCE_STATE_PRESENT)
                    .allow_render_target();
                });
            const auto back_desc = graph->resolve_descriptor(back_buffer);
            auto msaaTarget = graph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                builder.set_name("backbuffer")
                    .extent(back_desc->width, back_desc->height)
                    .format(back_desc->format)
                    .owns_memory()
                    .sample_count(sample_count)
                    .allow_render_target();
            });(void)msaaTarget;
        }
        else
        {
            back_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name("backbuffer")
                    .import(imported_backbuffer, CGPU_RESOURCE_STATE_PRESENT)
                    .allow_render_target();
                });
        }
        depth_buffer = graph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                builder.set_name("depth")
                    .extent(gdi.gfx.swapchain->back_buffers[0]->width, gdi.gfx.swapchain->back_buffers[0]->height)
                    .format(CGPU_FORMAT_D32_SFLOAT)
                    .owns_memory()
                    .sample_count(sample_count)
                    .allow_depth_stencil();
            });
    }

    void submit_render_graph()
    {
        namespace render_graph = skr::render_graph;
        // do present
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name("present")
                .swapchain(gdi.gfx.swapchain, gdi.gfx.backbuffer_index)
                .texture(back_buffer, true);
            });

        // render graph setup & compile & exec
        graph->compile();
        frame_index = graph->execute();

        // present
        cgpu_wait_queue_idle(gdi.gfx.gfx_queue);
        CGPUQueuePresentDescriptor present_desc = {};
        present_desc.index = gdi.gfx.backbuffer_index;
        present_desc.swapchain = gdi.gfx.swapchain;
        cgpu_queue_present(gdi.gfx.gfx_queue, &present_desc);
    }

    void draw()
    {
        skr::gui::WindowContext::DrawParams draw_params = {};
        window_context->draw(&draw_params);
    }

    void render()
    {
        // declare render resources
        declare_render_resources();

        // render
        skr::gui::WindowContext::RenderParams render_params = {};
        skr::gdi::ViewportRenderParams vp_render_params = {};
        skr::gdi::ViewportRenderParams_RenderGraph vp_render_params2 = {};
        vp_render_params2.render_graph = graph;
        vp_render_params.usr_data = &vp_render_params2;
        render_params.gdi_params = &vp_render_params;
        window_context->render(gdi.renderer, &render_params);

        // submit graph
        submit_render_graph();
    }

    void finalize()
    {
        namespace render_graph = skr::render_graph;
        render_graph::RenderGraph::destroy(graph);
        // clean up
        app_wait_gpu_idle(&gdi.gfx);
        // free GDI objects
        SkrDelete(grid_paper);
        SkrDelete(canvas);
        // free base app
        finalize_elem_application(this);
    }

    skr::gui::RenderCanvas* canvas = nullptr;
    skr::gui::RenderGridPaper* grid_paper = nullptr;

    skr::render_graph::RenderGraph* graph;
    skr::render_graph::TextureHandle back_buffer;
    skr::render_graph::TextureHandle depth_buffer;
    uint64_t frame_index = 0;
};

int main(int argc, char* argv[])
{
    auto App = make_zeroed<elements_example_application>();
    App.initialize();
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            auto sdl_window = (SDL_Window*)App.gdi.gfx.window_handle;
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    quit = true;
                }
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                uint8_t window_event = event.window.event;
                if (window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    app_resize_window(&App.gdi.gfx, event.window.data1, event.window.data2);
                }
            }
        }
        App.draw();
        App.render();
    }
    App.finalize();
    return 0;
}