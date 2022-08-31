#pragma once
#include "render_graph/frontend/resource_node.hpp"

namespace skr
{
namespace render_graph
{
class PassNode;
class TextureEdge : public RenderGraphEdge
{
public:
    inline TextureEdge(ERelationshipType type, ECGPUResourceState requested_state)
        : RenderGraphEdge(type)
        , requested_state(requested_state)
    {
    }
    virtual ~TextureEdge() = default;
    virtual TextureNode* get_texture_node() = 0;
    virtual PassNode* get_pass_node() = 0;
    const ECGPUResourceState requested_state;
};

class TextureReadEdge : public TextureEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t set = UINT32_MAX;
    const uint32_t binding = UINT32_MAX;
    const eastl::string name = "";

    TextureNode* get_texture_node() final;
    PassNode* get_pass_node() final;
    inline uint32_t get_array_base() const { return handle.array_base; }
    inline uint32_t get_array_count() const { return handle.array_count; }
    inline uint32_t get_mip_base() const { return handle.mip_base; }
    inline uint32_t get_mip_count() const { return handle.mip_count; }
    inline ECGPUTextureDimension get_dimension() const { return handle.dim; }

protected:
    const TextureSRVHandle handle;
    TextureReadEdge(
    uint32_t set, uint32_t binding, TextureSRVHandle handle,
    ECGPUResourceState state = CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    TextureReadEdge(
    const char8_t* name, TextureSRVHandle handle,
    ECGPUResourceState state = CGPU_RESOURCE_STATE_SHADER_RESOURCE);
};

class TextureReadWriteEdge : public TextureEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t set;
    const uint32_t binding;
    const eastl::string name;

    TextureNode* get_texture_node() final;
    PassNode* get_pass_node() final;

protected:
    const TextureUAVHandle handle;
    TextureReadWriteEdge(
        uint32_t set, uint32_t binding, TextureUAVHandle handle,
        ECGPUResourceState state = CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    TextureReadWriteEdge(
        const char8_t* name, TextureUAVHandle handle,
        ECGPUResourceState state = CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
};

class TextureRenderEdge : public TextureEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t mrt_index;

    TextureNode* get_texture_node() final;
    PassNode* get_pass_node() final;
    inline uint32_t get_array_base() const { return handle.array_base; }
    inline uint32_t get_array_count() const { return handle.array_count; }
    inline uint32_t get_mip_level() const { return handle.mip_level; }

protected:
    TextureRenderEdge(uint32_t mrt_index, TextureRTVHandle handle, CGPUClearValue clear_value,
    ECGPUResourceState state = CGPU_RESOURCE_STATE_RENDER_TARGET)
        : TextureEdge(ERelationshipType::TextureWrite, state)
        , mrt_index(mrt_index)
        , handle(handle)
        , clear_value(clear_value)
    {
    }
    TextureRTVHandle handle;
    CGPUClearValue clear_value;
};

class BufferEdge : public RenderGraphEdge
{
public:
    inline BufferEdge(ERelationshipType type, ECGPUResourceState requested_state)
        : RenderGraphEdge(type)
        , requested_state(requested_state)
    {
    }
    virtual ~BufferEdge() = default;

    virtual BufferNode* get_buffer_node() = 0;
    virtual PassNode* get_pass_node() = 0;
    const ECGPUResourceState requested_state;
};

class BufferReadEdge : public BufferEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t set;
    const uint32_t binding;
    const eastl::string name;

    BufferNode* get_buffer_node() final;
    PassNode* get_pass_node() final;

protected:
    BufferReadEdge(const char8_t* name, BufferRangeHandle handle, ECGPUResourceState state)
        : BufferEdge(ERelationshipType::BufferRead, state)
        , set(UINT32_MAX)
        , binding(UINT32_MAX)
        , name (name)
        , handle(handle)
    {
    }
    BufferRangeHandle handle;
};

class BufferReadWriteEdge : public BufferEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    BufferNode* get_buffer_node() final;
    PassNode* get_pass_node() final;

protected:
    BufferReadWriteEdge(BufferRangeHandle handle, ECGPUResourceState state)
        : BufferEdge(ERelationshipType::BufferReadWrite, state)
        , handle(handle)
    {
    }
    BufferRangeHandle handle;
};

class PipelineBufferEdge : public BufferEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    BufferNode* get_buffer_node() final;
    PassNode* get_pass_node() final;

protected:
    PipelineBufferEdge(PipelineBufferHandle handle, ECGPUResourceState state)
        : BufferEdge(ERelationshipType::PipelineBuffer, state)
        , handle(handle)
    {
    }
    PipelineBufferHandle handle;
};

inline TextureReadEdge::TextureReadEdge(
uint32_t set, uint32_t binding, TextureSRVHandle handle,
ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureRead, state)
    , set(set)
    , binding(binding)
    , handle(handle)

{
}
inline TextureReadEdge::TextureReadEdge(
const char8_t* name, TextureSRVHandle handle,
ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureRead, state)
    , set(UINT32_MAX)
    , binding(UINT32_MAX)
    , name(name)
    , handle(handle)
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
ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureReadWrite, state)
    , set(set)
    , binding(binding)
    , handle(handle)

{
}

inline TextureReadWriteEdge::TextureReadWriteEdge(
const char8_t* name, TextureUAVHandle handle,
ECGPUResourceState state)
    : TextureEdge(ERelationshipType::TextureReadWrite, state)
    , set(UINT32_MAX)
    , binding(UINT32_MAX)
    , name(name)
    , handle(handle)

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

// pipeline buffer
inline BufferNode* PipelineBufferEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(from());
}
inline PassNode* PipelineBufferEdge::get_pass_node()
{
    return (PassNode*)to();
}

inline BufferNode* BufferReadEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(from());
}
inline PassNode* BufferReadEdge::get_pass_node()
{
    return (PassNode*)to();
}

inline BufferNode* BufferReadWriteEdge::get_buffer_node()
{
    return static_cast<BufferNode*>(to());
}
inline PassNode* BufferReadWriteEdge::get_pass_node()
{
    return (PassNode*)from();
}

} // namespace render_graph
} // namespace skr