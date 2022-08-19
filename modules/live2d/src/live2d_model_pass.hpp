#pragma once
#include "render_graph/api.h"
#include "skr_renderer/primitive_draw.h"
#include "skr_renderer/skr_renderer.h"
#include "skr_renderer/mesh_resource.h"

#include "tracy/Tracy.hpp"

const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT;
const skr_render_pass_name_t live2d_pass_name = "Live2DPass";

struct RenderPassLive2D : public IPrimitiveRenderPass {
    void on_register(ISkrRenderer* renderer) override
    {

    }

    void on_unregister(ISkrRenderer* renderer) override
    {

    }

    void execute(skr::render_graph::RenderGraph* renderGraph, skr_primitive_draw_list_view_t drawcalls) override
    {
        auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            builder.set_name("depth")
                .extent(900, 900)
                .format(depth_format)
                .owns_memory()
                .allow_depth_stencil();
        });
        if (drawcalls.count)
        {
            renderGraph->add_render_pass(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
                const auto out_color = renderGraph->get_texture("backbuffer");
                const auto depth_buffer = renderGraph->get_texture("depth");
                builder.set_name("live2d_forward_pass")
                    // we know that the drawcalls always have a same pipeline
                    .set_pipeline(drawcalls.drawcalls->pipeline)
                    .write(0, out_color, CGPU_LOAD_ACTION_CLEAR)
                    .set_depth_stencil(depth_buffer);
            },
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)900, (float)900,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, 900, 900);
                for (uint32_t i = 0; i < drawcalls.count - 2; i++)
                {
                    ZoneScopedN("DrawCall");

                    auto&& dc = drawcalls.drawcalls[i];
                    if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;
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