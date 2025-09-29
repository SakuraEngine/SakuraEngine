#pragma once
#include "SkrProfile/profile.h"
#include "SkrGraphics/cgpux.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "live2d_helpers.hpp"

struct Live2DMaskPass 
{
    static void create_frame_resources(skr::RG::RenderGraph* render_graph)
    {
        auto live2d_mask_msaa = render_graph->create_texture(
        [=](skr::RG::RenderGraph& g, skr::RG::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value(u8"l2d_msaa", sample_level);

            builder.set_name(u8"live2d_mask_msaa")
                .extent(Csm::kMaskResolution, Csm::kMaskResolution)
                .format(live2d_mask_format)
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_render_target();
            if constexpr (Csm::kMaskResolution > 2048) builder.heap_dedicated();
        });(void)live2d_mask_msaa;

        auto mask = render_graph->create_texture(
        [=](skr::RG::RenderGraph& g, skr::RG::TextureBuilder& builder) {
            builder.set_name(u8"live2d_mask")
                .extent(Csm::kMaskResolution, Csm::kMaskResolution)
                .format(live2d_mask_format)
                .allow_render_target();
            if constexpr (Csm::kMaskResolution > 2048) builder.heap_dedicated();
        });(void)mask;
        
        auto depth = render_graph->create_texture(
        [=](skr::RG::RenderGraph& g, skr::RG::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value(u8"l2d_msaa", sample_level);

            builder.set_name(u8"mask_depth")
                .extent(Csm::kMaskResolution, Csm::kMaskResolution)
                .format(live2d_depth_format)
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_depth_stencil();
            if constexpr (Csm::kMaskResolution > 2048) builder.heap_dedicated();
        });(void)depth;
    }

    static void execute(skr::RG::RenderGraph* render_graph, skr::Span<skr_primitive_draw_t> drawcalls)
    {
        if (!drawcalls.size()) return;

        CGPURenderPipelineId pipeline = drawcalls[0].pipeline;

        render_graph->add_render_pass(
        [=](skr::RG::RenderGraph& g, skr::RG::RenderPassBuilder& builder) {
            double sample_level = 1.0;
            bool useMSAA = g.get_blackboard().value(u8"l2d_msaa", sample_level); useMSAA &= (sample_level > 1.0);
            
            const auto mask = render_graph->get_texture(u8"live2d_mask");
            const auto mask_msaa = render_graph->get_texture(u8"live2d_mask_msaa");
            const auto depth_buffer = render_graph->get_texture(u8"mask_depth");
            builder.set_name(u8"live2d_mask_pass")
                // we know that the drawcalls always have a same pipeline
                .set_pipeline(pipeline)
                .write(0, useMSAA ? mask_msaa : mask, CGPU_LOAD_ACTION_CLEAR, fastclear_1111)
                .set_depth_stencil(depth_buffer.clear_depth(1.f));
            if (useMSAA)
            {
                builder.resolve_msaa(0, mask);
            }
        },
        [=](skr::RG::RenderGraph& g, skr::RG::RenderPassContext& pass_context) {
            SkrZoneScopedN("DrawLive2DMasks");
            skr::InlineMap<CGPUXBindTableId, CGPUXMergedBindTableId, 8> merged_tables;
            cgpu_render_encoder_set_viewport(pass_context.encoder,
                0.0f, 0.0f,
                (float)Csm::kMaskResolution, (float)Csm::kMaskResolution,
                0.f, 1.f);
            cgpu_render_encoder_set_scissor(pass_context.encoder, 0, 0, Csm::kMaskResolution, Csm::kMaskResolution);
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