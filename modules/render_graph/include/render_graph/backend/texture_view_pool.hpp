#pragma once
#include <EASTL/unordered_map.h>
#include "utils/hash.h"
#include "platform/debug.h"
#include "cgpu/api.h"

namespace skr
{
namespace render_graph
{
class TextureViewPool
{
public:
    struct AllocationMark {
        uint64_t frame_index;
        uint32_t tags = 0;
    };
    struct PooledTextureView
    {
        PooledTextureView() = default;
        PooledTextureView(CGPUTextureViewId texture_view, AllocationMark mark)
            : texture_view(texture_view), mark(mark)
        {

        }
        CGPUTextureViewId texture_view;
        AllocationMark mark;
    };
    struct Key {
        CGPUDeviceId device = nullptr;
        CGPUTextureId texture = nullptr;
        ECGPUFormat format = CGPU_FORMAT_UNDEFINED;
        CGPUTexutreViewUsages usages = 0;
        CGPUTextureViewAspects aspects = 0;
        ECGPUTextureDimension dims = CGPU_TEX_DIMENSION_2D;
        uint32_t base_array_layer = 0;
        uint32_t array_layer_count = 0;
        uint32_t base_mip_level = 0;
        uint32_t mip_level_count = 0;
        uint32_t tex_width = 0;
        uint32_t tex_height = 0;
        uint64_t _pad2 = 0;
        
        operator size_t() const;
        friend class TextureViewPool;
    protected:
        Key(CGPUDeviceId device, const CGPUTextureViewDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    uint32_t erase(CGPUTextureId texture);
    CGPUTextureViewId allocate(const CGPUTextureViewDescriptor& desc, uint64_t frame_index);
protected:
    CGPUDeviceId device;
    eastl::unordered_map<Key, PooledTextureView> views;
};
} // namespace render_graph
} // namespace skr