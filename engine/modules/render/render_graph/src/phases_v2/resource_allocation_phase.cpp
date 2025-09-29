#include "SkrRenderGraph/phases_v2/resource_allocation_phase.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrCore/log.hpp"
#include "SkrProfile/profile.h"

namespace skr::RG
{

ResourceAllocationPhase::ResourceAllocationPhase(
    const MemoryAliasingPhase& aliasing_phase,
    const PassInfoAnalysis& pass_info_analysis,
    const ResourceAllocationConfig& config)
    : config_(config)
    , aliasing_phase_(aliasing_phase)
    , pass_info_analysis_(pass_info_analysis)
{
}

ResourceAllocationPhase::~ResourceAllocationPhase()
{
    release_resources_to_pool(graph->get_texture_pool(), graph->get_buffer_pool());
}

void ResourceAllocationPhase::on_execute(RenderGraph* graph_, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResourceAllocationPhase");
    graph = static_cast<RenderGraphBackend*>(graph_);

    allocate_pooled_resources(graph);

    if (config_.enable_debug_output)
    {
        SKR_LOG_INFO(u8"ResourceAllocationPhase: Created %u textures and %u buffers, total memory: %.2f MB",
            allocation_result_.total_textures_created,
            allocation_result_.total_buffers_created,
            allocation_result_.total_allocated_memory / (1024.0f * 1024.0f));
    }
}

void ResourceAllocationPhase::allocate_pooled_resources(RenderGraph* graph_) SKR_NOEXCEPT
{
    auto graph = static_cast<RenderGraphBackend*>(graph_);
    const auto& aliasing_result = aliasing_phase_.get_result();

    // 为每个池化资源创建实际的GPU资源
    for (size_t i = 0; i < aliasing_result.memory_buckets.size(); ++i)
    {
        ResourceNode* resource = aliasing_result.memory_buckets[i].aliased_resources[0];
        if (!resource) continue;

        // 根据资源类型创建对应的GPU资源
        if (resource->type == EObjectType::Texture)
        {
            TextureNode* texture = static_cast<TextureNode*>(resource);
            create_texture_from_pool(graph->get_texture_pool(), i, texture);
        }
        else if (resource->type == EObjectType::Buffer)
        {
            BufferNode* buffer = static_cast<BufferNode*>(resource);
            create_buffer_from_pool(graph->get_buffer_pool(), i, buffer);
        }
    }
}

void ResourceAllocationPhase::create_texture_from_pool(TexturePool& tpool, uint64_t bucket_id, TextureNode* texture) SKR_NOEXCEPT
{
    // 从纹理池获取GPU纹理
    // uint64_t latest_frame = (texture->get_tags() & kRenderGraphDynamicResourceTag) ?  graph->get_latest_finished_frame() : UINT64_MAX;
    auto [gpu_texture, init_state] = tpool.allocate(
        texture->get_desc(), { graph->get_frame_index(), texture->get_tags() });
    if (!gpu_texture)
    {
        SKR_LOG_ERROR(u8"ResourceAllocationPhase: Failed to acquire texture from pool for %s", texture->get_name());
        return;
    }

    // 存储映射关系
    allocation_result_.bucket_id_to_textures.add(bucket_id, { gpu_texture, init_state });

    // 更新统计信息
    allocation_result_.total_textures_created++;
    allocation_result_.total_allocated_memory += texture->get_size();

    if (config_.enable_debug_output)
    {
        auto desc = texture->get_desc();
        SKR_LOG_TRACE(u8"Allocated texture %s at pool (size: %u x %u, format: %d)",
            texture->get_name(),
            desc.width,
            desc.height,
            desc.format);
    }
}

void ResourceAllocationPhase::create_buffer_from_pool(BufferPool& bpool, uint64_t bucket_id, BufferNode* buffer) SKR_NOEXCEPT
{
    // 从缓冲区池获取GPU缓冲区
    uint64_t latest_frame = (buffer->get_tags() & kRenderGraphDynamicResourceTag) ? graph->get_latest_finished_frame() : UINT64_MAX;
    auto [gpu_buffer, init_state] = bpool.allocate(
        buffer->get_desc(), { graph->get_frame_index(), buffer->get_tags() }, latest_frame);

    if (!gpu_buffer)
    {
        SKR_LOG_ERROR(u8"ResourceAllocationPhase: Failed to acquire buffer from pool for %s", buffer->get_name());
        return;
    }

    // 存储映射关系
    allocation_result_.bucket_id_to_buffers.add(bucket_id, { gpu_buffer, init_state });

    // 更新统计信息
    allocation_result_.total_buffers_created++;
    allocation_result_.total_allocated_memory += buffer->get_desc().size;

    if (config_.enable_debug_output)
    {
        auto desc = buffer->get_desc();
        SKR_LOG_TRACE(u8"Allocated buffer %s at pool (size: %llu bytes)",
            buffer->get_name(),
            desc.size
        );
    }
}

void ResourceAllocationPhase::release_resources_to_pool(TexturePool& tpool, BufferPool& bpool) SKR_NOEXCEPT
{
    for (const auto& [bucket_id, gpu_texture] : allocation_result_.bucket_id_to_textures)
    {
        const auto& bucket = aliasing_phase_.get_result().memory_buckets[bucket_id];
        auto node = (TextureNode*)bucket.aliased_resources.at_last();
        auto lifetime = aliasing_phase_.get_lifetime_analysis().get_resource_lifetime(node);
        auto last_state = lifetime->last_using_state;
        tpool.deallocate(node->get_desc(), gpu_texture.v, last_state, { graph->get_frame_index(), node->get_tags() });
    }

    for (const auto& [bucket_id, gpu_buffer] : allocation_result_.bucket_id_to_buffers)
    {
        const auto& bucket = aliasing_phase_.get_result().memory_buckets[bucket_id];
        auto node = (BufferNode*)bucket.aliased_resources.at_last();
        auto lifetime = aliasing_phase_.get_lifetime_analysis().get_resource_lifetime(node);
        auto last_state = lifetime->last_using_state;
        bpool.deallocate(node->get_desc(), gpu_buffer.v, last_state, { graph->get_frame_index(), node->get_tags() });
    }

    if (config_.enable_debug_output)
    {
        SKR_LOG_TRACE(u8"ResourceAllocationPhase: Released all resources from previous frame to pool");
    }
}

CGPUTextureId ResourceAllocationPhase::get_resource(TextureNode* texture, ECGPUResourceState* pOutState) const
{
    const auto& aliasing_result = aliasing_phase_.get_result();
    if (auto redirect_it = aliasing_result.resource_to_bucket.find(texture))
    {
        const auto bucket_id = redirect_it.value();
        auto it = allocation_result_.bucket_id_to_textures.find(bucket_id);
        if (pOutState)
            *pOutState = it.value().init_state;
        return it.value().v;
    }
    return texture->get_imported();
}

CGPUBufferId ResourceAllocationPhase::get_resource(BufferNode* buffer, ECGPUResourceState* pOutState) const
{
    const auto& aliasing_result = aliasing_phase_.get_result();
    if (auto redirect_it = aliasing_result.resource_to_bucket.find(buffer))
    {
        const auto bucket_id = redirect_it.value();
        auto it = allocation_result_.bucket_id_to_buffers.find(bucket_id);
        if (!it)
        {
            auto bucket = aliasing_result.memory_buckets[bucket_id];
            for (auto r : bucket.aliased_resources)
            {
                SKR_LOG_DEBUG(u8"! %s", r->get_name());
            }
            for (auto [id, buf] : allocation_result_.bucket_id_to_textures)
            {
                SKR_LOG_DEBUG(u8"!");
            }
        }
        if (pOutState)
            *pOutState = it.value().init_state;
        return it.value().v;
    }
    return buffer->get_imported();
}

} // namespace skr::RG