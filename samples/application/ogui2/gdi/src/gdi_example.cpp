#include "gdi_application.h"
#include "gui_render_graph.hpp"

#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrGui/interface/gdi_renderer.hpp"

#include "SkrGuiRenderer/gdi_renderer.hpp"

struct gdi_example_application : public gdi_application_t
{
    void draw_background_canvas()
    {
        const bool bDrawRelativeXMesh = false;
        const bool bDrawRelativeYMesh = false;
        const float window_width = static_cast<float>(gfx.window_width);
        const float window_height = static_cast<float>(gfx.window_height);

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
        const auto unitX = gfx.window_width / 100.f;
        const auto unitY = gfx.window_height / 100.f;

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
        for (uint32_t i = 0; i < gfx.window_height / absUnitY / 10; ++i)
        {
            const auto pos = eastl::max(i * absUnitY * 10.f, epsillon);
            background_element->move_to(0.f, pos);
            background_element->line_to(window_width, pos);
        }
        for (uint32_t i = 0; i < gfx.window_width / absUnitX / 10; ++i)
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
        for (uint32_t i = 0; i < gfx.window_height / absUnitY; ++i)
        {
            const auto pos = eastl::max(i * absUnitY, epsillon);
            background_element->move_to(0.f, pos);
            background_element->line_to(window_width, pos);
        }
        for (uint32_t i = 0; i < gfx.window_width / absUnitX; ++i)
        {
            const auto pos = eastl::max(i * absUnitX, epsillon);
            background_element->move_to(pos, 0.f);
            background_element->line_to(pos, window_height);
        }
        background_element->stroke();
    }

    bool initialize()
    {
        // initialize base app
        if (!initialize_gdi_application(this)) return false;

        // initialize render graph
        graph.initialize(gfx);
        
        // create GDI objects
        gdi_viewport = device->create_viewport();
        gdi_canvas = device->create_canvas();
        background_render_group = device->create_canvas();
        gdi_viewport->add_canvas(background_render_group);
        gdi_viewport->add_canvas(gdi_canvas);
        gdi_canvas->size = { (float)gfx.window_width, (float)gfx.window_height };
        background_render_group->size = { (float)gfx.window_width, (float)gfx.window_height };
        
        background_element = device->create_element();
        background_render_group->add_element(background_element);

        test_element = device->create_element();
        debug_element = device->create_element();
        test_paint = device->create_paint();
        gdi_canvas->add_element(debug_element);
        gdi_canvas->add_element(test_element);
        {
            skr::gdi::GDITextureDescriptor tex_desc = {};
            skr::gdi::GDITextureDescriptor_RenderGraph tex_desc2 = {};
            tex_desc.source = skr::gdi::EGDITextureSource::File;
            tex_desc.from_file.u8Uri = "OpenGUI/rubduck.png";
            tex_desc2.useImageCoder = true;
            tex_desc.usr_data = &tex_desc2;
            test_texture = renderer->create_texture(&tex_desc);
        }

        return true;
    }

    void render()
    {
        // GDI
        draw_background_canvas();
        {
            debug_element->begin_frame(1.f);
            debug_element->begin_path();
            debug_element->rect(120, 120, 300, 300);
            debug_element->fill_color(128u, 0u, 0u, 128u);
            debug_element->set_z(5);
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
            test_element->set_z(10);
            test_element->fill();
        }

        // declare render resources
        graph.declare_render_resources(gfx);

        // render GDI canvas group
        skr::gdi::ViewportRenderParams render_params = {};
        skr::gdi::ViewportRenderParams_RenderGraph gdir_params2 = {};
        gdir_params2.render_graph = graph.graph;
        render_params.usr_data = &gdir_params2;
        renderer->render(gdi_viewport, &render_params);
        graph.submit_render_graph(gfx);
    }

    void finalize()
    {
        // clean up
        app_wait_gpu_idle(&gfx);
        graph.finalize();

        // free GDI objects
        if (test_texture) renderer->free_texture(test_texture);
        if (test_element) device->free_element(test_element);
        if (debug_element) device->free_element(debug_element);
        if (test_paint) device->free_paint(test_paint);
        if (background_element) device->free_element(background_element);
        device->free_canvas(background_render_group);
        device->free_canvas(gdi_canvas);
        device->free_viewport(gdi_viewport);
        // free base app
        finalize_gdi_application(this);
    }

    gui_render_graph_t graph;

    skr::gdi::GDICanvas* gdi_canvas = nullptr;
    skr::gdi::GDIViewport* gdi_viewport = nullptr;

    skr::gdi::GDICanvas* background_render_group = nullptr;
    skr::gdi::GDIElement* background_element = nullptr;

    skr::gdi::GDITextureId test_texture = nullptr;
    skr::gdi::GDIPaint* test_paint = nullptr;
    skr::gdi::GDIElement* test_element = nullptr;
    skr::gdi::GDIElement* debug_element = nullptr;
};

int main(int argc, char* argv[])
{
    auto App = make_zeroed<gdi_example_application>();
    App.initialize();
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            auto sdl_window = (SDL_Window*)App.gfx.window_handle;
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
                    app_resize_window(&App.gfx, event.window.data1, event.window.data2);
                    App.gdi_canvas->size = { (float)App.gfx.window_width, (float)App.gfx.window_height };
                    App.background_render_group->size = { (float)App.gfx.window_width, (float)App.gfx.window_height };
                }
            }
        }
        App.render();
    }
    App.finalize();
    return 0;
}