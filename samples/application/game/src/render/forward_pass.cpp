#include "forward_pass.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_viewport.h"
#include "SkrAnim/components/skin_component.hpp"
#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"

#include <EASTL/vector_map.h>

#include "SkrProfile/profile.h"

using namespace game;

void RenderPassForward::on_update(const skr_primitive_pass_context_t* context) 
{
    namespace rg = skr::render_graph;
    SkrZoneScopedN("ForwardPass Update");

    need_clear = true;

    auto render_graph = context->render_graph;
    auto storage = context->storage;

    if (!anim_query)
    {
        auto sig = "[in]skr::renderer::MeshComponent, [in]skr::anim::AnimComponent";
        *anim_query = dualQ_from_literal(storage, sig);
    }
    // upload skin mesh data
    {
        uint64_t skinVerticesSize = 0; 
        {
            SkrZoneScopedN("CalculateSkinMeshSize");

            auto calcUploadBufferSize = [&](dual_chunk_view_t* r_cv) {
                const auto anims = dual::get_component_ro<skr::anim::AnimComponent>(r_cv);
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    auto* anim = anims + i;
                    const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                    if (use_dynamic_buffer) continue;
                    for (size_t j = 0u; j < anim->buffers.size(); j++)
                    {
                        skinVerticesSize += anim->buffers[j]->get_size();
                    }
                }
            };
            dualQ_get_views(*anim_query, DUAL_LAMBDA(calcUploadBufferSize));
        }
        if (skinVerticesSize == 0) return;

        auto upload_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                builder.set_name(SKR_UTF8("SkinMeshUploadBuffer"))
                    .size(skinVerticesSize)
                    .with_tags(kRenderGraphDefaultResourceTag)
                    .as_upload_buffer();
        });

        render_graph->add_copy_pass(
        [=](rg::RenderGraph& g, rg::CopyPassBuilder& builder) {
            builder.set_name(SKR_UTF8("CopySkinMesh"))
                .from_buffer(upload_buffer_handle.range(0, skinVerticesSize))
                .can_be_lone();
        },
        [this, upload_buffer_handle](rg::RenderGraph& g, rg::CopyPassContext& context) {
            SkrZoneScopedN("CopySkinMesh");

            auto uploadVertices = [&](dual_chunk_view_t* r_cv) {
                const skr::anim::AnimComponent* anims = nullptr;
                {
                    SkrZoneScopedN("FetchAnims");

                    // duel to dependency, anims fetch here may block a bit, waiting CPU skinning job done
                    anims = dual::get_owned_ro<skr::anim::AnimComponent>(r_cv);
                }

                auto upload_buffer = context.resolve(upload_buffer_handle);
                auto mapped = (uint8_t*)upload_buffer->info->cpu_mapped_address;

                // barrier from vb to copy dest
                {
                    SkrZoneScopedN("Barriers");
                    CGPUResourceBarrierDescriptor barrier_desc = {};
                    eastl::vector<CGPUBufferBarrier> barriers;
                    for (uint32_t i = 0; i < r_cv->count; i++)
                    {
                        auto* anim = anims + i;
                        for (size_t j = 0u; j < anim->buffers.size(); j++)
                        {
                            const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                            if (anim->vbs[j] && !use_dynamic_buffer)
                            {
                                CGPUBufferBarrier& barrier = barriers.emplace_back();
                                barrier.buffer = anim->vbs[j];
                                barrier.src_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                                barrier.dst_state = CGPU_RESOURCE_STATE_COPY_DEST;
                            }
                        }
                    }
                    barrier_desc.buffer_barriers = barriers.data();
                    barrier_desc.buffer_barriers_count = (uint32_t)barriers.size();
                    cgpu_cmd_resource_barrier(context.cmd, &barrier_desc);
                }

                // upload
                {
                    SkrZoneScopedN("MemCopies");
                    uint64_t cursor = 0;
                    for (uint32_t i = 0; i < r_cv->count; i++)
                    {
                        auto* anim = anims + i;
                        const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                        if (use_dynamic_buffer) continue;
                        for (size_t j = 0u; j < anim->buffers.size(); j++)
                        {
                            // memcpy
                            memcpy(mapped + cursor, anim->buffers[j]->get_data(), anim->buffers[j]->get_size());

                            // queue cpy
                            CGPUBufferToBufferTransfer b2b = {};
                            b2b.src = upload_buffer;
                            b2b.src_offset = cursor;
                            b2b.dst = anim->vbs[j];
                            b2b.dst_offset = 0;
                            b2b.size = anim->buffers[j]->get_size();
                            cgpu_cmd_transfer_buffer_to_buffer(context.cmd, &b2b);

                            cursor += anim->buffers[j]->get_size();
                        }
                    }
                }
            };
            dualQ_get_views(*anim_query, DUAL_LAMBDA(uploadVertices));
        });   
    }
}

