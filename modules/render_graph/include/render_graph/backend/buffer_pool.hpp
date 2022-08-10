#pragma once
#include <EASTL/unordered_map.h>
#include <EASTL/deque.h>
#include "platform/configure.h"
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include "cgpu/api.h"

namespace skr
{
namespace render_graph
{
class BufferPool
{
public:
    struct AllocationMark {
        uint64_t frame_index;
        uint32_t tags;
    };
    struct Key {
        const CGPUDeviceId device;
        uint64_t size;
        CGPUResourceTypes descriptors;
        ECGPUMemoryUsage memory_usage;
        ECGPUFormat format;
        CGPUBufferCreationFlags flags;
        uint64_t first_element;
        uint64_t elemet_count;
        uint64_t element_stride;
        operator size_t() const;
        friend class BufferPool;

        Key(CGPUDeviceId device, const CGPUBufferDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    eastl::pair<CGPUBufferId, ECGPUResourceState> allocate(const CGPUBufferDescriptor& desc, AllocationMark mark, uint64_t min_frame_index);
    void deallocate(const CGPUBufferDescriptor& desc, CGPUBufferId buffer, ECGPUResourceState final_state, AllocationMark mark);

protected:
    CGPUDeviceId device;
    eastl::unordered_map<Key,
        eastl::deque<eastl::pair<
            eastl::pair<CGPUBufferId, ECGPUResourceState>, AllocationMark>>> buffers;
};

FORCEINLINE BufferPool::Key::Key(CGPUDeviceId device, const CGPUBufferDescriptor& desc)
    : device(device)
    , size(desc.size)
    , descriptors(desc.descriptors)
    , memory_usage(desc.memory_usage)
    , format(desc.format)
    , flags(desc.flags)
    , first_element(desc.first_element)
    , elemet_count(desc.elemet_count)
    , element_stride(desc.element_stride)
{
}

FORCEINLINE BufferPool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

FORCEINLINE void BufferPool::initialize(CGPUDeviceId device_)
{
    device = device_;
}

FORCEINLINE void BufferPool::finalize()
{
    for (auto queue : buffers)
    {
        while (!queue.second.empty())
        {
            cgpu_free_buffer(queue.second.front().first.first);
            queue.second.pop_front();
        }
    }
}

FORCEINLINE eastl::pair<CGPUBufferId, ECGPUResourceState> BufferPool::allocate(const CGPUBufferDescriptor& desc, AllocationMark mark, uint64_t min_frame_index)
{
    eastl::pair<CGPUBufferId, ECGPUResourceState> allocated = {
        nullptr, CGPU_RESOURCE_STATE_UNDEFINED
    };
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    if (buffers[key].empty() || buffers[key].front().second.frame_index < min_frame_index )
    {
        auto new_buffer = cgpu_create_buffer(device, &desc);
        buffers[key].push_front({ { new_buffer, desc.start_state }, mark });
    }
    allocated = buffers[key].front().first;
    buffers[key].pop_front();
    return allocated;
}

FORCEINLINE void BufferPool::deallocate(const CGPUBufferDescriptor& desc, CGPUBufferId buffer, ECGPUResourceState final_state, AllocationMark mark)
{
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    buffers[key].push_back({ { buffer, final_state }, mark });
}

} // namespace render_graph
} // namespace skr