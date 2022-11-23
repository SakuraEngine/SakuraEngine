#pragma once
#include "SkrRenderGraph/frontend/resource_node.hpp"

namespace skr
{
namespace render_graph
{
class PassNode;
class TextureEdge : public RenderGraphEdge
{
public:
    TextureEdge(ERelationshipType type, ECGPUResourceState requested_state) SKR_NOEXCEPT;
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
    const skr::string name = "";

    TextureNode* get_texture_node() final;
    PassNode* get_pass_node() final;
    inline uint32_t get_array_base() const { return handle.array_base; }
    inline uint32_t get_array_count() const { return handle.array_count; }
    inline uint32_t get_mip_base() const { return handle.mip_base; }
    inline uint32_t get_mip_count() const { return handle.mip_count; }
    inline ECGPUTextureDimension get_dimension() const { return handle.dim; }

    TextureReadEdge(uint32_t set, uint32_t binding, TextureSRVHandle handle, ECGPUResourceState state = CGPU_RESOURCE_STATE_SHADER_RESOURCE);
    TextureReadEdge(const char8_t* name, TextureSRVHandle handle, ECGPUResourceState state = CGPU_RESOURCE_STATE_SHADER_RESOURCE);
protected:
    const TextureSRVHandle handle;
};

class TextureReadWriteEdge : public TextureEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint32_t set;
    const uint32_t binding;
    const skr::string name;

    TextureNode* get_texture_node() final;
    PassNode* get_pass_node() final;

    TextureReadWriteEdge(uint32_t set, uint32_t binding, TextureUAVHandle handle, ECGPUResourceState state = CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
    TextureReadWriteEdge(const char8_t* name, TextureUAVHandle handle, ECGPUResourceState state = CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
protected:
    const TextureUAVHandle handle;
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

    TextureRenderEdge(uint32_t mrt_index, TextureRTVHandle handle, CGPUClearValue clear_value, ECGPUResourceState state = CGPU_RESOURCE_STATE_RENDER_TARGET);
protected:
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
    const skr::string name;

    BufferNode* get_buffer_node() final;
    PassNode* get_pass_node() final;

    BufferReadEdge(const char8_t* name, BufferRangeHandle handle, ECGPUResourceState state);
protected:
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

    BufferReadWriteEdge(BufferRangeHandle handle, ECGPUResourceState state);
protected:
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

    PipelineBufferEdge(PipelineBufferHandle handle, ECGPUResourceState state);
protected:
    PipelineBufferHandle handle;
};
} // namespace render_graph
} // namespace skr