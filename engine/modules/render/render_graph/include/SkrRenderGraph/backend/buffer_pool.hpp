#pragma once
#include "SkrRuntime/config.h"
#include <SkrContainers/hashmap.hpp>
#include <SkrContainers/stl_deque.hpp>
#include "SkrGraphics/api.h"
#include "SkrBase/config.h"

namespace skr
{
namespace RG
{
class BufferPool
{
public:
    struct AllocationMark
    {
        uint64_t frame_index;
        uint32_t tags;
    };
    struct PooledBuffer
    {
        SKR_FORCEINLINE PooledBuffer() = delete;
        SKR_FORCEINLINE PooledBuffer(CGPUBufferId buffer, ECGPUResourceState state, AllocationMark mark)
            : buffer(buffer)
            , state(state)
            , mark(mark)
        {
        }
        CGPUBufferId buffer;
        ECGPUResourceState state;
        AllocationMark mark;
    };
    struct Key
    {
        const CGPUDeviceId device = nullptr;
        CGPUBufferUsages descriptors = CGPU_BUFFER_USAGE_NONE;
        ECGPUMemoryUsage memory_usage = CGPU_MEM_USAGE_UNKNOWN;
        ECGPUFormat format = CGPU_FORMAT_UNDEFINED;
        CGPUBufferFlags flags = 0;
        CGPUMemoryPoolId pool = 0;
        uint64_t padding0 = 0;
        uint64_t padding1 = 0;
        uint64_t padding2 = 0;
        uint64_t padding3 = 0;
        operator size_t() const;
        struct hasher
        {
            inline size_t operator()(const Key& val) const { return (size_t)val; }
        };

        friend class BufferPool;

        Key(CGPUDeviceId device, const CGPUBufferDescriptor& desc);
    };
    static_assert(sizeof(Key) == 64);
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    std::pair<CGPUBufferId, ECGPUResourceState> allocate(const CGPUBufferDescriptor& desc, AllocationMark mark, uint64_t min_frame_index);
    void deallocate(const CGPUBufferDescriptor& desc, CGPUBufferId buffer, ECGPUResourceState final_state, AllocationMark mark);

protected:
    CGPUDeviceId device;
    skr::FlatHashMap<Key, skr::stl_deque<PooledBuffer>, Key::hasher> buffers;
};
} // namespace RG
} // namespace skr