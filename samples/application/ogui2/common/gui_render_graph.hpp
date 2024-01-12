#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "common/render_application.h"
#include "SkrBase/misc/make_zeroed.hpp"

#include "SkrProfile/profile.h"

struct gui_render_graph_t 
{
    bool initialize(render_application_t& render_app)
    {
        // initialize render graph
        namespace render_graph = skr::render_graph;
        graph = render_graph::RenderGraph::create(
        [=](render_graph::RenderGraphBuilder& builder) {
            builder.with_device(render_app.device)
            .with_gfx_queue(render_app.gfx_queue);
        });
        return true;
    }

    void declare_render_resources(render_application_t& render_app)
    {
        namespace render_graph = skr::render_graph;
        // acquire frame
        {
            SkrZoneScopedN("WaitPresent");
            cgpu_wait_fences(&render_app.present_fence, 1);
        }
        {
            SkrZoneScopedN("AcquireFrame");
            auto acquire_desc = make_zeroed<CGPUAcquireNextDescriptor>();
            acquire_desc.fence = render_app.present_fence;
            render_app.backbuffer_index = cgpu_acquire_next_image(render_app.swapchain, &acquire_desc);
        }
        // declare resources
        CGPUTextureId imported_backbuffer = render_app.swapchain->back_buffers[render_app.backbuffer_index];
        if (sample_count != CGPU_SAMPLE_COUNT_1)
        {
            back_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(SKR_UTF8("presentbuffer"))
                    .import(imported_backbuffer, CGPU_RESOURCE_STATE_PRESENT)
                    .allow_render_target();
                });
            const auto back_desc = graph->resolve_descriptor(back_buffer);
            auto msaaTarget = graph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                builder.set_name(SKR_UTF8("backbuffer"))
                    .extent(back_desc->width, back_desc->height)
                    .format(back_desc->format)
                    .sample_count(sample_count)
                    .allow_render_target();
            if (back_desc->width > 2048) builder.allocate_dedicated();
            });(void)msaaTarget;
        }
        else
        {
            back_buffer = graph->create_texture(
                [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                    builder.set_name(SKR_UTF8("backbuffer"))
                    .import(imported_backbuffer, CGPU_RESOURCE_STATE_PRESENT)
                    .allow_render_target();
                });
        }
        depth_buffer = graph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                const auto texInfo = render_app.swapchain->back_buffers[0]->info;
                builder.set_name(SKR_UTF8("depth"))
                    .extent(texInfo->width, texInfo->height)
                    .format(CGPU_FORMAT_D32_SFLOAT)
                    .sample_count(sample_count)
                    .allow_depth_stencil();
                if (texInfo->width > 2048) builder.allocate_dedicated();
            });
    }

    void submit_render_graph(render_application_t& render_app)
    {
        namespace render_graph = skr::render_graph;
        // do present
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name(SKR_UTF8("present"))
                .swapchain(render_app.swapchain, render_app.backbuffer_index)
                .texture(back_buffer, true);
            });

        // render graph setup & compile & exec
        graph->compile();
        frame_index = graph->execute();

        // present
        CGPUQueuePresentDescriptor present_desc = {};
        present_desc.index = render_app.backbuffer_index;
        present_desc.swapchain = render_app.swapchain;
        cgpu_queue_present(render_app.gfx_queue, &present_desc);
    }

    void finalize()
    {
        namespace render_graph = skr::render_graph;
        render_graph::RenderGraph::destroy(graph);
    }

    ECGPUSampleCount sample_count = CGPU_SAMPLE_COUNT_1;
    skr::render_graph::RenderGraph* graph;
    skr::render_graph::TextureHandle back_buffer;
    skr::render_graph::TextureHandle depth_buffer;
    uint64_t frame_index = 0;
};