#include "SkrRenderGraph/phases/pass_execution_phase.hpp"
#include "SkrRenderGraph/phases/barrier_calculation_phase.hpp"
#include "SkrRenderGraph/phases/bind_table_phase.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrContainers/string.hpp"
#include "SkrProfile/profile.h"

namespace skr {
namespace render_graph {

void PassExecutionPhase::on_compile(RenderGraph* graph) SKR_NOEXCEPT
{
    find_dependent_phases(graph);
}

void PassExecutionPhase::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("PassExecutionPhase::execute");
    
    auto backend = static_cast<RenderGraphBackend*>(graph);
    const auto executor_index = backend->frame_index % RG_MAX_FRAME_IN_FLIGHT;
    RenderGraphFrameExecutor& executor = backend->executors[executor_index];
    
    if (backend->device->is_lost)
    {
        for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
        {
            backend->executors[i].print_error_trace(backend->frame_index);
        }
        SKR_BREAK();
    }
    
    {
        SkrZoneScopedN("AcquireExecutor");
        cgpu_wait_fences(&executor.exec_fence, 1);
        if (profiler) profiler->on_acquire_executor(*backend, executor);
    }
    
    {
        SkrZoneScopedN("GraphExecutePasses");
        executor.reset_begin(backend->texture_view_pool);
        if (profiler) profiler->on_cmd_begin(*backend, executor);
        
        {
            SkrZoneScopedN("GraphExecutorBeginEvent");
            skr::String frameLabel = skr::format(u8"Frame-{}", backend->frame_index);
            CGPUEventInfo event = { (const char8_t*)frameLabel.c_str(), { 0.8f, 0.8f, 0.8f, 1.f } };
            cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
        }
        
        auto& passes = get_passes(graph);
        for (auto& pass : passes)
        {
            if (pass->pass_type == EPassType::Render)
            {
                if (profiler) profiler->on_pass_begin(*backend, executor, *pass);
                execute_render_pass(graph, executor, static_cast<RenderPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*backend, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Present)
            {
                if (profiler) profiler->on_pass_begin(*backend, executor, *pass);
                execute_present_pass(graph, executor, static_cast<PresentPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*backend, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Compute)
            {
                if (profiler) profiler->on_pass_begin(*backend, executor, *pass);
                execute_compute_pass(graph, executor, static_cast<ComputePassNode*>(pass));
                if (profiler) profiler->on_pass_end(*backend, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Copy)
            {
                if (profiler) profiler->on_pass_begin(*backend, executor, *pass);
                execute_copy_pass(graph, executor, static_cast<CopyPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*backend, executor, *pass);
            }
        }
        
        {
            cgpu_cmd_end_event(executor.gfx_cmd_buf);
        }
        if (profiler) profiler->on_cmd_end(*backend, executor);
        cgpu_cmd_end(executor.gfx_cmd_buf);
    }
    
    {
        // submit
        SkrZoneScopedN("GraphQueueSubmit");
        if (profiler) profiler->before_commit(*backend, executor);
        {
            SkrZoneScopedN("CGPUGfxQueueSubmit");
            executor.commit(backend->gfx_queue, backend->frame_index);
        }
        if (profiler) profiler->after_commit(*backend, executor);
    }
}

void PassExecutionPhase::find_dependent_phases(RenderGraph* graph) SKR_NOEXCEPT
{
    auto backend = static_cast<RenderGraphBackend*>(graph);
    for (auto& phase : backend->phases)
    {
        if (auto barrier_phase = skr::dynamic_pointer_cast<BarrierCalculationPhase>(phase))
        {
            barrier_calculation_phase = barrier_phase.get();
        }
        else if (auto bind_phase = skr::dynamic_pointer_cast<BindTablePhase>(phase))
        {
            bind_table_phase = bind_phase.get();
        }
    }
}

void PassExecutionPhase::execute_compute_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, ComputePassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedN("ExecuteComputePass");
    
    ComputePassContext pass_context = {};
    
    if (barrier_calculation_phase)
    {
        skr::Vector<CGPUTextureBarrier> tex_barriers = {};
        skr::Vector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
        skr::Vector<CGPUBufferBarrier> buffer_barriers = {};
        skr::Vector<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers = {};
        
        barrier_calculation_phase->calculate_barriers(graph, executor, pass, 
                                                     tex_barriers, resolved_textures,
                                                     buffer_barriers, resolved_buffers);
        
        CGPUResourceBarrierDescriptor barriers = {};
        if (!tex_barriers.is_empty())
        {
            barriers.texture_barriers = tex_barriers.data();
            barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
        }
        if (!buffer_barriers.is_empty())
        {
            barriers.buffer_barriers = buffer_barriers.data();
            barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
        }
        if (barriers.texture_barriers_count || barriers.buffer_barriers_count)
        {
            cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
        }
    }
    
    pass_context.graph = graph;
    pass_context.pass = pass;
    if (bind_table_phase)
    {
        pass_context.bind_table = bind_table_phase->alloc_update_pass_bind_table(graph, executor, pass, pass->root_signature);
    }
    pass_context.executor = &executor;
    
    pass->executor(graph, pass_context);
}

void PassExecutionPhase::execute_render_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, RenderPassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedN("ExecuteRenderPass");
    
    // Similar implementation to execute_compute_pass but for render passes
    RenderPassContext pass_context = {};
    
    if (barrier_calculation_phase)
    {
        stack_vector<CGPUTextureBarrier> tex_barriers = {};
        stack_vector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
        stack_vector<CGPUBufferBarrier> buffer_barriers = {};
        stack_vector<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers = {};
        
        barrier_calculation_phase->calculate_barriers(graph, executor, pass, 
                                                     tex_barriers, resolved_textures,
                                                     buffer_barriers, resolved_buffers);
        
        CGPUResourceBarrierDescriptor barriers = {};
        if (!tex_barriers.is_empty())
        {
            barriers.texture_barriers = tex_barriers.data();
            barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
        }
        if (!buffer_barriers.is_empty())
        {
            barriers.buffer_barriers = buffer_barriers.data();
            barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
        }
        if (barriers.texture_barriers_count || barriers.buffer_barriers_count)
        {
            cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
        }
    }
    
    pass_context.graph = graph;
    pass_context.pass = pass;
    if (bind_table_phase)
    {
        pass_context.bind_table = bind_table_phase->alloc_update_pass_bind_table(graph, executor, pass, pass->root_signature);
    }
    pass_context.executor = &executor;
    
    pass->executor(graph, pass_context);
}

void PassExecutionPhase::execute_copy_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, CopyPassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedN("ExecuteCopyPass");
    
