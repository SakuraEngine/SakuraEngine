#include "elem_application.h"
#include "gui_render_graph.hpp"

#include "SkrGui/interface/gdi_renderer.hpp"
#include "SkrGui/framework/window_context.hpp"

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
        return graph.initialize(gdi.gfx);
    }

    void draw()
    {
        skr::gui::WindowContext::DrawParams draw_params = {};
        window_context->draw(&draw_params);
    }

    void render()
    {
        // declare render resources
        graph.declare_render_resources(gdi.gfx);

        // render
        skr::gui::WindowContext::RenderParams render_params = {};
        skr::gdi::ViewportRenderParams vp_render_params = {};
        skr::gdi::ViewportRenderParams_RenderGraph vp_render_params2 = {};
        vp_render_params2.render_graph = graph.graph;
        vp_render_params.usr_data = &vp_render_params2;
        render_params.gdi_params = &vp_render_params;
        window_context->render(gdi.renderer, &render_params);

        // submit graph
        graph.submit_render_graph(gdi.gfx);
    }

    void finalize()
    {
        // clean up
        app_wait_gpu_idle(&gdi.gfx);
        graph.finalize();

        // free render elements
        SkrDelete(grid_paper);
        SkrDelete(canvas);
        
        // free base app
        finalize_elem_application(this);
    }

    skr::gui::RenderCanvas* canvas = nullptr;
    skr::gui::RenderGridPaper* grid_paper = nullptr;
    gui_render_graph_t graph;
};

#include "tracy/Tracy.hpp"

int main(int argc, char* argv[])
{
    auto App = make_zeroed<elements_example_application>();
    App.initialize();
    bool quit = false;
    while (!quit)
    {
        FrameMark;
        {
            ZoneScopedN("SystemEvents");
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
        }
        {
            ZoneScopedN("DrawGUI");
            App.draw();
        }
        {
            ZoneScopedN("RenderGUI");
            App.render();
        }
    }
    App.finalize();
    return 0;
}