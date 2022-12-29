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

const struct CGPUXBindTable* BindablePassContext::create_and_update_bind_table(CGPURootSignatureId root_sig) SKR_NOEXCEPT
{
    return graph->alloc_update_pass_bind_table(*executor, pass, root_sig);
}

const struct CGPUXMergedBindTable* BindablePassContext::merge_tables(const struct CGPUXBindTable **tables, uint32_t count) SKR_NOEXCEPT
{
    // allocate merged table from pool in executor
    return executor->merge_tables(tables, count);
}

void RenderPassContext::merge_and_bind_tables(const struct CGPUXBindTable **tables, uint32_t count) SKR_NOEXCEPT
{
    // allocate merged table from pool in executor
    const struct CGPUXMergedBindTable* merged_table = executor->merge_tables(tables, count);
    // bind merged table to cmd buffer
    cgpux_render_encoder_bind_merged_bind_table(encoder, merged_table);
}

void ComputePassContext::merge_and_bind_tables(const struct CGPUXBindTable **tables, uint32_t count) SKR_NOEXCEPT
{
    // allocate merged table from pool in executor
    const struct CGPUXMergedBindTable* merged_table = executor->merge_tables(tables, count);
    // bind merged table to cmd buffer
    cgpux_compute_encoder_bind_merged_bind_table(encoder, merged_table);
}

} // namespace render_graph
} // namespace skr