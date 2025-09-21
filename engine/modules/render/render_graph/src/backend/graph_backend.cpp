#include "SkrRenderGraph/stack_allocator.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrGraphics/cgpux.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"
#include "SkrProfile/profile.h"

#include "SkrRenderGraph/phases_v2/cull_phase.hpp"
#include "SkrRenderGraph/phases_v2/pass_info_analysis.hpp"
#include "SkrRenderGraph/phases_v2/pass_dependency_analysis.hpp"
#include "SkrRenderGraph/phases_v2/queue_schedule.hpp"
#include "SkrRenderGraph/phases_v2/schedule_reorder.hpp"
#include "SkrRenderGraph/phases_v2/resource_lifetime_analysis.hpp"
#include "SkrRenderGraph/phases_v2/cross_queue_sync_analysis.hpp"
#include "SkrRenderGraph/phases_v2/memory_aliasing_phase.hpp"
#include "SkrRenderGraph/phases_v2/barrier_generation_phase.hpp"
#include "SkrRenderGraph/phases_v2/resource_allocation_phase.hpp"
#include "SkrRenderGraph/phases_v2/bind_table_phase.hpp"
#include "SkrRenderGraph/phases_v2/pass_execution_phase.hpp"

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
    gfx_cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    CGPUCommandBufferDescriptor cmd_desc = {
        .name = u8"CommandBuffer-RenderGraphMain"
    };
    cmd_desc.is_secondary = false;
    gfx_cmd_buf = cgpu_create_command_buffer(gfx_cmd_pool, &cmd_desc);
    exec_fence = cgpu_create_fence(device);

    CGPUBufferDescriptor marker_buffer_desc = {};
    marker_buffer_desc.name = u8"MarkerBuffer";
    marker_buffer_desc.flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT;
    marker_buffer_desc.memory_usage = CGPU_MEM_USAGE_GPU_TO_CPU;
    marker_buffer_desc.size = 1024 * sizeof(uint32_t);
    marker_buffer = cgpu_create_buffer(device, &marker_buffer_desc);
}

void RenderGraphFrameExecutor::commit(CGPUQueueId gfx_queue, uint64_t frame_index)
{
    CGPUQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &gfx_cmd_buf;
    submit_desc.cmds_count = 1;
    submit_desc.signal_fence = exec_fence;
    cgpu_submit_queue(gfx_queue, &submit_desc);
    exec_frame = frame_index;
}

void RenderGraphFrameExecutor::reset_begin()
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
        SkrZoneScopedN("ResetMarkerBuffer");
        marker_idx = 0;
        marker_messages.clear();
        valid_marker_val++;
    }

    {
        SkrZoneScopedN("ResetCommandPool");
        cgpu_reset_command_pool(gfx_cmd_pool);
    }

    {
        SkrZoneScopedN("BeginCommandBuffer");
        cgpu_cmd_begin(gfx_cmd_buf);
    }
    write_marker(u8"Frame Begin");
}

