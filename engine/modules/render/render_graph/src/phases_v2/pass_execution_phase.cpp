#include "SkrRenderGraph/phases_v2/pass_execution_phase.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrCore/log.hpp"
#include "SkrProfile/profile.h"
#include "SkrContainers/string.hpp"

#define COMMAND_RECORDING_LOG(...)

namespace skr
{
namespace render_graph
{

PassExecutionPhase::PassExecutionPhase(
    const QueueSchedule& queue_schedule,
    const ExecutionReorderPhase& reorder_phase,
    const CrossQueueSyncAnalysis& sync_analysis,
    const BarrierGenerationPhase& barrier_generation_phase,
    const ResourceAllocationPhase& resource_allocation_phase,
    const BindTablePhase& bind_table_phase,
    const CommandRecordingConfig& config)
    : config_(config)
    , queue_schedule_(queue_schedule)
    , reorder_phase_(reorder_phase)
    , sync_analysis_(sync_analysis)
    , barrier_generation_phase_(barrier_generation_phase)
    , resource_allocation_phase_(resource_allocation_phase)
    , bind_table_phase_(bind_table_phase)
{
}

void PassExecutionPhase::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("PassExecutionPhase");
    COMMAND_RECORDING_LOG(u8"PassExecutionPhase: Starting command recording");

    auto backend = static_cast<RenderGraphBackend*>(graph);

    // Check for device lost
    if (backend->get_backend_device()->is_lost)
    {
        for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
        {
            backend->executors[i].print_error_trace(backend->get_frame_index());
        }
        SKR_BREAK();
    }

    // Begin command recording
    {
        SkrZoneScopedN("GraphExecutePasses");
        {
            SkrZoneScopedN("ResetCommandBuffer");
            executor->reset_begin();
            if (profiler) profiler->on_cmd_begin(*backend, *executor);
        }

        // Begin frame event
        if (config_.enable_debug_markers)
        {
            SkrZoneScopedN("BeginFrameEvent");
            skr::String frameLabel = skr::format(u8"Frame-{}", backend->frame_index);
            CGPUEventInfo event = { (const char8_t*)frameLabel.c_str(), { 0.8f, 0.8f, 0.8f, 1.f } };
            cgpu_cmd_begin_event(executor->gfx_cmd_buf, &event);
        }

        // Execute passes according to schedule
        execute_scheduled_passes(graph, executor, profiler);

        // End frame event
        if (config_.enable_debug_markers)
        {
            cgpu_cmd_end_event(executor->gfx_cmd_buf);
        }

        if (profiler) profiler->on_cmd_end(*backend, *executor);
        cgpu_cmd_end(executor->gfx_cmd_buf);
    }

    // Submit commands
    {
        SkrZoneScopedN("GraphQueueSubmit");
        if (profiler) profiler->before_commit(*backend, *executor);
        {
            SkrZoneScopedN("CGPUGfxQueueSubmit");
            executor->commit(backend->gfx_queue, backend->frame_index);
        }
        if (profiler) profiler->after_commit(*backend, *executor);
    }

