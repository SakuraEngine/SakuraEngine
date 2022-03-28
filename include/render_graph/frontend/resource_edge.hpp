#pragma once
#include "render_graph/frontend/resource_node.hpp"

namespace sakura
{
namespace render_graph
{
class TextureReadEdge : public RenderGraphEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t set;
    const uint32_t binding;

    inline TextureNode* get_texture_node()
    {
        return static_cast<TextureNode*>(graph->access_node(handle.handle));
    }

protected:
    TextureReadEdge(
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
    const ECGpuResourceState requested_state = RESOURCE_STATE_SHADER_RESOURCE;
    // temporal handle with a lifespan of only one frame
    CGpuTextureViewId texture_view;
};

class TextureRenderEdge : public RenderGraphEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t mrt_index;

    inline TextureNode* get_texture_node()
    {
        return static_cast<TextureNode*>(graph->access_node(handle.handle));
    }

protected:
    TextureRenderEdge(uint32_t mrt_index, TextureHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_RENDER_TARGET)
        : RenderGraphEdge(ERelationshipType::TextureWrite)
        , mrt_index(mrt_index)
        , handle(handle)
        , requested_state(state)
    {
    }
    TextureHandle handle;
    const ECGpuResourceState requested_state;
    // temporal handle with a lifespan of only one frame
    CGpuTextureViewId texture_view;
};

class BufferReadEdge : public RenderGraphEdge
{
};
} // namespace render_graph
} // namespace sakura