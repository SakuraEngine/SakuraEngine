#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/primitive_pass.h"
#include "live2d_helpers.hpp"

#include "tracy/Tracy.hpp"

const skr_render_pass_name_t live2d_pass_name = "Live2DPass";

struct RenderPassLive2D : public IPrimitiveRenderPass {
    void on_update(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph) override
    {
        auto backbuffer = renderGraph->get_texture("backbuffer");
        const auto back_desc = renderGraph->resolve_descriptor(backbuffer);
        auto msaaTarget = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value("l2d_msaa", sample_level);

            builder.set_name("live2d_msaa")
                .extent(back_desc->width, back_desc->height)
                .format(back_desc->format)
                .owns_memory()
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_render_target();
        });(void)msaaTarget;
        auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value("l2d_msaa", sample_level);

            builder.set_name("depth")
                .extent(back_desc->width, back_desc->height)
                .format(live2d_depth_format)
                .owns_memory()
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_depth_stencil();
        });(void)depth;
    }

    void post_update(SRendererId renderer, skr::render_graph::RenderGraph* renderGraph) override
    {

    }

    void execute(skr::render_graph::RenderGraph* renderGraph, skr_primitive_draw_list_view_t drawcalls) override
    {
        auto backbuffer = renderGraph->get_texture("backbuffer");
        const auto back_desc = renderGraph->resolve_descriptor(backbuffer);
        if (drawcalls.count)
        {
            renderGraph->add_render_pass(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                double sample_level = 1.0;
                bool useMSAA = g.get_blackboard().value("l2d_msaa", sample_level); useMSAA &= (sample_level > 1.0);
                const auto depth_buffer = renderGraph->get_texture("depth");
                const auto mask_buffer = renderGraph->get_texture("live2d_mask");
                const auto live2d_msaa = renderGraph->get_texture("live2d_msaa");
                const auto backbuffer = renderGraph->get_texture("backbuffer");
                builder.set_name("live2d_forward_pass")
                    .set_root_signature(drawcalls.drawcalls->pipeline->root_signature)
                    .read("mask_texture", mask_buffer)
                    .set_depth_stencil(depth_buffer.clear_depth(1.f))
                    .write(0, useMSAA ? live2d_msaa : backbuffer, CGPU_LOAD_ACTION_CLEAR);
                if (useMSAA)
                {
                    builder.resolve_msaa(0, backbuffer);
                }
            },
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& pass_context) {
                for (uint32_t i = 0; i < drawcalls.count; i++)
                {
                    ZoneScopedN("UpdateTextures");
                    auto&& dc = drawcalls.drawcalls[i];
                    if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;

                    CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                    pass_context.merge_tables(tables, 2);
                }
                cgpu_render_encoder_set_viewport(pass_context.encoder,
                    0.0f, 0.0f,
                    (float)back_desc->width, (float)back_desc->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(pass_context.encoder, 0, 0, back_desc->width, back_desc->height);
                CGPURenderPipelineId old_pipeline = nullptr;
                for (uint32_t i = 0; i < drawcalls.count; i++)
                {
                    ZoneScopedN("DrawCall");

                    auto&& dc = drawcalls.drawcalls[i];
                    if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;

                    if (old_pipeline != dc.pipeline)
                    {
                        cgpu_render_encoder_bind_pipeline(pass_context.encoder, dc.pipeline);
                        old_pipeline = dc.pipeline;
                    }
                    {
                        ZoneScopedN("BindTextures");
                        CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                        pass_context.merge_and_bind_tables(tables, 2);
                    }
                    {
                        ZoneScopedN("BindGeometry");
                        cgpu_render_encoder_bind_index_buffer(pass_context.encoder, dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
                        CGPUBufferId vertex_buffers[2] = {
                            dc.vertex_buffers[0].buffer, dc.vertex_buffers[1].buffer
                        };
                        const uint32_t strides[2] = {
                            dc.vertex_buffers[0].stride, dc.vertex_buffers[1].stride
                        };
                        const uint32_t offsets[2] = {
                            dc.vertex_buffers[0].offset, dc.vertex_buffers[1].offset
                        };
                        cgpu_render_encoder_bind_vertex_buffers(pass_context.encoder, 2, vertex_buffers, strides, offsets);
                    }
                    {
                        ZoneScopedN("PushConstants");
                        cgpu_render_encoder_push_constants(pass_context.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                    }
                    {
                        ZoneScopedN("DrawIndexed");
                        cgpu_render_encoder_draw_indexed_instanced(pass_context.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
                    }
                }
            });
        }
    }

    skr_render_pass_name_t identity() const override
    {
        return live2d_pass_name;
    }
};