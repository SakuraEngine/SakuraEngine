#pragma once
#include <atomic>
#include <EASTL/vector.h>
#include <containers/span.hpp>
#include "cgpu/api.h"

namespace skr
{
namespace render_graph
{
// thread-unsafe descriptor set heap
// it's supposed to be resized only once at compile
class DescSetHeap
{
    friend class RenderGraphBackend;
public:
    void expand(size_t set_count = 1);
    const skr::span<CGPUDescriptorSetId> pop();
    void reset();
    void destroy();

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