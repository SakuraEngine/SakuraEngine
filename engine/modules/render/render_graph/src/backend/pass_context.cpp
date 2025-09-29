#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderGraph/frontend/pass_node.hpp"

namespace skr
{
namespace RG
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

const struct CGPUXMergedBindTable* BindablePassContext::merge_tables(const struct CGPUXBindTable **tables, uint32_t count) SKR_NOEXCEPT
{
    // allocate merged table from pool in executor
    return executor->merge_tables(tables, count);
}

CGPUXMergedBindTableId RenderPassContext::merge_and_bind_tables(const struct CGPUXBindTable **tables, uint32_t count) SKR_NOEXCEPT
{
    // allocate merged table from pool in executor
    const struct CGPUXMergedBindTable* merged_table = executor->merge_tables(tables, count);
    // bind merged table to cmd buffer
    cgpux_render_encoder_bind_merged_bind_table(encoder, merged_table);
    return merged_table;
}

void RenderPassContext::bind(const CGPUXMergedBindTable* tbl)
{
    cgpux_render_encoder_bind_merged_bind_table(encoder, tbl);
}

CGPUXMergedBindTableId ComputePassContext::merge_and_bind_tables(const struct CGPUXBindTable **tables, uint32_t count) SKR_NOEXCEPT
{
    // allocate merged table from pool in executor
    const struct CGPUXMergedBindTable* merged_table = executor->merge_tables(tables, count);
    // bind merged table to cmd buffer
    cgpux_compute_encoder_bind_merged_bind_table(encoder, merged_table);
    return merged_table;
}

void ComputePassContext::bind(const CGPUXMergedBindTable* tbl)
{
    cgpux_compute_encoder_bind_merged_bind_table(encoder, tbl);
}

} // namespace RG
} // namespace skr