    COMMAND_RECORDING_LOG(u8"PassExecutionPhase: Executed {} passes, {} barriers, {} sync points",
        recording_result_.total_passes_executed,
        recording_result_.total_barriers_inserted,
        recording_result_.total_sync_points_processed);
}

void PassExecutionPhase::execute_scheduled_passes(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    // Get the scheduled order from QueueSchedule
    const auto& schedule_result = queue_schedule_.get_schedule_result();

    if (reorder_phase_.get_optimized_timeline().size() > 1)
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    for (uint32_t queue_index = 0; queue_index < schedule_result.all_queues.size(); queue_index++)
    {
        const auto& queue_info = schedule_result.all_queues[queue_index];
        const auto& timeline = reorder_phase_.get_optimized_timeline()[queue_index];
        for (uint32_t pass_idx = 0; pass_idx < timeline.size(); pass_idx++)
        {
            const auto& pass = timeline[pass_idx];
            if (config_.enable_profiler_events && profiler)
            {
                profiler->on_pass_begin(*static_cast<RenderGraphBackend*>(graph), *executor, *pass);
            }

            // Process sync points for this pass
            {
                SkrZoneScopedN("ProcessSyncPoints");
                process_sync_points(executor, pass);
            }

            // Insert barriers before pass execution
            {
                SkrZoneScopedN("InsertPassBarriers");
                insert_pass_barriers(executor, pass);
            }

            // Begin debug marker
            if (config_.enable_debug_markers)
            {
                begin_debug_marker(executor, pass);
            }

            // Execute pass based on type
            switch (pass->pass_type)
            {
            case EPassType::Render:
                execute_render_pass(graph, executor, static_cast<RenderPassNode*>(pass));
                break;
            case EPassType::Compute:
                execute_compute_pass(graph, executor, static_cast<ComputePassNode*>(pass));
                break;
            case EPassType::Copy:
                execute_copy_pass(graph, executor, static_cast<CopyPassNode*>(pass));
                break;
            case EPassType::Present:
                execute_present_pass(graph, executor, static_cast<PresentPassNode*>(pass));
                break;
            default:
                COMMAND_RECORDING_LOG(u8"Unknown pass type for pass: %s", pass->get_name());
                break;
            }

            // End debug marker
            if (config_.enable_debug_markers)
            {
                end_debug_marker(executor);
            }

            if (config_.enable_profiler_events && profiler)
            {
                profiler->on_pass_end(*static_cast<RenderGraphBackend*>(graph), *executor, *pass);
            }

            recording_result_.total_passes_executed++;
        }
    }
}

void PassExecutionPhase::execute_render_pass(RenderGraph* graph_, RenderGraphFrameExecutor* executor_, RenderPassNode* pass) SKR_NOEXCEPT
{
    RenderGraphBackend* graph = static_cast<RenderGraphBackend*>(graph_);
    auto& executor = *executor_;
    const auto frame_index = graph->get_frame_index();

    SkrZoneScopedC(tracy::Color::LightPink);
    ZoneName(pass->name.c_str_raw(), pass->name.size());

    StackVector<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers;
    StackVector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures;

    pass->foreach_textures([&](TextureNode* texture, TextureEdge* edge) {
        resolved_textures.emplace(texture->get_handle(), resource_allocation_phase_.get_resource(texture));
    });
    pass->foreach_buffers([&](BufferNode* buffer, BufferEdge* edge) {
        resolved_buffers.emplace(buffer->get_handle(), resource_allocation_phase_.get_resource(buffer));
    });

    RenderPassContext pass_context = {};
    // allocate & update descriptor sets
    pass_context.graph = graph;
    pass_context.pass = pass;
    pass_context.bind_table = bind_table_phase_.get_pass_bind_table(pass);
    pass_context.resolved_buffers = { resolved_buffers.data(), resolved_buffers.size() };
    pass_context.resolved_textures = { resolved_textures.data(), resolved_textures.size() };
    pass_context.executor = &executor;
    {
        SkrZoneScopedN("WriteBarrierMarker");
        skr::String message = u8"Pass-";
        message.append(pass->get_name());
        message.append(u8"-BeginBarrier");
        executor.write_marker(message.c_str());
    }
    // color attachments
    StackVector<CGPUColorAttachment> color_attachments = {};
    CGPUDepthStencilAttachment ds_attachment = {};
    auto write_edges = pass->tex_write_edges();
    auto pass_sample_count = CGPU_SAMPLE_COUNT_1;
    for (auto& write_edge : write_edges)
    {
        // TODO: MSAA
        auto texture_target = write_edge->get_texture_node();
        if (write_edge->mrt_index > CGPU_MAX_MRT_COUNT) continue; // Resolve Targets

        TextureNode* resolve_target = nullptr;
        const auto sample_count = texture_target->get_desc().sample_count;
        if (sample_count != CGPU_SAMPLE_COUNT_1)
        {
            pass_sample_count = sample_count;     // TODO: REFACTOR THIS
            for (auto& write_edge2 : write_edges) // find resolve target
            {
                if (write_edge2->mrt_index == write_edge->mrt_index + 1 + CGPU_MAX_MRT_COUNT)
                {
                    if (write_edge2->get_texture_node()->get_desc().sample_count == CGPU_SAMPLE_COUNT_1)
                    {
                        resolve_target = write_edge2->get_texture_node();
                    }
                    break;
                }
            }
        }
        const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(resource_allocation_phase_.get_resource(texture_target)->info->format);
        const bool is_depth_only = FormatUtil_IsDepthOnlyFormat(resource_allocation_phase_.get_resource(texture_target)->info->format);
        if (write_edge->requested_state == CGPU_RESOURCE_STATE_DEPTH_WRITE && is_depth_stencil)
        {
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture = resource_allocation_phase_.get_resource(texture_target);
            view_desc.base_array_layer = write_edge->get_array_base();
            view_desc.array_layer_count = write_edge->get_array_count();
            view_desc.base_mip_level = write_edge->get_mip_level();
            view_desc.mip_level_count = 1;
            view_desc.aspects =
                is_depth_only ? CGPU_TEXTURE_VIEW_ASPECTS_DEPTH : CGPU_TEXTURE_VIEW_ASPECTS_DEPTH | CGPU_TEXTURE_VIEW_ASPECTS_STENCIL;
            view_desc.format = view_desc.texture->info->format;
            view_desc.view_usages = CGPU_TEXTURE_VIEW_USAGE_RTV_DSV;
            if (sample_count == CGPU_SAMPLE_COUNT_1)
            {
                view_desc.dims = CGPU_TEXTURE_DIMENSION_2D;
            }
            else
            {
                view_desc.dims = CGPU_TEXTURE_DIMENSION_2DMS;
            }
            ds_attachment.view = graph->texture_view_pool.allocate(view_desc, frame_index);
            ds_attachment.depth_load_action = pass->depth_load_action;
            ds_attachment.depth_store_action = pass->depth_store_action;
            ds_attachment.stencil_load_action = pass->stencil_load_action;
            ds_attachment.stencil_store_action = pass->stencil_store_action;
            ds_attachment.clear_depth = pass->clear_depth;
            ds_attachment.write_depth = true;
        }
        else
        {
            CGPUColorAttachment attachment = {};
            if (resolve_target) // alocate resolve view
            {
                auto msaaTexture = resource_allocation_phase_.get_resource(resolve_target);
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = msaaTexture;
                view_desc.base_array_layer = 0; // TODO: resolve MSAA to specific slice ?
                view_desc.array_layer_count = 1;
                view_desc.base_mip_level = 0;
                view_desc.mip_level_count = 1; // TODO: resolve MSAA to specific mip slice?
                view_desc.format = view_desc.texture->info->format;
                view_desc.aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
                view_desc.view_usages = CGPU_TEXTURE_VIEW_USAGE_RTV_DSV;
                view_desc.dims = CGPU_TEXTURE_DIMENSION_2D;
                attachment.resolve_view = graph->texture_view_pool.allocate(view_desc, frame_index);
            }
            // allocate target view
            {
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture = resource_allocation_phase_.get_resource(texture_target);
                view_desc.base_array_layer = write_edge->get_array_base();
                view_desc.array_layer_count = write_edge->get_array_count();
                view_desc.base_mip_level = write_edge->get_mip_level();
                view_desc.mip_level_count = 1; // TODO: mip
                view_desc.format = view_desc.texture->info->format;
                view_desc.aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
                view_desc.view_usages = CGPU_TEXTURE_VIEW_USAGE_RTV_DSV;
                if (sample_count == CGPU_SAMPLE_COUNT_1)
                {
                    view_desc.dims = CGPU_TEXTURE_DIMENSION_2D;
                }
                else
                {
                    view_desc.dims = CGPU_TEXTURE_DIMENSION_2DMS;
                }
                attachment.view = graph->texture_view_pool.allocate(view_desc, frame_index);
            }
            attachment.load_action = pass->load_actions[write_edge->mrt_index];
            attachment.store_action = pass->store_actions[write_edge->mrt_index];
            attachment.clear_color = write_edge->get_clear_value();
            color_attachments.emplace(attachment);
        }
    }
    CGPURenderPassDescriptor pass_desc = {};
    pass_desc.render_target_count = (uint32_t)color_attachments.size();
    pass_desc.sample_count = pass_sample_count;
    pass_desc.name = pass->get_name();
    pass_desc.color_attachments = color_attachments.data();
    pass_desc.depth_stencil = &ds_attachment;
    pass_context.cmd = executor.gfx_cmd_buf;
    {
        SkrZoneScopedN("WriteBeginPassMarker");
        skr::String message = u8"Pass-";
        message.append(pass->get_name());
        message.append(u8"-BeginPass");
        executor.write_marker(message.c_str());
    }
    {
        SkrZoneScopedN("BeginRenderPass");
        pass_context.encoder = cgpu_cmd_begin_render_pass(executor.gfx_cmd_buf, &pass_desc);
    }
    if (pass->pipeline)
    {
        cgpu_render_encoder_bind_pipeline(pass_context.encoder, pass->pipeline);
    }
    if (pass_context.bind_table)
    {
        cgpux_render_encoder_bind_bind_table(pass_context.encoder, pass_context.bind_table);
    }
    {
        SkrZoneScopedN("PassExecutor");
        if (pass->executor)
        {
            pass->executor(*graph, pass_context);
        }
    }
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, pass_context.encoder);
    {
        SkrZoneScopedN("WriteEndPassMarker");
        skr::String message = u8"Pass-";
        message.append(pass->get_name());
        message.append(u8"-EndRenderPass");
        executor.write_marker(message.c_str());
    }
}

