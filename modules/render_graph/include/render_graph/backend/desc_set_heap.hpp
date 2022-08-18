#pragma once
#include "utils/log.h"
#include <gsl/span>
#include "render_graph/frontend/render_graph.hpp"

namespace skr
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
        auto old_count = heap.size();
        for (uint32_t i = 0; i < root_sig->table_count; i++)
        {
            CGPUDescriptorSetDescriptor desc = {};
            desc.root_signature = root_sig;
            desc.set_index = i;
            auto new_set = cgpu_create_descriptor_set(root_sig->device, &desc);
            heap.emplace_back(new_set);

            SKR_LOG_DEBUG("create set %d in heap, address %lld", i + old_count, new_set);
            SKR_ASSERT(new_set->root_signature->device);
        }
    }
    inline const gsl::span<CGPUDescriptorSetId> pop()
    {
        if (cursor >= heap.size()) expand();
        auto res = gsl::span<CGPUDescriptorSetId>(
        heap.data() + cursor, root_sig->table_count);
        cursor += root_sig->table_count;
        return res;
    }
    inline void reset() 
    { 
        for (uint32_t i = 0; i < heap.size(); i++)
        {
            ((CGPUDescriptorSet*)heap[i])->updated = false;
        }
        cursor = 0;
    }
    inline void destroy()
    {
        for (uint32_t i = 0; i < heap.size(); i++)
        {
            SKR_LOG_DEBUG("destroy set %d in heap with %d sets, address %lld", i, heap.size(), (int64_t)heap[i]);
            cgpu_free_descriptor_set(heap[i]);
        }
    }
    friend class RenderGraphBackend;

protected:
    DescSetHeap(CGPURootSignatureId root_sig)
        : root_sig(root_sig)
    {
    }
    std::atomic_uint32_t cursor = 0;
    const CGPURootSignatureId root_sig;
    eastl::vector<CGPUDescriptorSetId> heap;
};
} // namespace render_graph
} // namespace skr