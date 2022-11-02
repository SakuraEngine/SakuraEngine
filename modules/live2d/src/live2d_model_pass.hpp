#pragma once
#include "render_graph/api.h"
#include "skr_renderer/primitive_draw.h"
#include "skr_renderer/skr_renderer.h"
#include "live2d_helpers.hpp"

#include "tracy/Tracy.hpp"

const skr_render_pass_name_t live2d_pass_name = "Live2DPass";

struct RenderPassLive2D : public IPrimitiveRenderPass {
    void on_register(SRendererId renderer) override
    {

    }

    void on_unregister(SRendererId renderer) override
    {

    }

    void execute(skr::render_graph::RenderGraph* renderGraph, skr_primitive_draw_list_view_t drawcalls) override
    {
        auto backbuffer = renderGraph->get_texture("backbuffer");
        auto& back_desc = renderGraph->resolve(backbuffer)->get_desc();
        auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            builder.set_name("depth")
                .extent(back_desc.width, back_desc.height)
                .format(live2d_depth_format)
                .owns_memory()
                .allow_depth_stencil();
        });(void)depth;
        if (drawcalls.count)
        {
            renderGraph->add_render_pass(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                const auto out_color = renderGraph->get_texture("backbuffer");
                const auto depth_buffer = renderGraph->get_texture("depth");
                const auto mask_buffer = renderGraph->get_texture("live2d_mask");
                builder.set_name("live2d_forward_pass")
                    .set_root_signature(drawcalls.drawcalls->pipeline->root_signature)
                    .read("mask_texture", mask_buffer)
                    .write(0, out_color, CGPU_LOAD_ACTION_CLEAR)
                    .set_depth_stencil(depth_buffer);
            },
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)back_desc.width, (float)back_desc.height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, back_desc.width, back_desc.height);
                CGPURenderPipelineId old_pipeline = nullptr;
                for (uint32_t i = 0; i < drawcalls.count; i++)
                {
                    ZoneScopedN("DrawCall");

                    auto&& dc = drawcalls.drawcalls[i];
                    if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;

                    if (old_pipeline != dc.pipeline)
                    {
                        cgpu_render_encoder_bind_pipeline(stack.encoder, dc.pipeline);
                        old_pipeline = dc.pipeline;
                    }
                    {
                        ZoneScopedN("BindTextures");
                        for (uint32_t j = 0; j < dc.descriptor_set_count; j++)
                        {
                            cgpu_render_encoder_bind_descriptor_set(stack.encoder, dc.descriptor_sets[j]);
                        }
                    }
                    {
                        ZoneScopedN("BindGeometry");
                        cgpu_render_encoder_bind_index_buffer(stack.encoder, dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
                        CGPUBufferId vertex_buffers[2] = {
                            dc.vertex_buffers[0].buffer, dc.vertex_buffers[1].buffer
                        };
                        const uint32_t strides[2] = {
                            dc.vertex_buffers[0].stride, dc.vertex_buffers[1].stride
                        };
                        const uint32_t offsets[2] = {
                            dc.vertex_buffers[0].offset, dc.vertex_buffers[1].offset
                        };
                        cgpu_render_encoder_bind_vertex_buffers(stack.encoder, 2, vertex_buffers, strides, offsets);
                    }
                    {
                        ZoneScopedN("PushConstants");
                        cgpu_render_encoder_push_constants(stack.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                    }
                    {
                        ZoneScopedN("DrawIndexed");
                        cgpu_render_encoder_draw_indexed_instanced(stack.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
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