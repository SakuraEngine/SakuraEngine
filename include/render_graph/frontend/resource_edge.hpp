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

    const uint32_t set;
    const uint32_t binding;

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
    ECGpuResourceState requested_state = RESOURCE_STATE_SHADER_RESOURCE;
    // temporal handle with a lifespan of only one frame
    CGpuTextureViewId texture_view;
};

class TextureRenderEdge : public RenderGraphEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;

    const uint32_t mrt_index;

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
    ECGpuResourceState requested_state;
    // temporal handle with a lifespan of only one frame
    CGpuTextureViewId texture_view;
};

class BufferReadEdge : public RenderGraphEdge
{
};
} // namespace render_graph
} // namespace sakura