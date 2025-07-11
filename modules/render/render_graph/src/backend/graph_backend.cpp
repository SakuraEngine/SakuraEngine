#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrCore/memory/sp.hpp"
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrGraphics/cgpux.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"

#include "SkrProfile/profile.h"

namespace skr
{
namespace render_graph
{
// Render Graph Executor

void RenderGraphFrameExecutor::initialize(CGPUQueueId gfx_queue, CGPUDeviceId device)
{
    CGPUCommandPoolDescriptor pool_desc = {
        u8"RenderGraphCmdPool"
    };
    gfx_cmd_pool                         = cgpu_create_command_pool(gfx_queue, &pool_desc);
    CGPUCommandBufferDescriptor cmd_desc = {};
    cmd_desc.is_secondary                = false;
    gfx_cmd_buf                          = cgpu_create_command_buffer(gfx_cmd_pool, &cmd_desc);
    exec_fence                           = cgpu_create_fence(device);

    CGPUBufferDescriptor marker_buffer_desc = {};
    marker_buffer_desc.name = u8"MarkerBuffer";
    marker_buffer_desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
    marker_buffer_desc.memory_usage = CGPU_MEM_USAGE_GPU_TO_CPU;
    marker_buffer_desc.size = 1024 * sizeof(uint32_t);
    marker_buffer = cgpu_create_buffer(device, &marker_buffer_desc);
}

void RenderGraphFrameExecutor::commit(CGPUQueueId gfx_queue, uint64_t frame_index)
{
    CGPUQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds                      = &gfx_cmd_buf;
    submit_desc.cmds_count                = 1;
    submit_desc.signal_fence              = exec_fence;
    cgpu_submit_queue(gfx_queue, &submit_desc);
    exec_frame = frame_index;
}

void RenderGraphFrameExecutor::reset_begin(TextureViewPool& texture_view_pool)
{
    {
        SkrZoneScopedN("ResetBindTables");
        for (auto bind_table_pool : bind_table_pools)
        {
            bind_table_pool.second->reset();
        }
    }

    {
        SkrZoneScopedN("ResetMergedTables");
        for (auto merged_table_pool : merged_table_pools)
        {
            merged_table_pool.second->reset();
        }
    }

    {
        SkrZoneScopedN("ResetAliasingBinds");
        for (auto aliasing_texture : aliasing_textures)
        {
            texture_view_pool.erase(aliasing_texture);
            cgpu_free_texture(aliasing_texture);
        }
        aliasing_textures.clear();
    }

    {
        SkrZoneScopedN("ResetMarkerBuffer");
        marker_idx = 0;
        marker_messages.clear();
        valid_marker_val++;
    }

    {
        SkrZoneScopedN("ResetCommandPool");
        cgpu_reset_command_pool(gfx_cmd_pool);
    }

    cgpu_cmd_begin(gfx_cmd_buf);
    write_marker(u8"Frame Begin");
}

void RenderGraphFrameExecutor::write_marker(const char8_t* message)
{
    CGPUFillBufferDescriptor fill_desc = {
        .offset = marker_idx * sizeof(uint32_t),
        .value = valid_marker_val
    };
    marker_idx += 1;
    cgpu_cmd_fill_buffer(gfx_cmd_buf, marker_buffer, &fill_desc);
    marker_messages.add(message);
}

void RenderGraphFrameExecutor::print_error_trace(uint64_t frame_index)
{
    auto fill_data = (const uint32_t*)marker_buffer->info->cpu_mapped_address;
    if (fill_data[0] == 0) return; // begin cmd is unlikely to fail on gpu
    SKR_LOG_FATAL(u8"Device lost caused by GPU command buffer failure detected %d frames ago, command trace:", frame_index - exec_frame);
    for (uint32_t i = 0; i < marker_messages.size(); i++)
    {
        if (fill_data[i] != valid_marker_val)
        {
            SKR_LOG_ERROR(u8"\tFailed Command %d: %s (marker %d)", i, marker_messages[i].c_str(), fill_data[i]);
        }
        else
        {
            SKR_LOG_INFO(u8"\tCommand %d: %s (marker %d)", i, marker_messages[i].c_str(), fill_data[i]);
        }
    }
    skr_thread_sleep(2000);
}

CGPUXMergedBindTableId RenderGraphFrameExecutor::merge_tables(const struct CGPUXBindTable** tables, uint32_t count)
{
    auto root_sig = tables[0]->GetRootSignature();
    if (merged_table_pools.find(root_sig) == merged_table_pools.end())
    {
        merged_table_pools[root_sig] = SkrNew<MergedBindTablePool>(root_sig);
    }
    return merged_table_pools[root_sig]->pop(tables, count);
}

void RenderGraphFrameExecutor::finalize()
{
    if (gfx_cmd_buf) cgpu_free_command_buffer(gfx_cmd_buf);
    if (gfx_cmd_pool) cgpu_free_command_pool(gfx_cmd_pool);
    if (exec_fence) cgpu_free_fence(exec_fence);
    gfx_cmd_buf  = nullptr;
    gfx_cmd_pool = nullptr;
    exec_fence   = nullptr;
    for (auto [rs, pool] : bind_table_pools)
    {
        pool->destroy();
        SkrDelete(pool);
    }
    for (auto [rs, pool] : merged_table_pools)
    {
        pool->destroy();
        SkrDelete(pool);
    }
    for (auto aliasing_tex : aliasing_textures)
    {
        cgpu_free_texture(aliasing_tex);
    }
    if (marker_buffer) 
        cgpu_free_buffer(marker_buffer);
}

// Render Graph Backend

RenderGraphBackend::RenderGraphBackend(const RenderGraphBuilder& builder)
    : RenderGraph(builder)
    , device(builder.device)
    , gfx_queue(builder.gfx_queue)
    , cmpt_queues(builder.cmpt_queues)
    , cpy_queues(builder.cpy_queues)
{

}


RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup) SKR_NOEXCEPT
{
    RenderGraphBuilder builder = {};
    RenderGraph*       graph   = nullptr;
    setup(builder);
    if (builder.no_backend)
        graph = SkrNew<RenderGraph>(builder);
    else
    {
        if (!builder.gfx_queue) assert(0 && "not supported!");
        graph = SkrNew<RenderGraphBackend>(builder);
    }
    graph->initialize();
    return graph;
}

void RenderGraph::destroy(RenderGraph* g) SKR_NOEXCEPT
{
    g->finalize();
    SkrDelete(g);
}

void RenderGraphBackend::initialize() SKR_NOEXCEPT
{
    RenderGraph::initialize();
    backend = device->adapter->instance->backend;
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].initialize(gfx_queue, device);
    }
    buffer_pool.initialize(device);
    texture_pool.initialize(device);
    texture_view_pool.initialize(device);

    for (auto& phase : phases)
        phase->on_initialize(this);
}