void PassExecutionPhase::execute_compute_pass(RenderGraph* graph_, RenderGraphFrameExecutor* executor_, ComputePassNode* pass) SKR_NOEXCEPT
{
    RenderGraphBackend* graph = static_cast<RenderGraphBackend*>(graph_);
    auto& executor = *executor_;
    SkrZoneScopedC(tracy::Color::LightBlue);
    ZoneName(pass->name.c_str_raw(), pass->name.size());

    StackVector<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers;
    StackVector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures;

    pass->foreach_textures([&](TextureNode* texture, TextureEdge* edge) {
        resolved_textures.emplace(texture->get_handle(), resource_allocation_phase_.get_resource(texture));
    });
    pass->foreach_buffers([&](BufferNode* buffer, BufferEdge* edge) {
        resolved_buffers.emplace(buffer->get_handle(), resource_allocation_phase_.get_resource(buffer));
    });

    // allocate & update descriptor sets
    ComputePassContext pass_context = {};
    pass_context.graph = graph;
    pass_context.pass = pass;
    pass_context.bind_table = bind_table_phase_.get_pass_bind_table(pass);
    pass_context.resolved_buffers = resolved_buffers;
    pass_context.resolved_textures = resolved_textures;
    pass_context.executor = &executor;

    // dispatch
    CGPUComputePassDescriptor pass_desc = {};
    pass_desc.name = pass->get_name();
    pass_context.cmd = executor.gfx_cmd_buf;
    pass_context.encoder = cgpu_cmd_begin_compute_pass(executor.gfx_cmd_buf, &pass_desc);
    if (pass->pipeline)
    {
        cgpu_compute_encoder_bind_pipeline(pass_context.encoder, pass->pipeline);
    }
    if (pass->ray_pipeline)
    {
        cgpu_compute_encoder_bind_ray_pipeline(pass_context.encoder, pass->ray_pipeline);
    }
    if (pass_context.bind_table)
    {
        cgpux_compute_encoder_bind_bind_table(pass_context.encoder, pass_context.bind_table);
    }
    {
        SkrZoneScopedN("PassExecutor");
        if (pass->executor)
        {
            pass->executor(*graph, pass_context);
        }
    }
    cgpu_cmd_end_compute_pass(executor.gfx_cmd_buf, pass_context.encoder);
}

