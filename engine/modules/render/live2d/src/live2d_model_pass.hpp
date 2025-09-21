#pragma once
#include "SkrProfile/profile.h"
#include "SkrGraphics/cgpux.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "live2d_helpers.hpp"

struct Live2DRenderPass 
{
    static void create_frame_resources(skr::render_graph::RenderGraph* render_graph)
    {
        auto backbuffer = render_graph->get_texture(u8"backbuffer");
        const auto back_desc = render_graph->resolve_descriptor(backbuffer);
        auto msaaTarget = render_graph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value(u8"l2d_msaa", sample_level);

            builder.set_name(u8"live2d_msaa")
                .extent(back_desc->width, back_desc->height)
                .format(back_desc->format)
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_render_target();
            if (back_desc->height > 2048) builder.heap_dedicated();
        });(void)msaaTarget;
        
        auto depth = render_graph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value(u8"l2d_msaa", sample_level);

            builder.set_name(u8"depth")
                .extent(back_desc->width, back_desc->height)
                .format(live2d_depth_format)
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_depth_stencil();
            if (back_desc->height > 2048) builder.heap_dedicated();
        });(void)depth;
    }

    static void execute(skr::render_graph::RenderGraph* render_graph, skr::Span<skr_primitive_draw_t> drawcalls)
    {
        auto backbuffer = render_graph->get_texture(u8"backbuffer");
        const auto back_desc = render_graph->resolve_descriptor(backbuffer);
        if (!drawcalls.size()) return;

        if (drawcalls.size() == 0)
            return; // no models need to draw
        
        CGPURootSignatureId root_signature = nullptr;
        root_signature = drawcalls[0].pipeline->root_signature;
        render_graph->add_render_pass(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
            double sample_level = 1.0;
            bool useMSAA = g.get_blackboard().value(u8"l2d_msaa", sample_level); useMSAA &= (sample_level > 1.0);
            const auto depth_buffer = render_graph->get_texture(u8"depth");
            const auto mask_buffer = render_graph->get_texture(u8"live2d_mask");
            const auto live2d_msaa = render_graph->get_texture(u8"live2d_msaa");
            const auto backbuffer = render_graph->get_texture(u8"backbuffer");
            builder.set_name(u8"live2d_forward_pass")
                .set_root_signature(root_signature)
                .read(u8"mask_texture", mask_buffer)
                .set_depth_stencil(depth_buffer.clear_depth(1.f))
                .write(0, useMSAA ? live2d_msaa : backbuffer, CGPU_LOAD_ACTION_CLEAR);
            if (useMSAA)
            {
                builder.resolve_msaa(0, backbuffer);
            }
        },
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& pass_context) {
            SkrZoneScopedN("DrawLive2D");
            skr::InlineMap<CGPUXBindTableId, CGPUXMergedBindTableId, 8> merged_tables;
            cgpu_render_encoder_set_viewport(pass_context.encoder,
                0.0f, 0.0f,
                (float)back_desc->width, (float)back_desc->height,
                0.f, 1.f);
            cgpu_render_encoder_set_scissor(pass_context.encoder, 
                0, 0, (uint32_t)back_desc->width, (uint32_t)back_desc->height);
            CGPURenderPipelineId old_pipeline = nullptr;
            for (uint32_t i = 0; i < drawcalls.size(); i++)
            {
                auto&& dc = drawcalls[i];
                if (dc.deprecated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) 
                    continue;

                if (auto bd = merged_tables.find(dc.bind_table))
                {
                    pass_context.bind(bd.value());
                }
                else
                {
                    CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                    auto merged = pass_context.merge_tables(tables, 2);
                    merged_tables.add(dc.bind_table, merged);
                    pass_context.bind(merged);
                }


                if (old_pipeline != dc.pipeline)
                {
                    cgpu_render_encoder_bind_pipeline(pass_context.encoder, dc.pipeline);
                    old_pipeline = dc.pipeline;
                }
                {
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
                    cgpu_render_encoder_push_constants(pass_context.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                }
                {
                    cgpu_render_encoder_draw_indexed_instanced(pass_context.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
                }
            }
        });
    }
};