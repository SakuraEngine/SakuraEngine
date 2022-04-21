#pragma once
#include <EASTL/unordered_map.h>
#include <EASTL/deque.h>
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include "cgpu/api.h"

namespace sakura
{
namespace render_graph
{
class BufferPool
{
public:
    struct Key {
        const CGpuDeviceId device;
        uint64_t size;
        CGpuResourceTypes descriptors;
        ECGpuMemoryUsage memory_usage;
        ECGpuFormat format;
        CGpuBufferCreationFlags flags;
        uint64_t first_element;
        uint64_t elemet_count;
        uint64_t element_stride;
        operator size_t() const;
        friend class BufferPool;

        Key(CGpuDeviceId device, const CGpuBufferDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGpuDeviceId device);
    void finalize();
    eastl::pair<CGpuBufferId, ECGpuResourceState> allocate(const CGpuBufferDescriptor& desc, uint64_t frame_index);
    void deallocate(const CGpuBufferDescriptor& desc, CGpuBufferId buffer, ECGpuResourceState final_state, uint64_t frame_index);

protected:
    CGpuDeviceId device;
    eastl::unordered_map<Key,
        eastl::deque<eastl::pair<
            eastl::pair<CGpuBufferId, ECGpuResourceState>, uint64_t>>>
        buffers;
};

inline BufferPool::Key::Key(CGpuDeviceId device, const CGpuBufferDescriptor& desc)
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

inline void BufferPool::initialize(CGpuDeviceId device_)
{
    device = device_;
}

inline void BufferPool::finalize()
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

inline eastl::pair<CGpuBufferId, ECGpuResourceState> BufferPool::allocate(const CGpuBufferDescriptor& desc, uint64_t frame_index)
{
    eastl::pair<CGpuBufferId, ECGpuResourceState> allocated = {
        nullptr, RESOURCE_STATE_UNDEFINED
    };
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    auto&& queue_iter = buffers.find(key);
    // add queue
    if (queue_iter == buffers.end())
    {
        buffers[key] = {};
    }
    if (buffers[key].empty())
    {
        auto new_buffer = cgpu_create_buffer(device, &desc);
        buffers[key].push_back({ { new_buffer, desc.start_state }, frame_index });
    }
    allocated = buffers[key].front().first;
    buffers[key].pop_front();
    return allocated;
}

inline void BufferPool::deallocate(const CGpuBufferDescriptor& desc, CGpuBufferId buffer, ECGpuResourceState final_state, uint64_t frame_index)
{
    auto key = make_zeroed<BufferPool::Key>(device, desc);
    buffers[key].push_back({ { buffer, final_state }, frame_index });
}

} // namespace render_graph
} // namespace sakura