void RenderGraphFrameExecutor::write_marker(const char8_t* message)
{
    SkrZoneScopedN("WriteMarker");

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
    gfx_cmd_buf = nullptr;
    gfx_cmd_pool = nullptr;
    exec_fence = nullptr;
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

static std::atomic_uint64_t rg_count = 0;
RenderGraph* RenderGraph::create(const RenderGraphSetupFunction& setup) SKR_NOEXCEPT
{
    if (rg_count == 0)
    {
        rg_count += 1;
        RenderGraphStackAllocator::Initialize();
    }

    RenderGraphBuilder builder = {};
    RenderGraph* graph = nullptr;
    setup(builder);
    if (builder.no_backend)
    {
        graph = SkrNew<RenderGraph>(builder);
    }
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

    if (--rg_count == 0)
    {
        RenderGraphStackAllocator::Finalize();
    }
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
    buffer_view_pool.initialize(device);
}

void RenderGraphBackend::finalize() SKR_NOEXCEPT
{
    RenderGraph::finalize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        executors[i].finalize();
    }
    buffer_pool.finalize();
    buffer_view_pool.finalize();
    texture_pool.finalize();
    texture_view_pool.finalize();
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

void RenderGraphBackend::wait_frame(uint64_t to_wait) SKR_NOEXCEPT
{
    SkrZoneScopedN("WaitFrame");
    if (to_wait > frame_index)
        return;
    if (to_wait < frame_index - RG_MAX_FRAME_IN_FLIGHT)
        return;
    const auto executor_to_wait = to_wait % RG_MAX_FRAME_IN_FLIGHT;
    cgpu_wait_fences(&executors[executor_to_wait].exec_fence, 1);
}

bool RenderGraphBackend::check_frame(uint64_t to_check) SKR_NOEXCEPT
{
    SkrZoneScopedN("WaitFrame");
    if (to_check > frame_index)
        return false;
    if (to_check < frame_index - RG_MAX_FRAME_IN_FLIGHT)
        return true;
    return to_check <= get_latest_finished_frame();
}

uint64_t RenderGraphBackend::execute(RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    for (auto callback : exec_callbacks)
        callback(*this);
    exec_callbacks.clear();

    const auto executor_index = frame_index % RG_MAX_FRAME_IN_FLIGHT;
    RenderGraphStackAllocator::Reset();
    
    // Wait for executor to be available
    {
        SkrZoneScopedN("AcquireExecutor");
        cgpu_wait_fences(&executors[executor_index].exec_fence, 1);
        if (profiler) profiler->on_acquire_executor(*this, executors[executor_index]);
    }
    
    {
        auto culling = CullPhase();
        culling.on_execute(this, &executors[executor_index], profiler);

        auto info_analysis = PassInfoAnalysis();
        info_analysis.on_execute(this, &executors[executor_index], profiler);

        auto dependency_analysis = PassDependencyAnalysis(info_analysis);
        dependency_analysis.on_execute(this, &executors[executor_index], profiler);

        auto queue_schedule = QueueSchedule(dependency_analysis);
        queue_schedule.on_execute(this, &executors[executor_index], profiler);

        auto reorder_phase = ExecutionReorderPhase(info_analysis, dependency_analysis, queue_schedule);
        reorder_phase.on_execute(this, &executors[executor_index], profiler);

        auto lifetime_analysis = ResourceLifetimeAnalysis(info_analysis, dependency_analysis, queue_schedule);
        lifetime_analysis.on_execute(this, &executors[executor_index], profiler);

        auto ssis_phase = CrossQueueSyncAnalysis(dependency_analysis, queue_schedule);
        ssis_phase.on_execute(this, &executors[executor_index], profiler);

        auto aliasing_phase = MemoryAliasingPhase(info_analysis, lifetime_analysis, ssis_phase, MemoryAliasingConfig{ .aliasing_tier = EAliasingTier::Tier0 });
        aliasing_phase.on_execute(this, &executors[executor_index], profiler);

        auto barrier_phase = BarrierGenerationPhase(ssis_phase, aliasing_phase, info_analysis, reorder_phase);
        barrier_phase.on_execute(this, &executors[executor_index], profiler);

        auto resource_allocation_phase = ResourceAllocationPhase(aliasing_phase, info_analysis);
        resource_allocation_phase.on_execute(this, &executors[executor_index], profiler);

        auto bindtable_phase = BindTablePhase(info_analysis, resource_allocation_phase);
        bindtable_phase.on_execute(this, &executors[executor_index], profiler);

        auto execution_phase = PassExecutionPhase(queue_schedule, reorder_phase, ssis_phase, barrier_phase, resource_allocation_phase, bindtable_phase);
        execution_phase.on_execute(this, &executors[executor_index], profiler);

#if !SKR_SHIPPING
        if (frame_index == 0)
        {
            GraphViz::generate_graphviz_visualization(this, info_analysis, queue_schedule, ssis_phase, barrier_phase, aliasing_phase, lifetime_analysis);
        }
#endif
    }

    {
        SkrZoneScopedN("GraphCleanup");

        // 3.dealloc passes & connected edges
        for (auto pass : passes)
        {
            pass->foreach_textures(
                [this](TextureNode* t, TextureEdge* e) {
                    node_factory->Dealloc(e);
                });
            pass->foreach_buffers(
                [this](BufferNode* t, BufferEdge* e) {
                    node_factory->Dealloc(e);
                });
            pass->foreach_acceleration_structures(
                [this](AccelerationStructureNode* as, AccelerationStructureEdge* e) {
                    node_factory->Dealloc(e);
                });
            node_factory->Dealloc(pass);
        }
        passes.clear();
        // 4.dealloc resource nodes
        for (auto resource : resources)
        {
            node_factory->Dealloc(resource);
        }
        resources.clear();

        graph->clear();
        blackboard->clear();
        imported_textures.clear();
        imported_buffers.clear();
        imported_acceleration_structures.clear();
    }

    return frame_index++;
}

CGPUDeviceId RenderGraphBackend::get_backend_device() SKR_NOEXCEPT { return device; }

uint32_t RenderGraphBackend::collect_garbage(uint64_t critical_frame,
    uint32_t tex_with_tags,
    uint32_t tex_without_tags,
    uint32_t buf_with_tags,
    uint32_t buf_without_tags) SKR_NOEXCEPT
{
    return collect_texture_garbage(critical_frame, tex_with_tags, tex_without_tags) + collect_buffer_garbage(critical_frame, buf_with_tags, buf_without_tags);
}

uint32_t RenderGraphBackend::collect_texture_garbage(uint64_t critical_frame, uint32_t with_tags, uint32_t without_tags) SKR_NOEXCEPT
{
    if (critical_frame > get_latest_finished_frame())
    {
        SKR_LOG_ERROR(u8"undone frame on GPU detected, collect texture garbage may cause GPU Crash!!"
                      "\n\tcurrent: %d, latest finished: %d",
            critical_frame,
            get_latest_finished_frame());
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
            std::remove_if(queue.begin(), queue.end(), [&](auto& element) {
                return element.texture == nullptr;
            }),
            queue.end());
        total_count += prev_count - (uint32_t)queue.size();
    }
    return total_count;
}

uint32_t RenderGraphBackend::collect_buffer_garbage(uint64_t critical_frame, uint32_t with_tags, uint32_t without_tags) SKR_NOEXCEPT
{
    if (critical_frame > get_latest_finished_frame())
    {
        SKR_LOG_ERROR(u8"undone frame on GPU detected, collect buffer garbage may cause GPU Crash!!"
                      "\n\tcurrent: %d, latest finished: %d",
            critical_frame,
            get_latest_finished_frame());
    }
    uint32_t total_count = 0;
    for (auto&& [key, queue] : buffer_pool.buffers)
    {
        for (auto&& pooled : queue)
        {
            if (pooled.mark.frame_index <= critical_frame && (pooled.mark.tags & with_tags) && !(pooled.mark.tags & without_tags))
            {
                buffer_view_pool.erase(pooled.buffer);
                cgpu_free_buffer(pooled.buffer);
                pooled.buffer = nullptr;
            }
        }
        uint32_t prev_count = (uint32_t)queue.size();
        queue.erase(
            std::remove_if(queue.begin(), queue.end(), [&](auto& element) {
                return element.buffer == nullptr;
            }),
            queue.end());
        total_count += prev_count - (uint32_t)queue.size();
    }
    return total_count;
}
} // namespace render_graph
} // namespace skr