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
    // compile
    for (auto& pass : passes)
    {
        // init resources states
        graph->foreach_incoming_edges(pass,
            [=](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* edge) {

            });
    }
    return true;
}

uint64_t RenderGraph::execute()
{
    graph->clear();
    return frame_index++;
}

uint64_t RenderGraphBackend::execute()
{
    RenderGraphFrameExecutor& executor = executors[0];
    cgpu_reset_command_pool(executor.gfx_cmd_pool);
    cgpu_cmd_begin(executor.gfx_cmd_buf);
    for (auto& pass : passes)
    {
        (void)pass;
    }
    cgpu_cmd_end(executor.gfx_cmd_buf);
    // submit
    CGpuQueueSubmitDescriptor submit_desc = {
        .cmds = &executor.gfx_cmd_buf,
        .cmds_count = 1,
    };
    cgpu_submit_queue(gfx_queue, &submit_desc);
    graph->clear();
    return frame_index++;
}
} // namespace render_graph
} // namespace sakura