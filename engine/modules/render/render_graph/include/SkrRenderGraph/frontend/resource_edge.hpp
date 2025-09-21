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

    inline const char8_t* get_name() const { return name.c_str(); }
    const uint64_t name_hash = 0;

    TextureNode* get_texture_node() final;
    PassNode* get_pass_node() final;
    inline uint32_t get_array_base() const { return handle.array_base; }
    inline uint32_t get_array_count() const { return handle.array_count; }
    inline uint32_t get_mip_base() const { return handle.mip_base; }
    inline uint32_t get_mip_count() const { return handle.mip_count; }

    TextureReadEdge(const skr::StringView name, TextureSRVHandle handle, ECGPUResourceState state = CGPU_RESOURCE_STATE_SHADER_RESOURCE);
protected:
    const skr::String name = u8"";
    const TextureSRVHandle handle;
};

class TextureReadWriteEdge : public TextureEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    const uint64_t name_hash = 0;

    inline const char8_t* get_name() const { return name.c_str(); }
    TextureNode* get_texture_node() final;
    PassNode* get_pass_node() final;

    inline uint32_t get_array_base() const { return handle.array_base; }
    inline uint32_t get_array_count() const { return handle.array_count; }
    inline uint32_t get_mip_level() const { return handle.mip_level; }

    TextureReadWriteEdge(const skr::StringView name, TextureUAVHandle handle, ECGPUResourceState state = CGPU_RESOURCE_STATE_UNORDERED_ACCESS);
protected:
    const skr::String name = u8"";
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
    CGPUClearValue get_clear_value() const { return clear_value; }

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

    inline const char8_t* get_name() const { return name.c_str(); }
    const uint64_t name_hash = 0;

    inline BufferRangeHandle get_handle() const { return handle; }
    BufferNode* get_buffer_node() final;
    PassNode* get_pass_node() final;

    BufferReadEdge(const skr::StringView name, BufferRangeHandle handle, ECGPUResourceState state);
protected:
    const skr::String name = u8"";
    BufferRangeHandle handle;
};

class BufferReadWriteEdge : public BufferEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    inline const char8_t* get_name() const { return name.c_str(); }
    const uint64_t name_hash = 0;

    inline BufferRangeHandle get_handle() const { return handle; }
    BufferNode* get_buffer_node() final;
    PassNode* get_pass_node() final;

    BufferReadWriteEdge(const skr::StringView name, BufferRangeHandle handle, ECGPUResourceState state);
protected:
    const skr::String name = u8"";
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

class AccelerationStructureEdge : public RenderGraphEdge
{
public:
    inline AccelerationStructureEdge(ERelationshipType type, ECGPUResourceState requested_state)
        : RenderGraphEdge(type)
        , requested_state(requested_state)
    {
    }
    virtual ~AccelerationStructureEdge() = default;

    virtual AccelerationStructureNode* get_acceleration_structure_node() = 0;
    virtual PassNode* get_pass_node() = 0;
    const ECGPUResourceState requested_state;
};

class AccelerationStructureReadEdge : public AccelerationStructureEdge
{
public:
    friend class PassNode;
    friend class RenderGraph;
    friend class RenderGraphBackend;

    inline const char8_t* get_name() const { return name.c_str(); }
    const uint64_t name_hash = 0;

    AccelerationStructureNode* get_acceleration_structure_node() final;
    PassNode* get_pass_node() final;

    AccelerationStructureReadEdge(const skr::StringView name, AccelerationStructureSRVHandle handle);
protected:
    const skr::String name = u8"";
    const AccelerationStructureSRVHandle handle;
};
} // namespace render_graph
} // namespace skr