void RenderGraphBackend::finalize() SKR_NOEXCEPT
{
    RenderGraph::finalize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].finalize();
    }
    buffer_pool.finalize();
    texture_pool.finalize();
    texture_view_pool.finalize();

    for (auto& phase : phases)
        phase->on_finalize(this);
}

uint64_t RenderGraphBackend::get_latest_finished_frame() SKR_NOEXCEPT
{
    if (frame_index < RG_MAX_FRAME_IN_FLIGHT) return 0;

    uint64_t result = frame_index - RG_MAX_FRAME_IN_FLIGHT;
    for (auto&& executor : executors)
    {
        if (!executor.exec_fence) continue;
        if (cgpu_query_fence_status(executor.exec_fence) == CGPU_FENCE_STATUS_COMPLETE)
        {
            result = std::max(result, executor.exec_frame);
        }
    }
    return result;
}


const CGPUShaderResource* find_shader_resource(uint64_t name_hash, CGPURootSignatureId root_sig, ECGPUResourceType* type = nullptr)
{
    for (uint32_t i = 0; i < root_sig->table_count; i++)
    {
        for (uint32_t j = 0; j < root_sig->tables[i].resources_count; j++)
        {
            const auto& resource = root_sig->tables[i].resources[j];
            if (resource.name_hash == name_hash && 
                strcmp((const char*)resource.name, (const char*)root_sig->tables[i].resources[j].name) == 0)
            {
                if (type)
                    *type = resource.type;
                return &root_sig->tables[i].resources[j];
            }
        }
    }
    return nullptr;
}

