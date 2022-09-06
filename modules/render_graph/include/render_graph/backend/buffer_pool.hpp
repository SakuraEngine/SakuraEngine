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
    struct PooledBuffer
    {
        PooledBuffer() = delete;
        PooledBuffer(CGPUBufferId buffer, ECGPUResourceState state, AllocationMark mark)
            : buffer(buffer), state(state), mark(mark)
        {

        }
        CGPUBufferId buffer;
        ECGPUResourceState state;
        AllocationMark mark;
    };
    struct Key {
        const CGPUDeviceId device = nullptr;
        uint64_t size = 0;
        CGPUResourceTypes descriptors = CGPU_RESOURCE_TYPE_NONE;
        ECGPUMemoryUsage memory_usage = CGPU_MEM_USAGE_UNKNOWN;
        ECGPUFormat format = CGPU_FORMAT_UNDEFINED;
        CGPUBufferCreationFlags flags = 0;
        uint64_t first_element = 0;
        uint64_t elemet_count = 0;
        uint64_t element_stride = 0;
        uint64_t padding = 0;
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
    eastl::unordered_map<Key, eastl::deque<PooledBuffer>> buffers;
};

inline BufferPool::Key::Key(CGPUDeviceId device, const CGPUBufferDescriptor& desc)
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

inline BufferPool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

inline void BufferPool::initialize(CGPUDeviceId device_)
{
    device = device_;
}

inline void BufferPool::finalize()
{
    for (auto&& [key, queue] : buffers)
    {
        while (!queue.empty())
        {
            cgpu_free_buffer(queue.front().buffer);
            queue.pop_front();
        }
    }
}

inline eastl::pair<CGPUBufferId, ECGPUResourceState> BufferPool::allocate(const CGPUBufferDescriptor& desc, AllocationMark mark, uint64_t min_frame_index)
{
    eastl::pair<CGPUBufferId, ECGPUResourceState> allocated = {
        nullptr, CGPU_RESOURCE_STATE_UNDEFINED
    };
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    const auto empty = buffers[key].empty();
    const auto frame_available = empty ? false : buffers[key].front().mark.frame_index <= min_frame_index;
    if ( empty || !frame_available )
    {
        auto new_buffer = cgpu_create_buffer(device, &desc);
        buffers[key].emplace_front(new_buffer, desc.start_state , mark);
    }
    allocated = { buffers[key].front().buffer, buffers[key].front().state };
    buffers[key].pop_front();
    return allocated;
}

inline void BufferPool::deallocate(const CGPUBufferDescriptor& desc, CGPUBufferId buffer, ECGPUResourceState final_state, AllocationMark mark)
{
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    for (auto&& iter : buffers[key])
    {
        if (iter.buffer == buffer) 
            return;
    }
    buffers[key].emplace_back(buffer, final_state, mark);
}

} // namespace render_graph
} // namespace skr