#pragma once
#include <EASTL/unordered_map.h>
#include <EASTL/deque.h>
#include "utils/hash.h"
#include "cgpu/api.h"

namespace sakura
{
namespace render_graph
{
class TexturePool
{
public:
    struct Key {
        const CGpuDeviceId device;
        const CGpuTextureCreationFlags flags;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t array_size;
        ECGpuFormat format;
        uint32_t mip_levels;
        ECGpuSampleCount sample_count;
        uint32_t sample_quality;
        CGpuResourceTypes descriptors;
        bool can_aliasing;
        bool is_aliasing;
        operator size_t() const;
        friend class TexturePool;

    protected:
        Key(CGpuDeviceId device, const CGpuTextureDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGpuDeviceId device);
    void finalize();
    eastl::pair<CGpuTextureId, ECGpuResourceState> allocate(const CGpuTextureDescriptor& desc, uint64_t frame_index);
    void deallocate(const CGpuTextureDescriptor& desc, CGpuTextureId texture, ECGpuResourceState final_state, uint64_t frame_index);

protected:
    CGpuDeviceId device;
    eastl::unordered_map<Key,
        eastl::deque<eastl::pair<
            eastl::pair<CGpuTextureId, ECGpuResourceState>, uint64_t>>>
        textures;
};

inline TexturePool::Key::Key(CGpuDeviceId device, const CGpuTextureDescriptor& desc)
    : device(device)
    , flags(desc.flags)
    , width(desc.width)
    , height(desc.height)
    , depth(desc.depth ? desc.depth : 1)
    , array_size(desc.array_size ? desc.array_size : 1)
    , format(desc.format)
    , mip_levels(desc.mip_levels ? desc.mip_levels : 1)
    , sample_count(desc.sample_count ? desc.sample_count : SAMPLE_COUNT_1)
    , sample_quality(desc.sample_quality)
    , descriptors(desc.descriptors)
    , is_aliasing(desc.is_aliasing)
    , can_aliasing(desc.aliasing_capacity)
{
}

inline TexturePool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

inline void TexturePool::initialize(CGpuDeviceId device_)
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

inline eastl::pair<CGpuTextureId, ECGpuResourceState> TexturePool::allocate(const CGpuTextureDescriptor& desc, uint64_t frame_index)
{
    eastl::pair<CGpuTextureId, ECGpuResourceState> allocated = {
        nullptr, RESOURCE_STATE_UNDEFINED
    };
    const TexturePool::Key key(device, desc);
    CGpuTextureId new_tex = nullptr;
    auto&& queue_iter = textures.find(key);
    // add queue
    if (queue_iter == textures.end())
    {
        textures[key] = {};
    }
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

inline void TexturePool::deallocate(const CGpuTextureDescriptor& desc, CGpuTextureId texture, ECGpuResourceState final_state, uint64_t frame_index)
{
    const TexturePool::Key key(device, desc);
    textures[key].push_back({ { texture, final_state }, frame_index });
}
} // namespace render_graph
} // namespace sakura