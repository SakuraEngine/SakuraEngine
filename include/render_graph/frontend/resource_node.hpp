#pragma once
#include <atomic>
#include "render_graph/frontend/base_types.hpp"

namespace sakura
{
namespace render_graph
{
class ResourceNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    ResourceNode(EObjectType type)
        : RenderGraphNode(type)
        , imported(false)
    {
    }
    inline const bool is_imported() const { return imported; }
    inline const bool allow_lone() const { return canbe_lone; }

protected:
    bool imported : 1;
    bool canbe_lone : 1;
};

class TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    TextureNode()
        : ResourceNode(EObjectType::Texture)
    {
    }
    const TextureHandle get_handle() const
    {
        return TextureHandle(get_id());
    }
    const CGpuTextureDescriptor& get_desc() const
    {
        return descriptor;
    }

protected:
    CGpuTextureDescriptor descriptor;
    // temporal handle with a lifespan of only one frame
    CGpuTextureId frame_texture;
    ECGpuResourceState init_state;
};
} // namespace render_graph
} // namespace sakura