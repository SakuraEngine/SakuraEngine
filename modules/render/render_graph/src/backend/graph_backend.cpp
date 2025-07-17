#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrCore/memory/sp.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/memory/memory.h"
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrContainers/string.hpp"
#include "SkrContainers/btree.hpp"
#include "SkrGraphics/cgpux.hpp"

#include "SkrRenderGraph/phases/cull_phase.hpp"

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
{
    phases.add(skr::SP<CullPhase>::New());
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

// memory aliasing:
// - lifespan不重叠的资源对象都可以考虑做aliasing处理
// - aliasing的memory-type有要求，需要检查两个对象之间的内存堆兼容性
// - 可以撮合两个对象，将它们提升为可alias的，把他们的size和alignment取最大处理即可。
//  - 首先寻找可以直接alias处理的, cgpu_try_create_aliasing_resource(ResourceId, ResourceDesc)
//  - 其次再考虑提升合并的行为（此行为在前端无法模拟）
CGPUTextureId RenderGraphBackend::try_aliasing_allocate(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT
{
    if (node.frame_aliasing_source)
    {
        SkrZoneScopedN("AllocateAliasingResource");
        // allocate & try bind
        auto                              aliasing_texture = cgpu_create_texture(device, &node.descriptor);
        CGPUTextureAliasingBindDescriptor aliasing_desc    = {};
        aliasing_desc.aliased                              = resolve(executor, *node.frame_aliasing_source);
        aliasing_desc.aliasing                             = aliasing_texture;
        node.frame_aliasing                                = cgpu_try_bind_aliasing_texture(device, &aliasing_desc);
        if (!node.frame_aliasing)
        {
            cgpu_free_texture(aliasing_texture);
            ((TextureNode&)node).descriptor.flags &= ~CGPU_TCF_ALIASING_RESOURCE;
            return nullptr;
        }
        executor.aliasing_textures.add(aliasing_texture);
        return aliasing_texture;
    }
    return nullptr;
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

CGPUTextureId RenderGraphBackend::resolve(RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResolveTexture");
    if (!node.frame_texture)
    {
        if (auto aliased = try_aliasing_allocate(executor, node); aliased)
        {
            node.frame_texture = aliased;
            node.init_state    = CGPU_RESOURCE_STATE_UNDEFINED;
        }
        else
        {
            auto allocated     = texture_pool.allocate(node.descriptor, { frame_index, node.tags });
            node.frame_texture = node.imported ? node.frame_texture : allocated.first;
            node.init_state    = allocated.second;
        }
    }
    return node.frame_texture;
}

CGPUBufferId RenderGraphBackend::resolve(RenderGraphFrameExecutor& executor, const BufferNode& node) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResolveBuffer");
    if (!node.frame_buffer)
    {
        uint64_t latest_frame = (node.tags & kRenderGraphDynamicResourceTag) ? get_latest_finished_frame() : UINT64_MAX;
        auto     allocated    = buffer_pool.allocate(node.descriptor, { frame_index, node.tags }, latest_frame);
        node.frame_buffer     = node.imported ? node.frame_buffer : allocated.first;
        node.init_state       = allocated.second;
    }
    return node.frame_buffer;
}

void RenderGraphBackend::calculate_barriers(RenderGraphFrameExecutor& executor, PassNode* pass,
                                            stack_vector<CGPUTextureBarrier>& tex_barriers, stack_vector<std::pair<TextureHandle, CGPUTextureId>>& resolved_textures,
                                            stack_vector<CGPUBufferBarrier>& buf_barriers, stack_vector<std::pair<BufferHandle, CGPUBufferId>>& resolved_buffers) SKR_NOEXCEPT
{
    stack_set<TextureHandle> tex_resolve_set;
    stack_set<BufferHandle>  buf_resolve_set;

    SkrZoneScopedN("CalculateBarriers");
    pass->foreach_textures(
    [&](TextureNode* texture, TextureEdge* edge) {
        auto tex_resolved = resolve(executor, *texture);
        if (!tex_resolve_set.contains(texture->get_handle()) )
        {
            resolved_textures.emplace(texture->get_handle(), tex_resolved);
            tex_resolve_set.add(texture->get_handle());

            const auto current_state = get_lastest_state(texture, pass);
            const auto dst_state     = edge->requested_state;
            if (current_state == dst_state) return;

            CGPUTextureBarrier barrier = {};
            barrier.src_state          = current_state;
            barrier.dst_state          = dst_state;
            barrier.texture            = tex_resolved;

            tex_barriers.emplace(barrier);
        }
    });
    pass->foreach_buffers(
    [&](BufferNode* buffer, BufferEdge* edge) {
        auto buf_resolved = resolve(executor, *buffer);
        if (!buf_resolve_set.find(buffer->get_handle()) )
        {
            resolved_buffers.emplace(buffer->get_handle(), buf_resolved);
            buf_resolve_set.add(buffer->get_handle());

            const auto current_state = get_lastest_state(buffer, pass);
            const auto dst_state     = edge->requested_state;
            if (current_state == dst_state) return;

            CGPUBufferBarrier barrier = {};
            barrier.src_state         = current_state;
            barrier.dst_state         = dst_state;
            barrier.buffer            = buf_resolved;
            buf_barriers.emplace(barrier);
        }
    });
}

const CGPUShaderResource* find_shader_resource(uint64_t name_hash, CGPURootSignatureId root_sig, ECGPUResourceType* type = nullptr)
{
    for (uint32_t i = 0; i < root_sig->table_count; i++)
    {
        for (uint32_t j = 0; j < root_sig->tables[i].resources_count; j++)
        {
            const auto& resource = root_sig->tables[i].resources[j];
            if (resource.name_hash == name_hash)
            {
                if (type) *type = resource.type;
                return &root_sig->tables[i].resources[j];
            }
        }
    }
    return nullptr;
}

CGPUXBindTableId RenderGraphBackend::alloc_update_pass_bind_table(RenderGraphFrameExecutor& executor, PassNode* pass, CGPURootSignatureId root_sig) SKR_NOEXCEPT
{
    if (!root_sig) return nullptr;

    SkrZoneScopedN("UpdateBindings");
    auto tex_read_edges = pass->tex_read_edges();
    auto tex_rw_edges   = pass->tex_readwrite_edges();
    auto buf_read_edges = pass->buf_read_edges();
    auto buf_rw_edges   = pass->buf_readwrite_edges();
    (void)buf_rw_edges;

    // Allocate or get descriptor set heap
    auto&& table_pool_iter = executor.bind_table_pools.find(root_sig);
    if (table_pool_iter == executor.bind_table_pools.end())
    {
        executor.bind_table_pools.emplace(root_sig, SkrNew<BindTablePool>(root_sig));
    }
    skr::String bind_table_keys = u8"";
    // Bind resources
    stack_vector<CGPUDescriptorData> desc_set_updates;
    stack_vector<const char8_t*>     bindTableValueNames = {};
    // CBV Buffers
    stack_vector<CGPUBufferId>      cbvs(buf_read_edges.size());
    stack_vector<CGPUTextureViewId> srvs(tex_read_edges.size());
    stack_vector<CGPUTextureViewId> uavs(tex_rw_edges.size());
    {
        for (uint32_t e_idx = 0; e_idx < buf_read_edges.size(); e_idx++)
        {
            auto& read_edge = buf_read_edges[e_idx];
            SKR_ASSERT(!read_edge->name.is_empty());
            // TODO: refactor this
            const auto& resource = *find_shader_resource(read_edge->name_hash, root_sig);

            ECGPUResourceType resource_type = resource.type;
            {
                bind_table_keys.append(read_edge->name.is_empty() ? resource.name : (const char8_t*)read_edge->name.c_str());
                bind_table_keys.append(u8";");
                bindTableValueNames.emplace(resource.name);

                auto               buffer_readed = read_edge->get_buffer_node();
                CGPUDescriptorData update        = {};
                update.count                     = 1;
                update.name                      = resource.name;
                update.binding_type              = resource_type;
                update.binding                   = resource.binding;
                cbvs[e_idx]                      = resolve(executor, *buffer_readed);
                update.buffers                   = &cbvs[e_idx];
                desc_set_updates.emplace(update);
            }
        }
        // SRVs
        for (uint32_t e_idx = 0; e_idx < tex_read_edges.size(); e_idx++)
        {
            auto& read_edge = tex_read_edges[e_idx];
            SKR_ASSERT(!read_edge->name.is_empty());
            const auto& resource = *find_shader_resource(read_edge->name_hash, root_sig);

            {
                bind_table_keys.append(read_edge->name.is_empty() ? resource.name : (const char8_t*)read_edge->name.c_str());
                bind_table_keys.append(u8";");
                bindTableValueNames.emplace(resource.name);

                auto               texture_readed   = read_edge->get_texture_node();
                CGPUDescriptorData update           = {};
                update.count                        = 1;
                update.name                         = resource.name;
                update.binding_type                 = CGPU_RESOURCE_TYPE_TEXTURE;
                update.binding                      = resource.binding;
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture                   = resolve(executor, *texture_readed);
                view_desc.base_array_layer          = read_edge->get_array_base();
                view_desc.array_layer_count         = read_edge->get_array_count();
                view_desc.base_mip_level            = read_edge->get_mip_base();
                view_desc.mip_level_count           = read_edge->get_mip_count();
                view_desc.format                    = view_desc.texture->info->format;
                const bool is_depth_stencil         = FormatUtil_IsDepthStencilFormat(view_desc.format);
                const bool is_depth_only            = FormatUtil_IsDepthStencilFormat(view_desc.format);
                view_desc.aspects =
                is_depth_stencil ?
                is_depth_only ? CGPU_TVA_DEPTH : CGPU_TVA_DEPTH | CGPU_TVA_STENCIL :
                CGPU_TVA_COLOR;
                view_desc.usages = CGPU_TVU_SRV;
                view_desc.dims   = read_edge->get_dimension();
                srvs[e_idx]      = texture_view_pool.allocate(view_desc, frame_index);
                update.textures  = &srvs[e_idx];
                desc_set_updates.emplace(update);
            }
        }
        // UAVs
        for (uint32_t e_idx = 0; e_idx < tex_rw_edges.size(); e_idx++)
        {
            auto& rw_edge = tex_rw_edges[e_idx];
            SKR_ASSERT(!rw_edge->name.is_empty());
            const auto& resource = *find_shader_resource(rw_edge->name_hash, root_sig);

            {
                bind_table_keys.append(rw_edge->name.is_empty() ? resource.name : (const char8_t*)rw_edge->name.c_str());
                bind_table_keys.append(u8";");
                bindTableValueNames.emplace(resource.name);

                auto               texture_readwrite = rw_edge->get_texture_node();
                CGPUDescriptorData update            = {};
                update.count                         = 1;
                update.name                          = resource.name;
                update.binding_type                  = CGPU_RESOURCE_TYPE_RW_TEXTURE;
                update.binding                       = resource.binding;
                CGPUTextureViewDescriptor view_desc  = {};
                view_desc.texture                    = resolve(executor, *texture_readwrite);
                view_desc.base_array_layer           = 0;
                view_desc.array_layer_count          = 1;
                view_desc.base_mip_level             = 0;
                view_desc.mip_level_count            = 1;
                view_desc.aspects                    = CGPU_TVA_COLOR;
                view_desc.format                     = view_desc.texture->info->format;
                view_desc.usages                     = CGPU_TVU_UAV;
                view_desc.dims                       = CGPU_TEX_DIMENSION_2D;
                uavs[e_idx]                          = texture_view_pool.allocate(view_desc, frame_index);
                update.textures                      = &uavs[e_idx];
                desc_set_updates.emplace(update);
            }
        }
    }
    auto bind_table = executor.bind_table_pools[root_sig]->pop(bind_table_keys.c_str(), bindTableValueNames.data(), (uint32_t)bindTableValueNames.size());
    cgpux_bind_table_update(bind_table, desc_set_updates.data(), (uint32_t)desc_set_updates.size());
    return bind_table;
}

void RenderGraphBackend::deallocate_resources(PassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedN("VirtualDeallocate");
    pass->foreach_textures([this, pass](TextureNode* texture, TextureEdge* edge) {
        if (texture->imported) return;
        bool is_last_user = true;
        auto try_discard = [&](DependencyGraphNode* neig) {
            RenderGraphNode* rg_node = (RenderGraphNode*)neig;
            if (rg_node->type == EObjectType::Pass)
            {
                PassNode* other_pass = (PassNode*)rg_node;
                is_last_user         = is_last_user && (pass->order >= other_pass->order);
            }
        };
        texture->foreach_neighbors(try_discard);
        texture->foreach_inv_neighbors(try_discard);
        if (is_last_user)
        {
            if (!texture->frame_aliasing)
            {
                SkrZoneScopedN("VirtualDeallocate::TextureFromPool");

                texture_pool.deallocate(texture->descriptor, texture->frame_texture,
                                        edge->requested_state, { frame_index, texture->tags });
            }
        }
    });
    pass->foreach_buffers([this, pass](BufferNode* buffer, BufferEdge* edge) {
        if (buffer->imported) return;
        bool is_last_user = true;
        auto try_discard = [&](DependencyGraphNode* neig) {
            RenderGraphNode* rg_node = (RenderGraphNode*)neig;
            if (rg_node->type == EObjectType::Pass)
            {
                PassNode* other_pass = (PassNode*)rg_node;
                is_last_user         = is_last_user && (pass->order >= other_pass->order);
            }
        };
        buffer->foreach_neighbors(try_discard);
        buffer->foreach_inv_neighbors(try_discard);
        if (is_last_user)
        {
            SkrZoneScopedN("VirtualDeallocate::BufferFromPool");

            buffer_pool.deallocate(buffer->descriptor, buffer->frame_buffer,
                                   edge->requested_state, { frame_index, buffer->tags });
        }
    });
}

void RenderGraphBackend::execute_compute_pass(RenderGraphFrameExecutor& executor, ComputePassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedC(tracy::Color::LightBlue);
    ZoneName(pass->name.c_str_raw(), pass->name.size());

    ComputePassContext pass_context = {};
    // resource de-virtualize
    stack_vector<CGPUTextureBarrier>                        tex_barriers      = {};
    stack_vector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
    stack_vector<CGPUBufferBarrier>                         buffer_barriers   = {};
    stack_vector<std::pair<BufferHandle, CGPUBufferId>>   resolved_buffers  = {};
    calculate_barriers(executor, pass,
                       tex_barriers, resolved_textures,
                       buffer_barriers, resolved_buffers);
    // allocate & update descriptor sets
    pass_context.graph             = this;
    pass_context.pass              = pass;
    pass_context.bind_table        = alloc_update_pass_bind_table(executor, pass, pass->root_signature);
    pass_context.resolved_buffers  = resolved_buffers;
    pass_context.resolved_textures = resolved_textures;
    pass_context.executor          = &executor;
    // call cgpu apis
    CGPUResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.is_empty())
    {
        barriers.texture_barriers       = tex_barriers.data();
        barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    if (!buffer_barriers.is_empty())
    {
        barriers.buffer_barriers       = buffer_barriers.data();
        barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
    }
    CGPUEventInfo event = { (const char8_t*)pass->name.c_str(), { 1.f, 1.f, 0.f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    // dispatch
    CGPUComputePassDescriptor pass_desc = {};
    pass_desc.name                      = pass->get_name();
    pass_context.cmd                    = executor.gfx_cmd_buf;
    pass_context.encoder                = cgpu_cmd_begin_compute_pass(executor.gfx_cmd_buf, &pass_desc);
    if (pass->pipeline)
    {
        cgpu_compute_encoder_bind_pipeline(pass_context.encoder, pass->pipeline);
    }
    cgpux_compute_encoder_bind_bind_table(pass_context.encoder, pass_context.bind_table);
    {
        SkrZoneScopedN("PassExecutor");
        pass->executor(*this, pass_context);
    }
    cgpu_cmd_end_compute_pass(executor.gfx_cmd_buf, pass_context.encoder);
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_render_pass(RenderGraphFrameExecutor& executor, RenderPassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedC(tracy::Color::LightPink);
    ZoneName(pass->name.c_str_raw(), pass->name.size());

    RenderPassContext pass_context = {};
    // resource de-virtualize
    stack_vector<CGPUTextureBarrier>                        tex_barriers      = {};
    stack_vector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
    stack_vector<CGPUBufferBarrier>                         buffer_barriers   = {};
    stack_vector<std::pair<BufferHandle, CGPUBufferId>>   resolved_buffers  = {};
    calculate_barriers(executor, pass,
                       tex_barriers, resolved_textures,
                       buffer_barriers, resolved_buffers);
    // allocate & update descriptor sets
    pass_context.graph             = this;
    pass_context.pass              = pass;
    pass_context.bind_table        = alloc_update_pass_bind_table(executor, pass, pass->root_signature);
    pass_context.resolved_buffers  = { resolved_buffers.data(), resolved_buffers.size() };
    pass_context.resolved_textures = { resolved_textures.data(), resolved_textures.size() };
    pass_context.executor          = &executor;
    // call cgpu apis
    CGPUResourceBarrierDescriptor barriers = {};
    if (!tex_barriers.is_empty())
    {
        barriers.texture_barriers       = tex_barriers.data();
        barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    if (!buffer_barriers.is_empty())
    {
        barriers.buffer_barriers       = buffer_barriers.data();
        barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
    }
    CGPUEventInfo event = { (const char8_t*)pass->name.c_str(), { 1.f, 0.5f, 0.5f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    {
        SkrZoneScopedN("WriteBarrierMarker");
        graph_big_object_string message = u8"Pass-";
        message.append(pass->get_name());
        message.append(u8"-BeginBarrier");
        executor.write_marker(message.c_str());
    }
    // color attachments
    stack_vector<CGPUColorAttachment> color_attachments = {};
    CGPUDepthStencilAttachment        ds_attachment     = {};
    auto                              write_edges       = pass->tex_write_edges();
    auto                              pass_sample_count = CGPU_SAMPLE_COUNT_1;
    for (auto& write_edge : write_edges)
    {
        // TODO: MSAA
        auto texture_target = write_edge->get_texture_node();
        if (write_edge->mrt_index > CGPU_MAX_MRT_COUNT) continue; // Resolve Targets

        TextureNode* resolve_target = nullptr;
        const auto   sample_count   = texture_target->descriptor.sample_count;
        if (sample_count != CGPU_SAMPLE_COUNT_1)
        {
            pass_sample_count = sample_count;     // TODO: REFACTOR THIS
            for (auto& write_edge2 : write_edges) // find resolve target
            {
                if (write_edge2->mrt_index == write_edge->mrt_index + 1 + CGPU_MAX_MRT_COUNT)
                {
                    if (write_edge2->get_texture_node()->descriptor.sample_count == CGPU_SAMPLE_COUNT_1)
                    {
                        resolve_target = write_edge2->get_texture_node();
                    }
                    break;
                }
            }
        }
        const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(
        resolve(executor, *texture_target)->info->format);
        const bool is_depth_only = FormatUtil_IsDepthOnlyFormat(
        resolve(executor, *texture_target)->info->format);
        if (write_edge->requested_state == CGPU_RESOURCE_STATE_DEPTH_WRITE && is_depth_stencil)
        {
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture                   = resolve(executor, *texture_target);
            view_desc.base_array_layer          = write_edge->get_array_base();
            view_desc.array_layer_count         = write_edge->get_array_count();
            view_desc.base_mip_level            = write_edge->get_mip_level();
            view_desc.mip_level_count           = 1;
            view_desc.aspects =
            is_depth_only ? CGPU_TVA_DEPTH : CGPU_TVA_DEPTH | CGPU_TVA_STENCIL;
            view_desc.format = view_desc.texture->info->format;
            view_desc.usages = CGPU_TVU_RTV_DSV;
            if (sample_count == CGPU_SAMPLE_COUNT_1)
            {
                view_desc.dims = CGPU_TEX_DIMENSION_2D;
            }
            else
            {
                view_desc.dims = CGPU_TEX_DIMENSION_2DMS;
            }
            ds_attachment.view                 = texture_view_pool.allocate(view_desc, frame_index);
            ds_attachment.depth_load_action    = pass->depth_load_action;
            ds_attachment.depth_store_action   = pass->depth_store_action;
            ds_attachment.stencil_load_action  = pass->stencil_load_action;
            ds_attachment.stencil_store_action = pass->stencil_store_action;
            ds_attachment.clear_depth          = pass->clear_depth;
            ds_attachment.write_depth          = true;
        }
        else
        {
            CGPUColorAttachment attachment = {};
            if (resolve_target) // alocate resolve view
            {
                auto                      msaaTexture = resolve(executor, *resolve_target);
                CGPUTextureViewDescriptor view_desc   = {};
                view_desc.texture                     = msaaTexture;
                view_desc.base_array_layer            = 0; // TODO: resolve MSAA to specific slice ?
                view_desc.array_layer_count           = 1;
                view_desc.base_mip_level              = 0;
                view_desc.mip_level_count             = 1; // TODO: resolve MSAA to specific mip slice?
                view_desc.format                      = view_desc.texture->info->format;
                view_desc.aspects                     = CGPU_TVA_COLOR;
                view_desc.usages                      = CGPU_TVU_RTV_DSV;
                view_desc.dims                        = CGPU_TEX_DIMENSION_2D;
                attachment.resolve_view               = texture_view_pool.allocate(view_desc, frame_index);
            }
            // allocate target view
            {
                CGPUTextureViewDescriptor view_desc = {};
                view_desc.texture                   = resolve(executor, *texture_target);
                view_desc.base_array_layer          = write_edge->get_array_base();
                view_desc.array_layer_count         = write_edge->get_array_count();
                view_desc.base_mip_level            = write_edge->get_mip_level();
                view_desc.mip_level_count           = 1; // TODO: mip
                view_desc.format                    = view_desc.texture->info->format;
                view_desc.aspects                   = CGPU_TVA_COLOR;
                view_desc.usages                    = CGPU_TVU_RTV_DSV;
                if (sample_count == CGPU_SAMPLE_COUNT_1)
                {
                    view_desc.dims = CGPU_TEX_DIMENSION_2D;
                }
                else
                {
                    view_desc.dims = CGPU_TEX_DIMENSION_2DMS;
                }
                attachment.view = texture_view_pool.allocate(view_desc, frame_index);
            }
            attachment.load_action  = pass->load_actions[write_edge->mrt_index];
            attachment.store_action = pass->store_actions[write_edge->mrt_index];
            attachment.clear_color  = write_edge->clear_value;
            color_attachments.emplace(attachment);
        }
    }
    CGPURenderPassDescriptor pass_desc = {};
    pass_desc.render_target_count      = (uint32_t)color_attachments.size();
    pass_desc.sample_count             = pass_sample_count;
    pass_desc.name                     = pass->get_name();
    pass_desc.color_attachments        = color_attachments.data();
    pass_desc.depth_stencil            = &ds_attachment;
    pass_context.cmd                   = executor.gfx_cmd_buf;
    {
        SkrZoneScopedN("WriteBeginPassMarker");
        graph_big_object_string message = u8"Pass-";
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
        pass->executor(*this, pass_context);
    }
    cgpu_cmd_end_render_pass(executor.gfx_cmd_buf, pass_context.encoder);
    {
        SkrZoneScopedN("WriteEndPassMarker");
        graph_big_object_string message = u8"Pass-";
        message.append(pass->get_name());
        message.append(u8"-EndRenderPass");
        executor.write_marker(message.c_str());
    }
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    // deallocate
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_copy_pass(RenderGraphFrameExecutor& executor, CopyPassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedC(tracy::Color::LightYellow);
    ZoneName(pass->name.c_str_raw(), pass->name.size());
    // resource de-virtualize
    stack_vector<CGPUTextureBarrier>                        tex_barriers      = {};
    stack_vector<std::pair<TextureHandle, CGPUTextureId>> resolved_textures = {};
    stack_vector<CGPUBufferBarrier>                         buffer_barriers   = {};
    stack_vector<std::pair<BufferHandle, CGPUBufferId>>   resolved_buffers  = {};
    calculate_barriers(executor, pass,
                       tex_barriers, resolved_textures,
                       buffer_barriers, resolved_buffers);
    // late barriers
    stack_vector<CGPUTextureBarrier> late_tex_barriers = {};
    stack_vector<CGPUBufferBarrier>  late_buf_barriers = {};
    // call cgpu apis
    CGPUResourceBarrierDescriptor barriers      = {};
    CGPUResourceBarrierDescriptor late_barriers = {};
    if (!tex_barriers.is_empty())
    {
        barriers.texture_barriers       = tex_barriers.data();
        barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    if (!buffer_barriers.is_empty())
    {
        barriers.buffer_barriers       = buffer_barriers.data();
        barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
    }
    CGPUEventInfo event = { (const char8_t*)pass->name.c_str(), { 0.f, .5f, 1.f, 1.f } };
    cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
    {
        CopyPassContext stack   = {};
        stack.cmd               = executor.gfx_cmd_buf;
        stack.resolved_buffers  = { resolved_buffers.data(), resolved_buffers.size() };
        stack.resolved_textures = { resolved_textures.data(), resolved_textures.size() };
        pass->executor(*this, stack);
        for (auto [buffer_handle, state] : pass->bbarriers)
        {
            auto  buffer      = stack.resolve(buffer_handle);
            auto& barrier     = late_buf_barriers.emplace().ref();
            barrier.buffer    = buffer;
            barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = state;
        }
        for (auto [texture_handle, state] : pass->tbarriers)
        {
            auto  texture     = stack.resolve(texture_handle);
            auto& barrier     = late_tex_barriers.emplace().ref();
            barrier.texture   = texture;
            barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = state;
        }
        if (!late_tex_barriers.is_empty())
        {
            late_barriers.texture_barriers       = late_tex_barriers.data();
            late_barriers.texture_barriers_count = (uint32_t)late_tex_barriers.size();
        }
        if (!late_buf_barriers.is_empty())
        {
            late_barriers.buffer_barriers       = late_buf_barriers.data();
            late_barriers.buffer_barriers_count = (uint32_t)late_buf_barriers.size();
        }
    }
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    for (uint32_t i = 0; i < pass->t2ts.size(); i++)
    {
        auto                         src_node = RenderGraph::resolve(pass->t2ts[i].first);
        auto                         dst_node = RenderGraph::resolve(pass->t2ts[i].second);
        CGPUTextureToTextureTransfer t2t      = {};
        t2t.src                               = resolve(executor, *src_node);
        t2t.src_subresource.aspects           = pass->t2ts[i].first.aspects;
        t2t.src_subresource.mip_level         = pass->t2ts[i].first.mip_level;
        t2t.src_subresource.base_array_layer  = pass->t2ts[i].first.array_base;
        t2t.src_subresource.layer_count       = pass->t2ts[i].first.array_count;
        t2t.dst                               = resolve(executor, *dst_node);
        t2t.dst_subresource.aspects           = pass->t2ts[i].second.aspects;
        t2t.dst_subresource.mip_level         = pass->t2ts[i].second.mip_level;
        t2t.dst_subresource.base_array_layer  = pass->t2ts[i].second.array_base;
        t2t.dst_subresource.layer_count       = pass->t2ts[i].second.array_count;
        cgpu_cmd_transfer_texture_to_texture(executor.gfx_cmd_buf, &t2t);
    }
    for (uint32_t i = 0; i < pass->b2bs.size(); i++)
    {
        auto                       src_node = RenderGraph::resolve(pass->b2bs[i].first);
        auto                       dst_node = RenderGraph::resolve(pass->b2bs[i].second);
        CGPUBufferToBufferTransfer b2b      = {};
        b2b.src                             = resolve(executor, *src_node);
        b2b.src_offset                      = pass->b2bs[i].first.from;
        b2b.dst                             = resolve(executor, *dst_node);
        b2b.dst_offset                      = pass->b2bs[i].second.from;
        b2b.size                            = pass->b2bs[i].first.to - b2b.src_offset;
        cgpu_cmd_transfer_buffer_to_buffer(executor.gfx_cmd_buf, &b2b);
    }
    for (uint32_t i = 0; i < pass->b2ts.size(); i++)
    {
        auto                        src_node = RenderGraph::resolve(pass->b2ts[i].first);
        auto                        dst_node = RenderGraph::resolve(pass->b2ts[i].second);
        CGPUBufferToTextureTransfer b2t      = {};
        b2t.src                              = resolve(executor, *src_node);
        b2t.src_offset                       = pass->b2ts[i].first.from;
        b2t.dst                              = resolve(executor, *dst_node);
        b2t.dst_subresource.mip_level        = pass->b2ts[i].second.mip_level;
        b2t.dst_subresource.base_array_layer = pass->b2ts[i].second.array_base;
        b2t.dst_subresource.layer_count      = pass->b2ts[i].second.array_count;
        cgpu_cmd_transfer_buffer_to_texture(executor.gfx_cmd_buf, &b2t);
    }
    cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &late_barriers);
    cgpu_cmd_end_event(executor.gfx_cmd_buf);
    deallocate_resources(pass);
}

void RenderGraphBackend::execute_present_pass(RenderGraphFrameExecutor& executor, PresentPassNode* pass) SKR_NOEXCEPT
{
    auto               read_edges          = pass->tex_read_edges();
    auto&&             read_edge           = read_edges[0];
    auto               texture_target      = read_edge->get_texture_node();
    auto               back_buffer         = pass->descriptor.swapchain->back_buffers[pass->descriptor.index];
    CGPUTextureBarrier present_barrier     = {};
    present_barrier.texture                = back_buffer;
    present_barrier.src_state              = get_lastest_state(texture_target, pass);
    present_barrier.dst_state              = CGPU_RESOURCE_STATE_PRESENT;
    CGPUResourceBarrierDescriptor barriers = {};
    barriers.texture_barriers              = &present_barrier;
    barriers.texture_barriers_count        = 1;
    // 2023-9-21: 暂时修复 GUI 报错
    if (present_barrier.src_state != present_barrier.dst_state)
    {
        cgpu_cmd_resource_barrier(executor.gfx_cmd_buf, &barriers);
    }
}

uint64_t RenderGraphBackend::execute(RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    for (auto& phase : phases)
        phase->on_execute(this, profiler);

    const auto                executor_index = frame_index % RG_MAX_FRAME_IN_FLIGHT;
    RenderGraphFrameExecutor& executor       = executors[executor_index];
    if (device->is_lost)
    {
        for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
        {
            executors[i].print_error_trace(frame_index);
        }
        SKR_BREAK();
    }
    {
        SkrZoneScopedN("AcquireExecutor");
        cgpu_wait_fences(&executor.exec_fence, 1);
        if (profiler) profiler->on_acquire_executor(*this, executor);
    }
    {
        SkrZoneScopedN("GraphExecutePasses");
        executor.reset_begin(texture_view_pool);
        if (profiler) profiler->on_cmd_begin(*this, executor);
        {
            SkrZoneScopedN("GraphExecutorBeginEvent");

            skr::String   frameLabel = skr::format(u8"Frame-{}", frame_index);
            CGPUEventInfo event      = { (const char8_t*)frameLabel.c_str(), { 0.8f, 0.8f, 0.8f, 1.f } };
            cgpu_cmd_begin_event(executor.gfx_cmd_buf, &event);
        }
        for (auto& pass : passes)
        {
            if (pass->pass_type == EPassType::Render)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_render_pass(executor, static_cast<RenderPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Present)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_present_pass(executor, static_cast<PresentPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Compute)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_compute_pass(executor, static_cast<ComputePassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
            else if (pass->pass_type == EPassType::Copy)
            {
                if (profiler) profiler->on_pass_begin(*this, executor, *pass);
                execute_copy_pass(executor, static_cast<CopyPassNode*>(pass));
                if (profiler) profiler->on_pass_end(*this, executor, *pass);
            }
        }
        {
            cgpu_cmd_end_event(executor.gfx_cmd_buf);
        }
        if (profiler) profiler->on_cmd_end(*this, executor);
        cgpu_cmd_end(executor.gfx_cmd_buf);
    }
    {
        // submit
        SkrZoneScopedN("GraphQueueSubmit");
        if (profiler) profiler->before_commit(*this, executor);
        {
            SkrZoneScopedN("CGPUGfxQueueSubmit");
            executor.commit(gfx_queue, frame_index);
        }
        if (profiler) profiler->after_commit(*this, executor);
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
    }
    return frame_index++;
}

inline static bool aliasing_capacity(TextureNode* aliased, TextureNode* aliasing) SKR_NOEXCEPT
{
    return !aliased->is_imported() &&
           !aliased->get_desc().is_restrict_dedicated &&
           aliased->get_size() >= aliasing->get_size() &&
           aliased->get_sample_count() == aliasing->get_sample_count();
}

bool RenderGraphBackend::compile() SKR_NOEXCEPT
{
    RenderGraph::compile();

    for (auto& phase : phases)
        phase->on_compile(this);

    SkrZoneScopedN("RenderGraphCompile");
    if (aliasing_enabled)
    {
        SkrZoneScopedN("CalculateAliasing");
        // 2.calc aliasing
        // - 先在aliasing chain里找一圈，如果有不重合的，直接把它加入到aliasing chain里
        // - 如果没找到，在所有resource中找一个合适的加入到aliasing chain
        skr::BTreeMap<TextureNode*, TextureNode::LifeSpan> alliasing_lifespans;
        foreach_textures([&](TextureNode* texture) SKR_NOEXCEPT {
            if (texture->imported) return;
            for (auto&& [aliased, aliaed_span] : alliasing_lifespans)
            {
                if (aliasing_capacity(aliased, texture) &&
                    aliaed_span.to < texture->lifespan().from)
                {
                    if (!texture->frame_aliasing_source ||
                        texture->frame_aliasing_source->get_size() > aliased->get_size() // always choose smallest block
                    )
                    {
                        texture->descriptor.flags |= CGPU_TCF_ALIASING_RESOURCE;
                        texture->frame_aliasing_source = aliased;
                        aliaed_span.to                 = texture->lifespan().to;
                    }
                }
            }
            if (texture->frame_aliasing_source) return;
            foreach_textures([&](TextureNode* aliased) SKR_NOEXCEPT {
                if (texture == aliased) return;

                const auto lifespan         = texture->lifespan();
                const auto aliased_lifespan = aliased->lifespan();
                if (aliasing_capacity(aliased, texture) && aliased_lifespan.to < lifespan.from)
                {
                    if (!texture->frame_aliasing_source ||
                        texture->frame_aliasing_source->get_size() > aliased->get_size() // always choose smallest block
                    )
                    {
                        texture->descriptor.flags |= CGPU_TCF_ALIASING_RESOURCE;
                        texture->frame_aliasing_source    = aliased;
                        alliasing_lifespans[aliased].from = aliased->lifespan().from;
                        alliasing_lifespans[aliased].to   = aliased->lifespan().from;
                    }
                }
            });
        });
    }
    return true;
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