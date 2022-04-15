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

    const uint32_t set = UINT32_MAX;
    const uint32_t binding = UINT32_MAX;
    const eastl::string name = "";

    TextureNode* get_texture_node();
    PassNode* get_pass_node();
    inline uint32_t get_array_base() const { return handle.array_base; }
    inline uint32_t get_array_count() const { return handle.array_count; }
    inline uint32_t get_mip_base() const { return handle.mip_base; }
    inline uint32_t get_mip_count() const { return handle.mip_count; }
    inline ECGpuTextureDimension get_dimension() const { return handle.dim; }

protected:
    const TextureSRVHandle handle;
    TextureReadEdge(
        uint32_t set, uint32_t binding, TextureSRVHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_SHADER_RESOURCE);
    TextureReadEdge(
        const char8_t* name, TextureSRVHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_SHADER_RESOURCE);
    const ECGpuResourceState requested_state = RESOURCE_STATE_SHADER_RESOURCE;
    // temporal handle with a lifespan of only one frame
};

class TextureReadWriteEdge : public RenderGraphEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t set;
    const uint32_t binding;
    const eastl::string name;

    TextureNode* get_texture_node();
    PassNode* get_pass_node();

protected:
    const TextureUAVHandle handle;
    TextureReadWriteEdge(
        uint32_t set, uint32_t binding, TextureUAVHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_UNORDERED_ACCESS);
    TextureReadWriteEdge(
        const char8_t* name, TextureUAVHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_UNORDERED_ACCESS);
    const ECGpuResourceState requested_state = RESOURCE_STATE_UNORDERED_ACCESS;
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
    inline uint32_t get_array_base() const { return handle.array_base; }
    inline uint32_t get_array_count() const { return handle.array_count; }
    inline uint32_t get_mip_level() const { return handle.mip_level; }

protected:
    TextureRenderEdge(uint32_t mrt_index, TextureRTVHandle handle,
        ECGpuResourceState state = RESOURCE_STATE_RENDER_TARGET)
        : RenderGraphEdge(ERelationshipType::TextureWrite)
        , mrt_index(mrt_index)
        , handle(handle)
        , requested_state(state)
    {
    }
    TextureRTVHandle handle;
    const ECGpuResourceState requested_state;
    // temporal handle with a lifespan of only one frame
};

class BufferReadEdge : public RenderGraphEdge
{
};

inline TextureReadEdge::TextureReadEdge(
    uint32_t set, uint32_t binding, TextureSRVHandle handle,
    ECGpuResourceState state)
    : RenderGraphEdge(ERelationshipType::TextureRead)
    , set(set)
    , binding(binding)
    , handle(handle)
    , requested_state(state)

{
}
inline TextureReadEdge::TextureReadEdge(
    const char8_t* name, TextureSRVHandle handle,
    ECGpuResourceState state)
    : RenderGraphEdge(ERelationshipType::TextureRead)
    , set(UINT32_MAX)
    , binding(UINT32_MAX)
    , name(name)
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

// UAV
inline TextureReadWriteEdge::TextureReadWriteEdge(
    uint32_t set, uint32_t binding, TextureUAVHandle handle,
    ECGpuResourceState state)
    : RenderGraphEdge(ERelationshipType::TextureReadWrite)
    , set(set)
    , binding(binding)
    , handle(handle)
    , requested_state(state)

{
}

inline TextureReadWriteEdge::TextureReadWriteEdge(
    const char8_t* name, TextureUAVHandle handle,
    ECGpuResourceState state)
    : RenderGraphEdge(ERelationshipType::TextureReadWrite)
    , set(UINT32_MAX)
    , binding(UINT32_MAX)
    , name(name)
    , handle(handle)
    , requested_state(state)

{
}

inline TextureNode* TextureReadWriteEdge::get_texture_node()
{
    return static_cast<TextureNode*>(to());
}

inline PassNode* TextureReadWriteEdge::get_pass_node()
{
    return (PassNode*)from();
}

} // namespace render_graph
} // namespace sakura