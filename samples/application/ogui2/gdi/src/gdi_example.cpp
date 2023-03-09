#include "../../../../common/render_application.h"
#include "gdi_application.h"
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
        namespace render_graph = skr::render_graph;
        graph = render_graph::RenderGraph::create(
        [=](render_graph::RenderGraphBuilder& builder) {
            builder.with_device(gfx.device)
            .with_gfx_queue(gfx.gfx_queue);
        });
       
        // create GDI objects
        device = skr::gdi::SGDIDevice::Create(skr::gdi::EGDIBackend::NANOVG);
        gdi_canvas_group = device->create_canvas_group();
        gdi_canvas = device->create_canvas();
        background_canvas = device->create_canvas();
        gdi_canvas_group->add_canvas(background_canvas);
        gdi_canvas_group->add_canvas(gdi_canvas);
        gdi_canvas->size = { (float)gfx.window_width, (float)gfx.window_height };
        background_canvas->size = { (float)gfx.window_width, (float)gfx.window_height };
        
        background_element = device->create_element();
        background_canvas->add_element(background_element);

        test_element = device->create_element();
        debug_element = device->create_element();
        test_paint = device->create_paint();
        gdi_canvas->add_element(debug_element);
        gdi_canvas->add_element(test_element);
        {
            skr::gdi::SGDITextureDescriptor tex_desc = {};
            skr::gdi::SGDITextureDescriptor_RenderGraph tex_desc2 = {};
            tex_desc.source = skr::gdi::EGDITextureSource::File;
            tex_desc.from_file.u8Uri = "OpenGUI/rubduck.png";
            tex_desc2.useImageCoder = true;
            tex_desc.usr_data = &tex_desc2;
            test_texture = renderer->create_texture(&tex_desc);
        }

        return true;
    }

    ECGPUSampleCount sample_count = CGPU_SAMPLE_COUNT_1;
    void declare_render_resources()
    {
        namespace render_graph = skr::render_graph;
        // acquire frame
        cgpu_wait_fences(&gfx.present_fence, 1);
        CGPUAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = gfx.present_fence;
        gfx.backbuffer_index = cgpu_acquire_next_image(gfx.swapchain, &acquire_desc);

        // declare resources
        CGPUTextureId imported_backbuffer = gfx.swapchain->back_buffers[gfx.backbuffer_index];
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
                    .extent(gfx.swapchain->back_buffers[0]->width, gfx.swapchain->back_buffers[0]->height)
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
                .swapchain(gfx.swapchain, gfx.backbuffer_index)
                .texture(back_buffer, true);
            });

        // render graph setup & compile & exec
        graph->compile();
        if (frame_index == 0)
        {
            render_graph::RenderGraphViz::write_graphviz(*graph, "gdi_renderer.gv");
        }
        frame_index = graph->execute();

        // present
        cgpu_wait_queue_idle(gfx.gfx_queue);
        CGPUQueuePresentDescriptor present_desc = {};
        present_desc.index = gfx.backbuffer_index;
        present_desc.swapchain = gfx.swapchain;
        cgpu_queue_present(gfx.gfx_queue, &present_desc);
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
        declare_render_resources();

        // render GDI canvas group
        skr::gdi::SGDIRenderParams render_params = {};
        skr::gdi::SGDIRenderParams_RenderGraph gdir_params2 = {};
        gdir_params2.render_graph = graph;
        render_params.usr_data = &gdir_params2;
        renderer->render(gdi_canvas_group, &render_params);
        submit_render_graph();
    }

    void finalize()
    {
        namespace render_graph = skr::render_graph;
        render_graph::RenderGraph::destroy(graph);
        // clean up
        app_wait_gpu_idle(&gfx);
        // free GDI objects
        if (test_texture) renderer->free_texture(test_texture);
        if (test_element) device->free_element(test_element);
        if (debug_element) device->free_element(debug_element);
        if (test_paint) device->free_paint(test_paint);
        if (background_element) device->free_element(background_element);
        device->free_canvas(background_canvas);
        device->free_canvas(gdi_canvas);
        device->free_canvas_group(gdi_canvas_group);
        // free base app
        finalize_gdi_application(this);
    }

    skr::render_graph::RenderGraph* graph;
    skr::render_graph::TextureHandle back_buffer;
    skr::render_graph::TextureHandle depth_buffer;
    uint64_t frame_index = 0;

    skr::gdi::SGDICanvas* gdi_canvas = nullptr;
    skr::gdi::SGDICanvasGroup* gdi_canvas_group = nullptr;

    skr::gdi::SGDICanvas* background_canvas = nullptr;
    skr::gdi::SGDIElement* background_element = nullptr;

    skr::gdi::SGDITextureId test_texture = nullptr;
    skr::gdi::SGDIPaint* test_paint = nullptr;
    skr::gdi::SGDIElement* test_element = nullptr;
    skr::gdi::SGDIElement* debug_element = nullptr;
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
            if (SDL_GetWindowID(App.gfx.sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, App.gfx.sdl_window))
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
                    App.background_canvas->size = { (float)App.gfx.window_width, (float)App.gfx.window_height };
                }
            }
        }
        App.render();
    }
    App.finalize();
    return 0;
}