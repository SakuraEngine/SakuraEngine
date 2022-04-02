#pragma once
#include <gsl/span>
#include "render_graph/frontend/render_graph.hpp"

namespace sakura
{
namespace render_graph
{
// thread-unsafe descriptor set heap
// it's supposed to be resized only once at compile
class DescSetHeap
{
public:
    inline void expand(size_t set_count = 1)
    {
        for (uint32_t i = 0; i < root_sig->table_count; i++)
        {
            CGpuDescriptorSetDescriptor desc = {};
            desc.root_signature = root_sig;
            desc.set_index = i;
            auto new_set = cgpu_create_descriptor_set(root_sig->device, &desc);
            heap.emplace_back(new_set);
        }
    }
    inline const gsl::span<CGpuDescriptorSetId> pop()
    {
        if (cursor >= heap.size()) expand();
        auto res = gsl::span<CGpuDescriptorSetId>(
            heap.data() + cursor, root_sig->table_count);
        cursor += root_sig->table_count;
        return res;
    }
    inline void reset() { cursor = 0; }
    inline void destroy()
    {
        for (auto desc_set : heap)
            cgpu_free_descriptor_set(desc_set);
    }
    friend class RenderGraphBackend;

protected:
    DescSetHeap(CGpuRootSignatureId root_sig)
        : root_sig(root_sig)
    {
    }
    std::atomic_uint32_t cursor = 0;
    const CGpuRootSignatureId root_sig;
    eastl::vector<CGpuDescriptorSetId> heap;
};
} // namespace render_graph
} // namespace sakura