void PassExecutionPhase::execute_copy_pass(RenderGraph* graph_, RenderGraphFrameExecutor* executor_, CopyPassNode* pass) SKR_NOEXCEPT
{
    RenderGraphBackend* graph = static_cast<RenderGraphBackend*>(graph_);
    auto& executor = *executor_;
    SkrZoneScopedC(tracy::Color::LightYellow);
    ZoneName(pass->name.c_str_raw(), pass->name.size());
    // resource de-virtualize
    StackVector<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers;
    StackVector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures;

    pass->foreach_textures([&](TextureNode* texture, TextureEdge* edge) {
        resolved_textures.emplace(texture->get_handle(), resource_allocation_phase_.get_resource(texture));
    });
    pass->foreach_buffers([&](BufferNode* buffer, BufferEdge* edge) {
        resolved_buffers.emplace(buffer->get_handle(), resource_allocation_phase_.get_resource(buffer));
    });

    // late barriers
    stack_vector<CGPUTextureBarrier> late_tex_barriers = {};
    stack_vector<CGPUBufferBarrier> late_buf_barriers = {};
    // call cgpu apis
    CGPUResourceBarrierDescriptor late_barriers = {};
    {
        CopyPassContext stack = {};
        stack.cmd = executor.gfx_cmd_buf;
        stack.resolved_buffers = { resolved_buffers.data(), resolved_buffers.size() };
        stack.resolved_textures = { resolved_textures.data(), resolved_textures.size() };
        if (pass->executor)
        {
            pass->executor(*graph, stack);
        }
        for (auto [buffer_handle, state] : pass->bbarriers)
        {
            auto buffer = stack.resolve(buffer_handle);
            auto& barrier = late_buf_barriers.emplace().ref();
            barrier.buffer = buffer;
            barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = state;
        }
        for (auto [texture_handle, state] : pass->tbarriers)
        {
            auto texture = stack.resolve(texture_handle);
            auto& barrier = late_tex_barriers.emplace().ref();
            barrier.texture = texture;
            barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = state;
        }
        if (!late_tex_barriers.is_empty())
        {
            late_barriers.texture_barriers = late_tex_barriers.data();
            late_barriers.texture_barriers_count = (uint32_t)late_tex_barriers.size();
        }
        if (!late_buf_barriers.is_empty())
        {
            late_barriers.buffer_barriers = late_buf_barriers.data();
            late_barriers.buffer_barriers_count = (uint32_t)late_buf_barriers.size();
        }
    }
    for (uint32_t i = 0; i < pass->t2ts.size(); i++)
    {
        auto src_node = graph->resolve(pass->t2ts[i].first);
        auto dst_node = graph->resolve(pass->t2ts[i].second);
        CGPUTextureToTextureTransfer t2t = {};
        t2t.src = resource_allocation_phase_.get_resource(src_node);
        t2t.src_subresource.aspects = pass->t2ts[i].first.get_aspects();
        t2t.src_subresource.mip_level = pass->t2ts[i].first.get_mip_level();
        t2t.src_subresource.base_array_layer = pass->t2ts[i].first.get_array_base();
        t2t.src_subresource.layer_count = pass->t2ts[i].first.get_array_count();
        t2t.dst = resource_allocation_phase_.get_resource(dst_node);
        t2t.dst_subresource.aspects = pass->t2ts[i].second.get_aspects();
        t2t.dst_subresource.mip_level = pass->t2ts[i].second.get_mip_level();
        t2t.dst_subresource.base_array_layer = pass->t2ts[i].second.get_array_base();
        t2t.dst_subresource.layer_count = pass->t2ts[i].second.get_array_count();
        cgpu_cmd_transfer_texture_to_texture(executor.gfx_cmd_buf, &t2t);
    }
    for (uint32_t i = 0; i < pass->b2bs.size(); i++)
    {
        auto src_node = graph->resolve(pass->b2bs[i].first);
        auto dst_node = graph->resolve(pass->b2bs[i].second);
        CGPUBufferToBufferTransfer b2b = {};
        b2b.src = resource_allocation_phase_.get_resource(src_node);
        b2b.src_offset = pass->b2bs[i].first.from;
        b2b.dst = resource_allocation_phase_.get_resource(dst_node);
        b2b.dst_offset = pass->b2bs[i].second.from;
        b2b.size = pass->b2bs[i].first.to - b2b.src_offset;
        cgpu_cmd_transfer_buffer_to_buffer(executor.gfx_cmd_buf, &b2b);
    }
    for (uint32_t i = 0; i < pass->b2ts.size(); i++)
    {
        auto src_node = graph->resolve(pass->b2ts[i].first);
        auto dst_node = graph->resolve(pass->b2ts[i].second);
        CGPUBufferToTextureTransfer b2t = {};
        b2t.src = resource_allocation_phase_.get_resource(src_node);
        b2t.src_offset = pass->b2ts[i].first.from;
        b2t.dst = resource_allocation_phase_.get_resource(dst_node);
        b2t.dst_subresource.mip_level = pass->b2ts[i].second.get_mip_level();
        b2t.dst_subresource.base_array_layer = pass->b2ts[i].second.get_array_base();
        b2t.dst_subresource.layer_count = pass->b2ts[i].second.get_array_count();
        cgpu_cmd_transfer_buffer_to_texture(executor.gfx_cmd_buf, &b2t);
    }
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &late_barriers);
}