uint64_t RenderGraphBackend::execute(RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    for (auto& phase : phases)
        phase->on_execute(this, profiler);

    return frame_index++;
}

CGPUDeviceId RenderGraphBackend::get_backend_device() SKR_NOEXCEPT { return device; }

uint32_t RenderGraphBackend::collect_garbage(uint64_t critical_frame,
                                             uint32_t tex_with_tags, uint32_t tex_without_tags,
                                             uint32_t buf_with_tags, uint32_t buf_without_tags) SKR_NOEXCEPT
{
    return collect_texture_garbage(critical_frame, tex_with_tags, tex_without_tags) + collect_buffer_garbage(critical_frame, buf_with_tags, buf_without_tags);
}

uint32_t RenderGraphBackend::collect_texture_garbage(uint64_t critical_frame, uint32_t with_tags, uint32_t without_tags) SKR_NOEXCEPT
{
    for (auto& phase : phases)
        phase->on_collect_texture_garbage(this, critical_frame, with_tags, without_tags);

    if (critical_frame > get_latest_finished_frame())
    {
        SKR_LOG_ERROR(u8"undone frame on GPU detected, collect texture garbage may cause GPU Crash!!"
                      "\n\tcurrent: %d, latest finished: %d",
                      critical_frame, get_latest_finished_frame());
    }
    uint32_t total_count = 0;
    for (auto&& [key, queue] : texture_pool.textures)
    {
        for (auto&& pooled : queue)
        {
            if (pooled.mark.frame_index <= critical_frame && (pooled.mark.tags & with_tags) && !(pooled.mark.tags & without_tags))
            {
                texture_view_pool.erase(pooled.texture);
                cgpu_free_texture(pooled.texture);
                pooled.texture = nullptr;
            }
        }
        uint32_t prev_count = (uint32_t)queue.size();
        queue.erase(
        std::remove_if(queue.begin(), queue.end(),
                         [&](auto& element) {
                             return element.texture == nullptr;
                         }),
        queue.end());
        total_count += prev_count - (uint32_t)queue.size();
    }
    return total_count;
}

uint32_t RenderGraphBackend::collect_buffer_garbage(uint64_t critical_frame, uint32_t with_tags, uint32_t without_tags) SKR_NOEXCEPT
{
    for (auto& phase : phases)
        phase->on_collect_buffer_garbage(this, critical_frame, with_tags, without_tags);

    if (critical_frame > get_latest_finished_frame())
    {
        SKR_LOG_ERROR(u8"undone frame on GPU detected, collect buffer garbage may cause GPU Crash!!"
                      "\n\tcurrent: %d, latest finished: %d",
                      critical_frame, get_latest_finished_frame());
    }
    uint32_t total_count = 0;
    for (auto&& [key, queue] : buffer_pool.buffers)
    {
        for (auto&& pooled : queue)
        {
            if (pooled.mark.frame_index <= critical_frame && (pooled.mark.tags & with_tags) && !(pooled.mark.tags & without_tags))
            {
                cgpu_free_buffer(pooled.buffer);
                pooled.buffer = nullptr;
            }
        }
        uint32_t prev_count = (uint32_t)queue.size();
        queue.erase(
        std::remove_if(queue.begin(), queue.end(),
                         [&](auto& element) {
                             return element.buffer == nullptr;
                         }),
        queue.end());
        total_count += prev_count - (uint32_t)queue.size();
    }
    return total_count;
}
} // namespace render_graph
} // namespace skr