    CopyPassContext pass_context = {};
    
    if (barrier_calculation_phase)
    {
        stack_vector<CGPUTextureBarrier> tex_barriers = {};
        stack_vector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
        stack_vector<CGPUBufferBarrier> buffer_barriers = {};
        stack_vector<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers = {};
        
        barrier_calculation_phase->calculate_barriers(graph, executor, pass, 
                                                     tex_barriers, resolved_textures,
                                                     buffer_barriers, resolved_buffers);
        
        CGPUResourceBarrierDescriptor barriers = {};
        if (!tex_barriers.is_empty())
        {
            barriers.texture_barriers = tex_barriers.data();
            barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
        }
        if (!buffer_barriers.is_empty())
        {
            barriers.buffer_barriers = buffer_barriers.data();
            barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
        }
        if (barriers.texture_barriers_count || barriers.buffer_barriers_count)
        {
            cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
        }
    }
    
    pass_context.graph = graph;
    pass_context.pass = pass;
    pass_context.executor = &executor;
    
    pass->executor(graph, pass_context);
}

void PassExecutionPhase::execute_present_pass(RenderGraph* graph, RenderGraphFrameExecutor& executor, PresentPassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedN("ExecutePresentPass");
    
    PresentPassContext pass_context = {};
    
    if (barrier_calculation_phase)
    {
        stack_vector<CGPUTextureBarrier> tex_barriers = {};
        stack_vector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
        stack_vector<CGPUBufferBarrier> buffer_barriers = {};
        stack_vector<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers = {};
        
        barrier_calculation_phase->calculate_barriers(graph, executor, pass, 
                                                     tex_barriers, resolved_textures,
                                                     buffer_barriers, resolved_buffers);
        
        CGPUResourceBarrierDescriptor barriers = {};
        if (!tex_barriers.is_empty())
        {
            barriers.texture_barriers = tex_barriers.data();
            barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
        }
        if (!buffer_barriers.is_empty())
        {
            barriers.buffer_barriers = buffer_barriers.data();
            barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
        }
        if (barriers.texture_barriers_count || barriers.buffer_barriers_count)
        {
            cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
        }
    }
    
    pass_context.graph = graph;
    pass_context.pass = pass;
    pass_context.executor = &executor;
    
    pass->executor(graph, pass_context);
}

} // namespace render_graph
} // namespace skr