void PassExecutionPhase::execute_present_pass(RenderGraph* graph, RenderGraphFrameExecutor* executor, PresentPassNode* pass) SKR_NOEXCEPT
{
    // null op for now
}

void PassExecutionPhase::insert_pass_barriers(RenderGraphFrameExecutor* executor, PassNode* pass) SKR_NOEXCEPT
{
    const auto& barrier_batches = barrier_generation_phase_.get_pass_barrier_batches(pass);
    if (barrier_batches.is_empty())
        return;

    skr::InlineVector<CGPUTextureBarrier, 8> texture_barriers;
    skr::InlineVector<CGPUBufferBarrier, 8> buffer_barriers;

    const bool useRealAliasing = resource_allocation_phase_.get_aliasing_phase().get_aliasing_tier() != EAliasingTier::Tier0;
    for (const auto& batch : barrier_batches)
    {
        if (batch.barriers.is_empty()) continue;

        if (batch.batch_type == EBarrierType::ResourceTransition)
        {
            for (const auto& barrier : batch.barriers)
            {
                const auto resource_type = barrier.resource->get_type();
                if (resource_type == EObjectType::Texture)
                {
                    CGPUTextureBarrier tex_barrier = {};
                    tex_barrier.texture = resource_allocation_phase_.get_resource((TextureNode*)barrier.resource);
                    tex_barrier.src_state = barrier.transition.before_state;
                    tex_barrier.dst_state = barrier.transition.after_state;
                    tex_barrier.begin_only = barrier.transition.is_begin;
                    tex_barrier.end_only = barrier.transition.is_end;
                    
                    tex_barrier.subresource_barrier = barrier.transition.is_subresource;
                    tex_barrier.mip_level = barrier.transition.mip_level;
                    tex_barrier.array_layer = barrier.transition.array_level;

                    texture_barriers.add(tex_barrier);
                }
                else if (resource_type == EObjectType::Buffer)
                {
                    CGPUBufferBarrier buf_barrier = {};
                    buf_barrier.buffer = resource_allocation_phase_.get_resource((BufferNode*)barrier.resource);
                    buf_barrier.src_state = barrier.transition.before_state;
                    buf_barrier.dst_state = barrier.transition.after_state;
                    buf_barrier.begin_only = barrier.transition.is_begin;
                    buf_barrier.end_only = barrier.transition.is_end;
                    buffer_barriers.add(buf_barrier);
                }
            }
        }
        else if ((batch.batch_type == EBarrierType::MemoryAliasing) && !useRealAliasing)
        {
            for (const auto& barrier : batch.barriers)
            {
                const auto resource_type = barrier.resource->get_type();
                ECGPUResourceState state_from_pool = ECGPUResourceState::CGPU_RESOURCE_STATE_UNDEFINED;
                if (resource_type == EObjectType::Texture)
                {
                    CGPUTextureBarrier tex_barrier = {};
                    tex_barrier.texture = resource_allocation_phase_.get_resource((TextureNode*)barrier.resource, &state_from_pool);
                    tex_barrier.src_state = state_from_pool;
                    tex_barrier.dst_state = barrier.aliasing.after_state;
                    tex_barrier.queue_acquire = false;
                    tex_barrier.queue_release = false;
                    texture_barriers.add(tex_barrier);
                }
                else if (resource_type == EObjectType::Buffer)
                {
                    CGPUBufferBarrier buf_barrier = {};
                    buf_barrier.buffer = resource_allocation_phase_.get_resource((BufferNode*)barrier.resource, &state_from_pool);
                    buf_barrier.src_state = state_from_pool;
                    buf_barrier.dst_state = barrier.aliasing.after_state;
                    buf_barrier.queue_acquire = false;
                    buf_barrier.queue_release = false;
                    buffer_barriers.add(buf_barrier);
                }
            }
        }
        else if ((batch.batch_type == EBarrierType::MemoryAliasing) && useRealAliasing)
        {
            SKR_UNIMPLEMENTED_FUNCTION();
        }
    }

    // Insert barriers if any
    if (!texture_barriers.is_empty() || !buffer_barriers.is_empty())
    {
        CGPUResourceBarrierDescriptor barriers = {};
        if (!texture_barriers.is_empty())
        {
            barriers.texture_barriers = texture_barriers.data();
            barriers.texture_barriers_count = static_cast<uint32_t>(texture_barriers.size());
        }
        if (!buffer_barriers.is_empty())
        {
            barriers.buffer_barriers = buffer_barriers.data();
            barriers.buffer_barriers_count = static_cast<uint32_t>(buffer_barriers.size());
        }

        cgpu_cmd_resource_barrier(executor->gfx_cmd_buf, &barriers);
        recording_result_.total_barriers_inserted += static_cast<uint32_t>(texture_barriers.size() + buffer_barriers.size());
    }
}

