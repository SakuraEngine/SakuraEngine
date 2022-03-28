#include "render_graph/backend/graph_backend.hpp"

namespace sakura
{
namespace render_graph
{
RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup)
{
    RenderGraphBuilder builder = {};
    setup(builder);
    if (builder.no_backend)
    {
        return new RenderGraph();
    }
    else
    {
        if (!builder.gfx_queue)
        {
            assert(0 && "not supported!");
        }
        auto backend_graph = new RenderGraphBackend(builder.gfx_queue, builder.device);
        return backend_graph;
    }
}

bool RenderGraph::compile()
{
    // TODO: cull
    // 1.init resources states
    for (auto& resource : resources)
    {
        switch (resource->type)
        {
            case EObjectType::Texture: {
                auto texture = static_cast<TextureNode*>(resource);
                texture->init_state = RESOURCE_STATE_COMMON;
            }
            break;
            default:
                break;
        }
    }
    // 2.calc lifespan
    for (auto& pass : passes)
    {
        graph->foreach_incoming_edges(pass,
            [=](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* edge) {
                auto rg_from = static_cast<RenderGraphNode*>(from);
                auto rg_to = static_cast<RenderGraphNode*>(to);
                if (rg_from->type == EObjectType::Texture)
                {
                    auto texture_from = static_cast<TextureNode*>(rg_from);
                }
            });
    }
    return true;
}

const ECGpuResourceState RenderGraph::get_lastest_state(TextureHandle texture, PassHandle pending_pass) const
{
    auto in_edges = graph->incoming_edges(texture.handle);
    auto this_tex = static_cast<TextureNode*>(graph->node_at(texture.handle));
    auto result = RESOURCE_STATE_UNDEFINED;
    if (in_edges.size() == 0)
    {
        result = this_tex->init_state;
    }
    else
    {
        dep_graph_handle_t max_idx = 0;
        for (auto&& iter : in_edges)
        {
            auto&& rg_edge = static_cast<RenderGraphEdge&&>(iter);
            if (rg_edge.type == ERelationshipType::TextureRead)
            {
                auto&& read_edge = static_cast<TextureReadEdge&&>(iter);
                auto* pass = static_cast<RenderPassNode*>(read_edge.from());
                if ((max_idx <= pass->get_id()))
                {
                    max_idx = pass->get_id();
                    result = read_edge.requested_state;
                }
            }
        }
    }
    return result;
}

uint64_t RenderGraph::execute()
{
    graph->clear();
    return frame_index++;
}

void RenderGraphBackend::execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass)
{
    auto read_edges = pass->read_edges();
    auto write_edges = pass->write_edges();
    // resource de-virtualize
    // init barriers
    eastl::vector<CGpuTextureBarrier> tex_barriers = {};
    for (auto& read_edge : read_edges)
    {
        auto texture_readed = read_edge->get_texture_node();
        const auto current_state = get_lastest_state(texture_readed->get_handle(), pass->get_handle());
        const auto dst_state = read_edge->requested_state;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture =
            texture_readed->imported ? texture_readed->frame_texture : nullptr;
        tex_barriers.emplace_back(barrier);
    }
    for (auto& write_edge : write_edges)
    {
        auto texture_target = write_edge->get_texture_node();
        const auto current_state = get_lastest_state(texture_target->get_handle(), pass->get_handle());
        const auto dst_state = write_edge->requested_state;
        CGpuTextureBarrier barrier = {};
        barrier.src_state = current_state;
        barrier.dst_state = dst_state;
        barrier.texture =
            texture_target->imported ? texture_target->frame_texture : nullptr;
        tex_barriers.emplace_back(barrier);
    }
    // color attachments
    eastl::vector<CGpuColorAttachment> color_attachments = {};
    for (auto& write_edge : write_edges)
    {
        CGpuColorAttachment attachment = {};
        auto texture_target = write_edge->get_texture_node();
        // TODO: MSAA
        attachment.view = texture_target->imported ? texture_target->default_view : nullptr;
        attachment.load_action = LOAD_ACTION_CLEAR;
        attachment.store_action = STORE_ACTION_STORE;
        color_attachments.emplace_back(attachment);
    }
    // call cgpu apis
    CGpuResourceBarrierDescriptor barriers = {};
    barriers.texture_barriers = tex_barriers.data();
    barriers.texture_barriers_count = tex_barriers.size();
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    // TODO: MSAA
    CGpuRenderPassDescriptor pass_desc = {};
    pass_desc.render_target_count = write_edges.size();
    pass_desc.sample_count = SAMPLE_COUNT_1;
    pass_desc.name = pass->get_name();
    pass_desc.color_attachments = color_attachments.data();
    pass_desc.depth_stencil = nullptr;
    auto encoder = cgpu_cmd_begin_render_pass(executor.gfx_cmd_buf, &pass_desc);
    pass->executor(*this, encoder);
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, encoder);
}

void RenderGraphBackend::execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass)
{
    CGpuTextureBarrier present_barrier = {};
    present_barrier.texture = pass->descriptor.swapchain->back_buffers[pass->descriptor.index];
    present_barrier.src_state = RESOURCE_STATE_RENDER_TARGET; // TODO: !
    present_barrier.dst_state = RESOURCE_STATE_PRESENT;
    CGpuResourceBarrierDescriptor barriers = {};
    barriers.texture_barriers = &present_barrier;
    barriers.texture_barriers_count = 1;
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
}

uint64_t RenderGraphBackend::execute()
{
    RenderGraphFrameExecutor& executor = executors[0];
    cgpu_reset_command_pool(executor.gfx_cmd_pool);
    cgpu_cmd_begin(executor.gfx_cmd_buf);
    for (auto& pass : passes)
    {
        if (pass->pass_type == EPassType::Render)
        {
            execute_render_pass(executor, static_cast<RenderPassNode*>(pass));
        }
        else if (pass->pass_type == EPassType::Present)
        {
            execute_present_pass(executor, static_cast<PresentPassNode*>(pass));
        }
    }
    cgpu_cmd_end(executor.gfx_cmd_buf);
    // submit
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &executor.gfx_cmd_buf;
    submit_desc.cmds_count = 1;
    cgpu_submit_queue(gfx_queue, &submit_desc);
    graph->clear();
    blackboard.clear();
    for (auto pass : passes)
    {
        delete pass;
    }
    passes.clear();
    for (auto resource : resources)
    {
        delete resource;
    }
    resources.clear();
    return frame_index++;
}
} // namespace render_graph
} // namespace sakura