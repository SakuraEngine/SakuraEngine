#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/frontend/node_and_edge_factory.hpp"

namespace skr
{
namespace render_graph
{

CGPUBufferId PassContext::resolve(BufferHandle buffer_handle) const
{
    for (auto iter : resolved_buffers)
    {
        if (iter.first == buffer_handle) return iter.second;
    }
    return nullptr;
}
CGPUTextureId PassContext::resolve(TextureHandle tex_handle) const
{
    for (auto iter : resolved_textures)
    {
        if (iter.first == tex_handle) return iter.second;
    }
    return nullptr;
}

void BindablePassContext::merge_and_bind_tables(const struct CGPUXBindTable **tables, uint32_t count)
{
    
}

} // namespace render_graph
} // namespace skr