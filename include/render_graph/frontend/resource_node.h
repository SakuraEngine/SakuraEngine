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
    ResourceNode(EObjectType type)
        : RenderGraphNode(type)
        , is_imported(false)
    {
    }
    const bool is_imported : 1;
};

class TextureNode : public ResourceNode
{
public:
    TextureNode()
        : ResourceNode(EObjectType::Texture){};
    const TextureHandle get_handle() const
    {
        return TextureHandle(get_id());
    }

protected:
    // temporal handle with a lifespan of only one frame
    CGpuTextureId frame_texture;
};

class TextureReferenceEdge : public RenderGraphEdge
{
    friend class PassNode;
    friend class RenderGraph;

public:
    const uint32_t set;
    const uint32_t binding;

protected:
    TextureReferenceEdge(
        uint32_t set, uint32_t binding, TextureHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_SHADER_RESOURCE)
        : RenderGraphEdge(ERelationshipType::TextureRead)
        , set(set)
        , binding(binding)
        , handle(handle)
        , requested_state(state)
    {
    }
    TextureHandle handle;
    ECGpuResourceState requested_state = RESOURCE_STATE_SHADER_RESOURCE;
    // temporal handle with a lifespan of only one frame
    CGpuTextureViewId texture_view;
};

class TextureAccessEdge : public RenderGraphEdge
{
    friend class PassNode;
    friend class RenderGraph;

public:
    const uint32_t mrt_index;

protected:
    TextureAccessEdge(uint32_t mrt_index, TextureHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_RENDER_TARGET)
        : RenderGraphEdge(ERelationshipType::TextureWrite)
        , mrt_index(mrt_index)
        , handle(handle)
        , requested_state(state)
    {
    }
    TextureHandle handle;
    ECGpuResourceState requested_state;
    // temporal handle with a lifespan of only one frame
    CGpuTextureViewId texture_view;
};
} // namespace render_graph
} // namespace sakura