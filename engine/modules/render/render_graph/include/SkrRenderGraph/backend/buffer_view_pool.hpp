#pragma once
#include "SkrGraphics/api.h"
#include "SkrContainers/map.hpp"

namespace skr
{
namespace RG
{
class BufferViewPool
{
public:
    struct AllocationMark
    {
        uint64_t frame_index;
        uint32_t tags = 0;
    };
    struct PooledBufferView
    {
        PooledBufferView() = default;
        PooledBufferView(CGPUBufferViewId buffer_view, AllocationMark mark)
            : buffer_view(buffer_view)
            , mark(mark)
        {
        }
        CGPUBufferViewId buffer_view;
        AllocationMark mark;
    };
    struct Key
    {
        CGPUDeviceId device = nullptr;
        CGPUBufferId buffer = nullptr;
        CGPUBufferViewUsages view_usages = 0;
        uint32_t offset = 0;
        uint32_t size = 0;
        ECGPUFormat texel_format = CGPU_FORMAT_UNDEFINED;
        uint32_t element_stride = 0;
        uint64_t buffer_size = 0;
        int64_t padding = 0;
        int64_t padding1 = 0;

        operator skr_hash() const;
        inline static skr_hash _skr_hash(const Key& key) { return (skr_hash)key; }

        friend class BufferViewPool;

        Key(CGPUDeviceId device, const CGPUBufferViewDescriptor& desc);
    };
    static_assert(sizeof(Key) == 64);
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    uint32_t erase(CGPUBufferId buffer);
    CGPUBufferViewId allocate(const CGPUBufferViewDescriptor& desc, uint64_t frame_index);

protected:
    CGPUDeviceId device;
    skr::Map<Key, PooledBufferView> views;
};
} // namespace RG
} // namespace skr