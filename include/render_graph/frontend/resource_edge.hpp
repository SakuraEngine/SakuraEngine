#pragma once
#include "render_graph/frontend/resource_node.hpp"

namespace sakura
{
namespace render_graph
{
class PassNode;
class TextureReadEdge : public RenderGraphEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t set;
    const uint32_t binding;
    const uint32_t mip_base;
    const uint32_t mip_count;
    const uint32_t array_base;
    const uint32_t array_count;

    TextureNode* get_texture_node();
    PassNode* get_pass_node();

protected:
    TextureReadEdge(
        uint32_t set, uint32_t binding, TextureHandle handle,
        uint32_t mip_base, uint32_t mip_count,
        uint32_t array_base, uint32_t array_count,
        ECGpuResourceState state = RESOURCE_STATE_SHADER_RESOURCE);
    TextureHandle handle;
    const ECGpuResourceState requested_state = RESOURCE_STATE_SHADER_RESOURCE;
    // temporal handle with a lifespan of only one frame
};

class TextureRenderEdge : public RenderGraphEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t mrt_index;

    TextureNode* get_texture_node();
    PassNode* get_pass_node();

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
};

class BufferReadEdge : public RenderGraphEdge
{
};

inline TextureReadEdge::TextureReadEdge(
    uint32_t set, uint32_t binding, TextureHandle handle,
    uint32_t mip_base, uint32_t mip_count,
    uint32_t array_base, uint32_t array_count, ECGpuResourceState state)
    : RenderGraphEdge(ERelationshipType::TextureRead)
    , set(set)
    , binding(binding)
    , mip_base(mip_base)
    , mip_count(mip_count)
    , array_base(array_base)
    , array_count(array_count)
    , handle(handle)
    , requested_state(state)

{
}

inline TextureNode* TextureReadEdge::get_texture_node()
{
    return static_cast<TextureNode*>(from());
}

inline PassNode* TextureReadEdge::get_pass_node()
{
    return (PassNode*)to();
}

inline TextureNode* TextureRenderEdge::get_texture_node()
{
    return static_cast<TextureNode*>(to());
}

inline PassNode* TextureRenderEdge::get_pass_node()
{
    return (PassNode*)from();
}
} // namespace render_graph
} // namespace sakura