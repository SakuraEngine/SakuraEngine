#pragma once
#include <EASTL/unordered_map.h>
#include <EASTL/deque.h>
#include "utils/hash.h"
#include "utils/make_zeroed.hpp"
#include "cgpu/api.h"

namespace skr
{
namespace render_graph
{
class TexturePool
{
public:
    struct Key {
        const CGPUDeviceId device;
        const CGPUTextureCreationFlags flags;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t array_size;
        ECGPUFormat format;
        uint32_t mip_levels;
        ECGPUSampleCount sample_count;
        uint32_t sample_quality;
        CGPUResourceTypes descriptors;
        bool is_dedicated = 0;
        operator size_t() const;
        friend class TexturePool;

        Key(CGPUDeviceId device, const CGPUTextureDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGPUDeviceId device);
    void finalize();
    eastl::pair<CGPUTextureId, ECGPUResourceState> allocate(const CGPUTextureDescriptor& desc, uint64_t frame_index);
    void deallocate(const CGPUTextureDescriptor& desc, CGPUTextureId texture, ECGPUResourceState final_state, uint64_t frame_index);

protected:
    CGPUDeviceId device;
    eastl::unordered_map<Key,
    eastl::deque<eastl::pair<
    eastl::pair<CGPUTextureId, ECGPUResourceState>, uint64_t>>>
    textures;
};

inline TexturePool::Key::Key(CGPUDeviceId device, const CGPUTextureDescriptor& desc)
    : device(device)
    , flags(desc.flags)
    , width(desc.width)
    , height(desc.height)
    , depth(desc.depth ? desc.depth : 1)
    , array_size(desc.array_size ? desc.array_size : 1)
    , format(desc.format)
    , mip_levels(desc.mip_levels ? desc.mip_levels : 1)
    , sample_count(desc.sample_count ? desc.sample_count : CGPU_SAMPLE_COUNT_1)
    , sample_quality(desc.sample_quality)
    , descriptors(desc.descriptors)
    , is_dedicated(desc.is_dedicated)
{
}

inline TexturePool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

inline void TexturePool::initialize(CGPUDeviceId device_)
{
    device = device_;
}

inline void TexturePool::finalize()
{
    for (auto&& queue : textures)
    {
        while (!queue.second.empty())
        {
            cgpu_free_texture(queue.second.front().first.first);
            queue.second.pop_front();
        }
    }
}

inline eastl::pair<CGPUTextureId, ECGPUResourceState> TexturePool::allocate(const CGPUTextureDescriptor& desc, uint64_t frame_index)
{
    eastl::pair<CGPUTextureId, ECGPUResourceState> allocated = {
        nullptr, CGPU_RESOURCE_STATE_UNDEFINED
    };
    auto key = make_zeroed<TexturePool::Key>(device, desc);
    CGPUTextureId new_tex = nullptr;
    // add queue
    if (textures[key].empty())
    {
        new_tex = cgpu_create_texture(device, &desc);
        textures[key].push_back({ { new_tex, desc.start_state }, frame_index });
    }
    textures[key].front().second = frame_index;
    allocated = textures[key].front().first;
    textures[key].pop_front();
    return allocated;
}

inline void TexturePool::deallocate(const CGPUTextureDescriptor& desc, CGPUTextureId texture, ECGPUResourceState final_state, uint64_t frame_index)
{
    auto key = make_zeroed<TexturePool::Key>(device, desc);
    textures[key].push_back({ { texture, final_state }, frame_index });
}
} // namespace render_graph
} // namespace skr