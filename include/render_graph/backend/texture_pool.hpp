#pragma once
#include <EASTL/unordered_map.h>
#include <EASTL/queue.h>
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
        CGpuDeviceId device;
        CGpuTextureCreationFlags flags;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t array_size;
        ECGpuFormat format;
        uint32_t mip_levels;
        ECGpuSampleCount sample_count;
        uint32_t sample_quality;
        CGpuResourceTypes descriptors;
        Key(CGpuDeviceId device, const CGpuTextureDescriptor& desc);
        operator size_t() const;
    };
    friend class RenderGraphBackend;
    void initialize(CGpuDeviceId device);
    void finalize();
    CGpuTextureId allocate(const CGpuTextureDescriptor& desc);
    void deallocate(const CGpuTextureDescriptor& desc, CGpuTextureId texture);

protected:
    CGpuDeviceId device;
    eastl::unordered_map<Key, eastl::queue<CGpuTextureId>> textures;
};

inline TexturePool::Key::Key(CGpuDeviceId device, const CGpuTextureDescriptor& desc)
    : device(device)
    , flags(desc.flags)
    , width(desc.width)
    , height(desc.height)
    , depth(desc.depth)
    , array_size(desc.array_size)
    , format(desc.format)
    , mip_levels(desc.mip_levels)
    , sample_count(desc.sample_count)
    , sample_quality(desc.sample_quality)
    , descriptors(desc.descriptors)
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
    for (auto queue : textures)
    {
        while (!queue.second.empty())
        {
            cgpu_free_texture(queue.second.front());
            queue.second.pop();
        }
    }
}

inline CGpuTextureId TexturePool::allocate(const CGpuTextureDescriptor& desc)
{
    CGpuTextureId allocated = nullptr;
    const TexturePool::Key key(device, desc);
    auto&& queue_iter = textures.find(key);
    // add queue
    if (queue_iter == textures.end())
    {
        textures[key] = {};
    }
    if (textures[key].empty())
    {
        textures[key].push(cgpu_create_texture(device, &desc));
    }
    allocated = textures[key].front();
    textures[key].pop();
    return allocated;
}

inline void TexturePool::deallocate(const CGpuTextureDescriptor& desc, CGpuTextureId texture)
{
    const TexturePool::Key key(device, desc);
    textures[key].push(texture);
}
} // namespace render_graph
} // namespace sakura