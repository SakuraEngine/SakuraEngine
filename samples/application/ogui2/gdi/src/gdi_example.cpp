#include "gdi_application.h"
#include "gui_render_graph.hpp"
#include "SkrRT/platform/system.h"

#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrGui/dev/gdi/gdi.hpp"

#include "SkrGuiRenderer/gdi_renderer.hpp"

struct gdi_example_application : public gdi_application_t {
    void draw_background_canvas()
    {
        const bool  bDrawRelativeXMesh = false;
        const bool  bDrawRelativeYMesh = false;
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
        backgroud_canvas = device->create_canvas();
        gdi_viewport->add_canvas(backgroud_canvas);
        gdi_viewport->add_canvas(gdi_canvas);
        gdi_canvas->set_size((float)gfx.window_width, (float)gfx.window_height);
        backgroud_canvas->set_size((float)gfx.window_width, (float)gfx.window_height);

        background_element = device->create_element();
        backgroud_canvas->add_element(background_element);

        test_element = device->create_element();
        debug_element = device->create_element();
        test_paint = device->create_paint();
        gdi_canvas->add_element(debug_element);
        gdi_canvas->add_element(test_element);
        {
            skr::gui::GDITextureDescriptor             tex_desc = {};
            skr::gui::GDITextureDescriptor_RenderGraph tex_desc2 = {};
            tex_desc.source = skr::gui::EGDITextureSource::File;
            tex_desc.from_file.u8Uri = u8"OpenGUI/rubduck.png";
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
            skr_float4_t color = { 1.f, 1.f, 1.f, 1.f };
            if (test_texture->get_state() != skr::gui::EGDIResourceState::Okay)
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
        skr::gui::ViewportRenderParams             render_params = {};
        skr::gui::ViewportRenderParams_RenderGraph gdir_params2 = {};
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
        device->free_canvas(backgroud_canvas);
        device->free_canvas(gdi_canvas);
        device->free_viewport(gdi_viewport);
        // free base app
        finalize_gdi_application(this);
    }

    gui_render_graph_t graph;

    skr::gui::IGDICanvas*   gdi_canvas = nullptr;
    skr::gui::IGDIViewport* gdi_viewport = nullptr;

    skr::gui::IGDICanvas*  backgroud_canvas = nullptr;
    skr::gui::IGDIElement* background_element = nullptr;

    skr::gui::IGDITexture* test_texture = nullptr;
    skr::gui::IGDIPaint*   test_paint = nullptr;
    skr::gui::IGDIElement* test_element = nullptr;
    skr::gui::IGDIElement* debug_element = nullptr;
};

#include "tracy/Tracy.hpp"

int main(int argc, char* argv[])
{
    SkrDStorageConfig config = {};
    skr_create_dstorage_instance(&config);
    auto App = make_zeroed<gdi_example_application>();
    App.initialize();
    bool quit = false;
    auto handler = skr_system_get_default_handler();
    handler->add_window_close_handler(
    +[](SWindowHandle window, void* pQuit) {
        bool& quit = *(bool*)pQuit;
        quit = true;
    },
    &quit);
    handler->add_window_resize_handler(
    +[](SWindowHandle window, int32_t w, int32_t h, void* usr_data) {
        gdi_example_application* pApp = (gdi_example_application*)usr_data;
        app_resize_window(&pApp->gfx, w, h);
        pApp->gdi_canvas->set_size((float)pApp->gfx.window_width, (float)pApp->gfx.window_height);
        pApp->backgroud_canvas->set_size((float)pApp->gfx.window_width, (float)pApp->gfx.window_height);
    },
    &App);

    while (!quit)
    {
        FrameMark;
        float delta = 1.f / 60.f;
        {
            ZoneScopedN("SystemEvents");
            handler->pump_messages(delta);
            handler->process_messages(delta);
        }
        {
            ZoneScopedN("GDI Draw & Render");
            App.render();
        }
    }
    App.finalize();
    return 0;
}