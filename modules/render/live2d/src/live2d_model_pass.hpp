#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/primitive_pass.h"
#include "live2d_helpers.hpp"

#include "SkrProfile/profile.h"

const skr_render_pass_name_t live2d_pass_name = u8"Live2DPass";

struct RenderPassLive2D : public IPrimitiveRenderPass {
    void on_update(const skr_primitive_pass_context_t* context) override
    {
        auto renderGraph = context->render_graph;

        auto backbuffer = renderGraph->get_texture(u8"backbuffer");
        const auto back_desc = renderGraph->resolve_descriptor(backbuffer);
        auto msaaTarget = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value(u8"l2d_msaa", sample_level);

            builder.set_name(u8"live2d_msaa")
                .extent(back_desc->width, back_desc->height)
                .format(back_desc->format)
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_render_target();
            if (back_desc->height > 2048) builder.allocate_dedicated();
        });(void)msaaTarget;
        
        auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            double sample_level = 1.0;
            g.get_blackboard().value(u8"l2d_msaa", sample_level);

            builder.set_name(u8"depth")
                .extent(back_desc->width, back_desc->height)
                .format(live2d_depth_format)
                .sample_count((ECGPUSampleCount)sample_level)
                .allow_depth_stencil();
            if (back_desc->height > 2048) builder.allocate_dedicated();
        });(void)depth;
    }

    void post_update(const skr_primitive_pass_context_t* context) override
    {

    }

    void execute(const skr_primitive_pass_context_t* context, skr::span<const skr_primitive_draw_packet_t> drawcalls) override
    {
        auto renderGraph = context->render_graph;
        auto backbuffer = renderGraph->get_texture(u8"backbuffer");
        const auto back_desc = renderGraph->resolve_descriptor(backbuffer);
        if (!drawcalls.size()) return;

        CGPURootSignatureId root_signature = nullptr;
        for (auto pak : drawcalls)
        {
            root_signature = pak.count ? pak.lists[0].count ? pak.lists[0].drawcalls[0].pipeline->root_signature : nullptr : nullptr;
        }
        if (!root_signature) return; // no models need to draw

        renderGraph->add_render_pass(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
            double sample_level = 1.0;
            bool useMSAA = g.get_blackboard().value(u8"l2d_msaa", sample_level); useMSAA &= (sample_level > 1.0);
            const auto depth_buffer = renderGraph->get_texture(u8"depth");
            const auto mask_buffer = renderGraph->get_texture(u8"live2d_mask");
            const auto live2d_msaa = renderGraph->get_texture(u8"live2d_msaa");
            const auto backbuffer = renderGraph->get_texture(u8"backbuffer");
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
            for (uint32_t i = 0; i < drawcalls.size(); i++)
                for (uint32_t j = 0; j < drawcalls[i].count; j++)
                    for (uint32_t k = 0; k < drawcalls[i].lists[j].count; k++)
                    {
                        SkrZoneScopedN("UpdateBindTables");
                        auto&& dc = drawcalls[i].lists[j].drawcalls[k];
                        if (!dc.bind_table || dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;
                        
                        CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                        pass_context.merge_tables(tables, 2);
                    }

            cgpu_render_encoder_set_viewport(pass_context.encoder,
                0.0f, 0.0f,
                (float)back_desc->width, (float)back_desc->height,
                0.f, 1.f);
            cgpu_render_encoder_set_scissor(pass_context.encoder, 
                0, 0, (uint32_t)back_desc->width, (uint32_t)back_desc->height);
            CGPURenderPipelineId old_pipeline = nullptr;
            for (uint32_t i = 0; i < drawcalls.size(); i++)
            for (uint32_t j = 0; j < drawcalls[i].count; j++)
            for (uint32_t k = 0; k < drawcalls[i].lists[j].count; k++)
            {
                SkrZoneScopedN("DrawCall");

                auto&& dc = drawcalls[i].lists[j].drawcalls[k];
                if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) 
                    continue;

                if (old_pipeline != dc.pipeline)
                {
                    cgpu_render_encoder_bind_pipeline(pass_context.encoder, dc.pipeline);
                    old_pipeline = dc.pipeline;
                }
                {
                    SkrZoneScopedN("BindTextures");
                    CGPUXBindTableId tables[2] = { dc.bind_table, pass_context.bind_table };
                    pass_context.merge_and_bind_tables(tables, 2);
                }
                {
                    SkrZoneScopedN("BindGeometry");
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
                    SkrZoneScopedN("PushConstants");
                    cgpu_render_encoder_push_constants(pass_context.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                }
                {
                    SkrZoneScopedN("DrawIndexed");
                    cgpu_render_encoder_draw_indexed_instanced(pass_context.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
                }
            }
        });
    }

    skr_render_pass_name_t identity() const override
    {
        return live2d_pass_name;
    }
};