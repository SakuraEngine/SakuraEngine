#pragma once
#include <SkrContainers/hashmap.hpp>
#include <SkrContainers/stl_deque.hpp>
#include "SkrGraphics/api.h"

namespace skr
{
namespace render_graph
{
class TexturePool
{
public:
    struct AllocationMark {
        uint64_t frame_index;
        uint32_t tags;
    };
    struct PooledTexture
    {
        PooledTexture() = delete;
        PooledTexture(CGPUTextureId texture, ECGPUResourceState state, AllocationMark mark)
            : texture(texture), state(state), mark(mark)
        {

        }
        CGPUTextureId texture;
        ECGPUResourceState state;
        AllocationMark mark;
    };
    struct Key {
        const CGPUDeviceId device;
        const CGPUTextureCreationFlags flags;
        uint64_t width;
        uint64_t height;
        uint64_t depth;
        uint32_t array_size;
        ECGPUFormat format;
        uint32_t mip_levels;
        ECGPUSampleCount sample_count;
        uint32_t sample_quality;
        CGPUResourceTypes descriptors;
        bool is_restrict_dedicated = 0;
        operator size_t() const;
        friend class TexturePool;

        struct hasher { inline size_t operator()(const Key& val) const { return (size_t)val; } };

        Key(CGPUDeviceId device, const CGPUTextureDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    std::pair<CGPUTextureId, ECGPUResourceState> allocate(const CGPUTextureDescriptor& desc, AllocationMark mark);
    void deallocate(const CGPUTextureDescriptor& desc, CGPUTextureId texture, ECGPUResourceState final_state, AllocationMark mark);

protected:
    CGPUDeviceId device;
    skr::FlatHashMap<Key, skr::stl_deque<PooledTexture>, Key::hasher> textures;
};
} // namespace render_graph
} // namespace skr