void PassExecutionPhase::process_sync_points(RenderGraphFrameExecutor* executor, PassNode* pass) SKR_NOEXCEPT
{
    // Get sync points from CrossQueueSyncAnalysis
    const auto& ssis_result = sync_analysis_.get_ssis_result();

    // Process sync points that target this pass
    for (const auto& sync_point : ssis_result.optimized_sync_points)
    {
        if (sync_point.consumer_pass == pass)
        {
            // Insert fence/semaphore wait for cross-queue synchronization
            // TODO: Implement proper cross-queue synchronization with fences/semaphores
            // For now, we rely on barriers for synchronization

            if (config_.enable_debug_output)
            {
                COMMAND_RECORDING_LOG(u8"Processing sync point: %s -> %s (queue %u -> %u)",
                    sync_point.producer_pass ? sync_point.producer_pass->get_name() : u8"unknown",
                    sync_point.consumer_pass ? sync_point.consumer_pass->get_name() : u8"unknown",
                    sync_point.producer_queue_index,
                    sync_point.consumer_queue_index);
            }

            recording_result_.total_sync_points_processed++;
        }
    }
}

void PassExecutionPhase::begin_debug_marker(RenderGraphFrameExecutor* executor, PassNode* pass) SKR_NOEXCEPT
{
    if (!config_.enable_debug_markers) return;

    CGPUEventInfo event = {
        (const char8_t*)pass->get_name(),
        { 0.2f, 0.6f, 1.0f, 1.0f } // Blue color for passes
    };
    cgpu_cmd_begin_event(executor->gfx_cmd_buf, &event);
}

void PassExecutionPhase::end_debug_marker(RenderGraphFrameExecutor* executor) SKR_NOEXCEPT
{
    if (!config_.enable_debug_markers) return;

    cgpu_cmd_end_event(executor->gfx_cmd_buf);
}

} // namespace render_graph
} // namespace skr