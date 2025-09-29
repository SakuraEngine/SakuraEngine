#pragma once
#include "SkrGraphics/api.h"
#include "SkrContainers/map.hpp"

namespace skr
{
namespace RG
{
class TextureViewPool
{
public:
    struct AllocationMark
    {
        uint64_t frame_index;
        uint32_t tags = 0;
    };
    struct PooledTextureView
    {
        PooledTextureView() = default;
        PooledTextureView(CGPUTextureViewId texture_view, AllocationMark mark)
            : texture_view(texture_view)
            , mark(mark)
        {
        }
        CGPUTextureViewId texture_view;
        AllocationMark mark;
    };
    struct Key
    {
        CGPUDeviceId device = nullptr;
        CGPUTextureId texture = nullptr;
        ECGPUFormat format = CGPU_FORMAT_UNDEFINED;
        CGPUTextureViewUsages usages = 0;
        CGPUTextureViewAspects aspects = 0;
        ECGPUTextureDimension dims = CGPU_TEXTURE_DIMENSION_2D;
        uint32_t base_array_layer = 0;
        uint32_t array_layer_count = 0;
        uint32_t base_mip_level = 0;
        uint32_t mip_level_count = 0;
        uint32_t tex_width = 0;
        uint32_t tex_height = 0;
        int64_t unique_id = 0;

        operator skr_hash() const;
        inline static skr_hash _skr_hash(const Key& key) { return (skr_hash)key; }

        friend class TextureViewPool;

        Key(CGPUDeviceId device, const CGPUTextureViewDescriptor& desc);
    };
    static_assert(sizeof(Key) == 64);
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    uint32_t erase(CGPUTextureId texture);
    CGPUTextureViewId allocate(const CGPUTextureViewDescriptor& desc, uint64_t frame_index);

protected:
    CGPUDeviceId device;
    skr::Map<Key, PooledTextureView> views;
};
} // namespace RG
} // namespace skr