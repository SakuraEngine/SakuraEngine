#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "containers/string.hpp"

class PassProfiler : public skr::render_graph::RenderGraphProfiler
{
public:
    void initialize(CGPUDeviceId device)
    {
        CGPUQueryPoolDescriptor desc = {};
        desc.query_count = 512;
        desc.type = CGPU_QUERY_TYPE_TIMESTAMP;
        query_pool = cgpu_create_query_pool(device, &desc);
        CGPUBufferDescriptor buf_desc = {};
        buf_desc.name = u8"RenderGraphQueryBuffer";
        buf_desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
        buf_desc.memory_usage = CGPU_MEM_USAGE_GPU_TO_CPU;
        buf_desc.size = sizeof(uint64_t) * 512;
        query_buffer = cgpu_create_buffer(device, &buf_desc);
    }
    void finalize()
    {
        if (query_pool) cgpu_free_query_pool(query_pool);
        if (query_buffer) cgpu_free_buffer(query_buffer);
        query_pool = nullptr;
        query_buffer = nullptr;
    }
    skr::span<uint64_t> readback_query_data()
    {
        return skr::span<uint64_t>((uint64_t*)query_buffer->cpu_mapped_address, query_cursor);
    }
    virtual void on_acquire_executor(class skr::render_graph::RenderGraph& g, class skr::render_graph::RenderGraphFrameExecutor& e)
    {
        auto timestamps = readback_query_data();
        times_ms.resize(timestamps.size());
        auto ns_period = cgpu_queue_get_timestamp_period_ns(g.get_gfx_queue());
        for (uint32_t i = 1; i < times_ms.size(); i++)
        {
            times_ms[i] = ((timestamps[i] - timestamps[i - 1]) * ns_period) * 1e-6f;
        }
        frame_index = g.get_frame_index() - RG_MAX_FRAME_IN_FLIGHT;
    }
    virtual void on_cmd_begin(class skr::render_graph::RenderGraph& g, class skr::render_graph::RenderGraphFrameExecutor& executor)
    {
        query_cursor = 0;
        query_names.clear();
        cgpu_cmd_reset_query_pool(executor.gfx_cmd_buf, query_pool, 0, 512);
        CGPUQueryDescriptor query_desc = {};
        query_desc.index = query_cursor++;
        query_desc.stage = CGPU_SHADER_STAGE_NONE;
        query_names.emplace_back(u8"cmd_begin");
        cgpu_cmd_begin_query(executor.gfx_cmd_buf, query_pool, &query_desc);
    }
    virtual void on_cmd_end(class skr::render_graph::RenderGraph&, class skr::render_graph::RenderGraphFrameExecutor& executor)
    {
        cgpu_cmd_resolve_query(executor.gfx_cmd_buf, query_pool,
        query_buffer, 0, query_cursor);
    }
    virtual void on_pass_begin(class skr::render_graph::RenderGraph&, class skr::render_graph::RenderGraphFrameExecutor&, class skr::render_graph::PassNode& pass)
    {
    }
    virtual void on_pass_end(class skr::render_graph::RenderGraph&, class skr::render_graph::RenderGraphFrameExecutor& executor, class skr::render_graph::PassNode& pass)
    {
        CGPUQueryDescriptor query_desc = {};
        query_desc.index = query_cursor++;
        query_desc.stage = CGPU_SHADER_STAGE_ALL_GRAPHICS;
        query_names.emplace_back(pass.get_name());
        cgpu_cmd_begin_query(executor.gfx_cmd_buf, query_pool, &query_desc);
    }
    virtual void before_commit(class skr::render_graph::RenderGraph&, class skr::render_graph::RenderGraphFrameExecutor&) {}
    virtual void after_commit(class skr::render_graph::RenderGraph&, class skr::render_graph::RenderGraphFrameExecutor&) {}

    CGPUQueryPoolId query_pool = nullptr;
    CGPUBufferId query_buffer = nullptr;
    uint32_t query_cursor = 0;
    eastl::vector<float> times_ms;
    eastl::vector<skr::string> query_names;
    uint64_t frame_index;
};