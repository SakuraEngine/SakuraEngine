#include "SkrRenderGraph/phases/resource_cleanup_phase.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"
#include "SkrProfile/profile.h"

namespace skr {
namespace render_graph {

void ResourceCleanupPhase::on_compile(RenderGraph* graph) SKR_NOEXCEPT
{
    // 清理操作在执行阶段进行
}

void ResourceCleanupPhase::on_execute(RenderGraph* graph, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("ResourceCleanupPhase::execute");
    
    auto& passes = get_passes(graph);
    auto& resources = get_resources(graph);
    auto node_factory = graph->get_node_factory();
    
    {
        SkrZoneScopedN("GraphCleanup");
        
        // 3.dealloc passes & connected edges
        for (auto pass : passes)
        {
            pass->foreach_textures([this, node_factory](TextureNode* t, TextureEdge* e) {
                node_factory->Dealloc(e);
            });
            pass->foreach_buffers([this, node_factory](BufferNode* t, BufferEdge* e) {
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
        
        auto backend = static_cast<RenderGraphBackend*>(graph);
        backend->imported_buffers.clear();
        backend->imported_textures.clear();
        
        graph->clear();
    }
}

void ResourceCleanupPhase::deallocate_resources(RenderGraph* graph, PassNode* pass) SKR_NOEXCEPT
{
    SkrZoneScopedN("VirtualDeallocate");
    
    pass->foreach_textures([this, graph, pass](TextureNode* texture, TextureEdge* edge) {
        if (texture->is_last_read_write(pass))
        {
            if (texture->frame_texture && !texture->imported)
            {
                auto backend = static_cast<RenderGraphBackend*>(graph);
                backend->texture_pool.deallocate(texture->frame_texture, texture->descriptor, 
                                                 { backend->frame_index, texture->tags });
            }
            texture->frame_texture = nullptr;
        }
    });
    
    pass->foreach_buffers([this, graph, pass](BufferNode* buffer, BufferEdge* edge) {
        if (buffer->is_last_read_write(pass))
        {
            if (buffer->frame_buffer && !buffer->imported)
            {
                auto backend = static_cast<RenderGraphBackend*>(graph);
                backend->buffer_pool.deallocate(buffer->frame_buffer, buffer->descriptor, 
                                               { backend->frame_index, buffer->tags });
            }
            buffer->frame_buffer = nullptr;
        }
    });
}

} // namespace render_graph
} // namespace skr