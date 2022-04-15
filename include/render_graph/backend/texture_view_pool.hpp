#pragma once
#include <EASTL/unordered_map.h>
#include "utils/hash.h"
#include "cgpu/api.h"

namespace sakura
{
namespace render_graph
{
class TextureViewPool
{
public:
    struct Key {
        const CGpuDeviceId device;
        const CGpuTextureViewDescriptor desc;
        operator size_t() const;
        friend class TextureViewPool;

    protected:
        Key(CGpuDeviceId device, const CGpuTextureViewDescriptor& desc);
    };
    friend class RenderGraphBackend;
    void initialize(CGpuDeviceId device);
    void finalize();
    CGpuTextureViewId allocate(const CGpuTextureViewDescriptor& desc, uint64_t frame_index);

protected:
    CGpuDeviceId device;
    eastl::unordered_map<Key, eastl::pair<CGpuTextureViewId, uint64_t>> views;
};

inline TextureViewPool::Key::Key(CGpuDeviceId device, const CGpuTextureViewDescriptor& desc)
    : device(device)
    , desc(desc)
{
}

inline TextureViewPool::Key::operator size_t() const
{
    return skr_hash(this, sizeof(*this), (size_t)device);
}

inline void TextureViewPool::initialize(CGpuDeviceId device_)
{
    device = device_;
}

inline void TextureViewPool::finalize()
{
    for (auto view : views)
    {
        cgpu_free_texture_view(view.second.first);
    }
}

inline CGpuTextureViewId TextureViewPool::allocate(const CGpuTextureViewDescriptor& desc, uint64_t frame_index)
{
    const TextureViewPool::Key key(device, desc);
    auto&& found = views.find(key);
    if (found != views.end())
    {
        found->second.second = frame_index;
        return found->second.first;
    }
    else
    {
        CGpuTextureViewId new_view = cgpu_create_texture_view(device, &desc);
        views.insert({ key, { new_view, frame_index } });
        return new_view;
    }
}
} // namespace render_graph
} // namespace sakura