void RenderPassForward::post_update(const skr_primitive_pass_context_t* context) 
{

}

void RenderPassForward::execute(const skr_primitive_pass_context_t* context, skr::span<const skr_primitive_draw_packet_t> drawcalls) 
{
    auto renderGraph = context->render_graph;
    // TODO: multi-viewport
    auto viewport_manager = context->renderer->get_viewport_manager();
    auto viewport = viewport_manager->find_viewport(0u); // the main viewport

    auto depth = renderGraph->create_texture(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            builder.set_name(SKR_UTF8("depth"))
                .extent(viewport->viewport_width, viewport->viewport_height)
                .format(depth_format)
                .allow_depth_stencil();
            if (viewport->viewport_width > 2048) builder.allocate_dedicated();
        });(void)depth;
    if (!drawcalls.size()) return;

    // 1.IMGUI control shading rate
    const char8_t* shadingRateNames[] = {
        u8"1x1", u8"2x2", u8"4x4", 
        u8"1x2", u8"2x1", u8"2x4", u8"4x2"
    };
    const auto ButtonText = skr::format(u8"SwitchShadingRate-{}", shadingRateNames[shading_rate]);
    ImGui::Begin("ShadingRate");
    if (ImGui::Button(ButtonText.c_str()))
    {
        if (shading_rate != CGPU_SHADING_RATE_COUNT - 1)
            shading_rate = (ECGPUShadingRate)(shading_rate + 1);
        else
            shading_rate = CGPU_SHADING_RATE_FULL;
    }
    ImGui::End();
    
    // 2.add a render graph buffer for forward cbuffer
    auto cbuffer = renderGraph->create_buffer(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::BufferBuilder& builder) {
            builder.set_name(SKR_UTF8("forward_cbuffer"))
                .size(sizeof(skr_float4x4_t))
                .memory_usage(CGPU_MEM_USAGE_CPU_TO_GPU)
                .with_flags(CGPU_BCF_PERSISTENT_MAP_BIT)
                .with_tags(kRenderGraphDynamicResourceTag)
                .prefer_on_device()
                .as_uniform_buffer();
        });

    // 3.barrier skin vbs
    renderGraph->add_copy_pass(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::CopyPassBuilder& builder) {
            builder.set_name(SKR_UTF8("BarrierSkinVertexBuffers"))
                .can_be_lone();
        },
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::CopyPassContext& context){
            SkrZoneScopedN("BarrierSkinMeshes");
            CGPUResourceBarrierDescriptor barrier_desc = {};
            eastl::vector<CGPUBufferBarrier> barriers;
            auto barrierVertices = [&](dual_chunk_view_t* r_cv) {
                const skr::anim::AnimComponent* anims = nullptr;
                {
                    SkrZoneScopedN("FetchAnims");
                    // duel to dependency, anims fetch here may block a bit, waiting CPU skinning job done
                    anims = dual::get_owned_ro<skr::anim::AnimComponent>(r_cv);
                }
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    auto* anim = anims + i;
                    for (size_t j = 0u; j < anim->buffers.size(); j++)
                    {
                        const bool use_dynamic_buffer = anim->use_dynamic_buffer;
                        if (anim->vbs[j] && !use_dynamic_buffer)
                        {
                            CGPUBufferBarrier& barrier = barriers.emplace_back();
                            barrier.buffer = anim->vbs[j];
                            barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
                            barrier.dst_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                        }
                    }
                }
                {
                    SkrZoneScopedN("RecordBarrier");
                    barrier_desc.buffer_barriers = barriers.data();
                    barrier_desc.buffer_barriers_count = (uint32_t)barriers.size();
                    cgpu_cmd_resource_barrier(context.cmd, &barrier_desc);
                }
            };
            dualQ_sync(*anim_query);
            dualQ_get_views(*anim_query, DUAL_LAMBDA(barrierVertices));
        });
    
    // 4.add a render graph pass for forward shading
    CGPURootSignatureId root_signature = nullptr;
    for (auto pak : drawcalls)
    {
        root_signature = pak.count ? pak.lists[0].count ? pak.lists[0].drawcalls[0].pipeline->root_signature : nullptr : nullptr;
    }
    if (!root_signature) return; // no drawcalls

    renderGraph->add_render_pass(
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassBuilder& builder) {
            const auto out_color = renderGraph->get_texture(SKR_UTF8("backbuffer"));
            const auto depth_buffer = renderGraph->get_texture(SKR_UTF8("depth"));
            builder.set_name(SKR_UTF8("forward_pass"))
                // we know that the drawcalls always have a same pipeline
                .read(SKR_UTF8("pass_cb"), cbuffer.range(0, sizeof(skr_float4x4_t)))
                .write(0, out_color, need_clear ? CGPU_LOAD_ACTION_CLEAR : CGPU_LOAD_ACTION_LOAD);
            if (need_clear)
                builder.set_depth_stencil(depth_buffer.clear_depth(1.f));
            else
                builder.set_depth_stencil(depth_buffer, CGPU_LOAD_ACTION_LOAD);
            need_clear = false;
        },
        [=](skr::render_graph::RenderGraph& g, skr::render_graph::RenderPassContext& pass_context) {
            auto cb = pass_context.resolve(cbuffer);
            SKR_ASSERT(cb && "cbuffer not found");
            ::memcpy(cb->info->cpu_mapped_address, &viewport->view_projection, sizeof(viewport->view_projection));
            cgpu_render_encoder_set_viewport(pass_context.encoder,
                0.0f, 0.0f,
                (float)viewport->viewport_width, (float)viewport->viewport_height,
                0.f, 1.f);
            cgpu_render_encoder_set_scissor(pass_context.encoder,
                0, 0, 
                viewport->viewport_width, viewport->viewport_height);
            
            {
            SkrZoneScopedN("DrawCalls");
            CGPURenderPipelineId old_pipeline = nullptr;
            eastl::vector_map<CGPURootSignatureId, CGPUXBindTableId> bind_tables;
            for (uint32_t i = 0; i < drawcalls.size(); i++)
            for (uint32_t j = 0; j < drawcalls[i].count; j++)
            for (uint32_t k = 0; k < drawcalls[i].lists[j].count; k++)
            {
                auto&& dc = drawcalls[i].lists[j].drawcalls[k];
                if (dc.desperated || (dc.index_buffer.buffer == nullptr) || (dc.vertex_buffer_count == 0)) continue;
                
                if (old_pipeline != dc.pipeline)
                {
                    cgpu_render_encoder_bind_pipeline(pass_context.encoder, dc.pipeline);
                    old_pipeline = dc.pipeline;
                }

                CGPURootSignatureId dcRS = dc.pipeline->root_signature;
                if (bind_tables.find(dcRS) == bind_tables.end())
                {
                    bind_tables[dcRS] = pass_context.create_and_update_bind_table(dc.pipeline->root_signature);
                }
                CGPUXBindTableId pass_table = bind_tables[dcRS];
                if (dc.bind_table)
                {
                    CGPUXBindTableId tables[2] = { dc.bind_table, pass_table };
                    pass_context.merge_and_bind_tables(tables, 2);
                }
                else
                {
                    cgpux_render_encoder_bind_bind_table(pass_context.encoder, pass_table);
                }

                {
                    cgpu_render_encoder_bind_index_buffer(pass_context.encoder, 
                        dc.index_buffer.buffer, dc.index_buffer.stride, dc.index_buffer.offset);
                    CGPUBufferId vertex_buffers[16] = { 0 };
                    uint32_t strides[16] = { 0 };
                    uint32_t offsets[16] = { 0 };
                    for (size_t i = 0; i < dc.vertex_buffer_count; i++)
                    {
                        vertex_buffers[i] = dc.vertex_buffers[i].buffer;
                        strides[i] = dc.vertex_buffers[i].stride;
                        offsets[i] = dc.vertex_buffers[i].offset;
                    }
                    for (size_t i = 0; i < dc.vertex_buffer_count; i++)
                    {
                        if (strides[i] == 0) offsets[i] = 0;
                    }
                    cgpu_render_encoder_bind_vertex_buffers(pass_context.encoder, dc.vertex_buffer_count, vertex_buffers, strides, offsets);
                }
                cgpu_render_encoder_push_constants(pass_context.encoder, dc.pipeline->root_signature, dc.push_const_name, dc.push_const);
                cgpu_render_encoder_set_shading_rate(pass_context.encoder, shading_rate, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH, CGPU_SHADING_RATE_COMBINER_PASSTHROUGH);
                cgpu_render_encoder_draw_indexed_instanced(pass_context.encoder, dc.index_buffer.index_count, dc.index_buffer.first_index, 1, 0, 0);
            }
            }
    });
}