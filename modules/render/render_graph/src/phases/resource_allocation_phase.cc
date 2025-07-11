#include "SkrRenderGraph/phases/resource_allocation_phase.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrProfile/profile.h"

namespace skr {
namespace render_graph {

void ResourceAllocationPhase::on_compile(RenderGraph* graph) SKR_NOEXCEPT
{
    // 资源分配在执行时进行，编译时不需要做任何事
}

void ResourceAllocationPhase::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResourceAllocationPhase::execute");
    
    // 清理缓存
    resolved_textures_cache.clear();
    resolved_buffers_cache.clear();
}

CGPUTextureId ResourceAllocationPhase::resolve_texture(RenderGraph* graph, RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResolveTexture");
    
    // 检查缓存
    auto it = resolved_textures_cache.find(node.get_handle());
    if (it != resolved_textures_cache.end())
    {
        return it->second;
    }
    
    CGPUTextureId result = nullptr;
    
    if (!node.frame_texture)
    {
        if (auto aliased = try_aliasing_allocate(graph, executor, node); aliased)
        {
            node.frame_texture = aliased;
            node.init_state = CGPU_RESOURCE_STATE_UNDEFINED;
        }
        else
        {
            auto backend = static_cast<RenderGraphBackend*>(graph);
            auto allocated = backend->texture_pool.allocate(node.descriptor, { backend->frame_index, node.tags });
            node.frame_texture = node.imported ? node.frame_texture : allocated.first;
            node.init_state = allocated.second;
        }
    }
    
    result = node.frame_texture;
    resolved_textures_cache[node.get_handle()] = result;
    return result;
}

CGPUTextureId ResourceAllocationPhase::try_aliasing_allocate(RenderGraph* graph, RenderGraphFrameExecutor& executor, const TextureNode& node) SKR_NOEXCEPT
{
    if (node.frame_aliasing_source)
    {
        SkrZoneScopedN("AllocateAliasingResource");
        
        auto backend = static_cast<RenderGraphBackend*>(graph);
        auto device = backend->get_backend_device();
        
        // allocate & try bind
        auto aliasing_texture = cgpu_create_texture(device, &node.descriptor);
        CGPUTextureAliasingBindDescriptor aliasing_desc = {};
        aliasing_desc.aliased = resolve_texture(graph, executor, *node.frame_aliasing_source);
        aliasing_desc.aliasing = aliasing_texture;
        node.frame_aliasing = cgpu_try_bind_aliasing_texture(device, &aliasing_desc);
        
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

CGPUBufferId ResourceAllocationPhase::resolve_buffer(RenderGraph* graph, RenderGraphFrameExecutor& executor, const BufferNode& node) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResolveBuffer");
    
    // 检查缓存
    auto it = resolved_buffers_cache.find(node.get_handle());
    if (it != resolved_buffers_cache.end())
    {
        return it->second;
    }
    
    CGPUBufferId result = nullptr;
    
    if (!node.frame_buffer)
    {
        auto backend = static_cast<RenderGraphBackend*>(graph);
        uint64_t latest_frame = (node.tags & kRenderGraphDynamicResourceTag) ? 
            backend->get_latest_finished_frame() : UINT64_MAX;
        auto allocated = backend->buffer_pool.allocate(node.descriptor, 
            { backend->frame_index, node.tags }, latest_frame);
        node.frame_buffer = node.imported ? node.frame_buffer : allocated.first;
        node.init_state = allocated.second;
    }
    
    result = node.frame_buffer;
    resolved_buffers_cache[node.get_handle()] = result;
    return result;
}

} // namespace render_graph
} // namespace skr