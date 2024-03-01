#pragma once
#include "SkrRenderGraph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
class ResourceNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    ResourceNode(EObjectType type) SKR_NOEXCEPT;
    virtual ~ResourceNode() SKR_NOEXCEPT = default;
    struct LifeSpan {
        uint32_t from;
        uint32_t to;
    };
    inline const bool is_imported() const SKR_NOEXCEPT { return imported; }
    inline const bool allow_lone() const SKR_NOEXCEPT { return canbe_lone; }
    SKR_RENDER_GRAPH_API const LifeSpan lifespan() const SKR_NOEXCEPT;
protected:
    bool imported = false;
    bool canbe_lone = false;
    uint32_t tags = kRenderGraphInvalidResourceTag;
    mutable LifeSpan frame_lifespan = { UINT32_MAX, UINT32_MAX };
};

class TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    TextureNode() SKR_NOEXCEPT;
    inline bool reimport(CGPUTextureId texture)
    {
        if (!imported) return false;
        frame_texture = texture;
        return true;
    }
    inline const TextureHandle get_handle() const SKR_NOEXCEPT { return TextureHandle(get_id()); }
    inline const CGPUTextureDescriptor& get_desc() const SKR_NOEXCEPT { return descriptor; }
    inline const uint64_t get_size() const SKR_NOEXCEPT
    {
        uint64_t asize = cgpu_max(descriptor.array_size, 1);
        uint64_t mips = cgpu_max(descriptor.mip_levels, 1);
        uint64_t width = cgpu_max(descriptor.width, 1);
        uint64_t height = cgpu_max(descriptor.height, 1);
        uint64_t depth = cgpu_max(descriptor.depth, 1);
        return asize * mips * width * height * depth * FormatUtil_BitSizeOfBlock(descriptor.format);
    }
    inline const ECGPUSampleCount get_sample_count() const SKR_NOEXCEPT { return descriptor.sample_count; }
    inline const TextureNode* get_aliasing_parent() const SKR_NOEXCEPT { return frame_aliasing_source; }

protected:
    CGPUTextureDescriptor descriptor = {};
    // temporal handle with a lifespan of only one frame
    TextureNode* frame_aliasing_source = nullptr;
    mutable CGPUTextureId frame_texture = nullptr;
    mutable ECGPUResourceState init_state = CGPU_RESOURCE_STATE_UNDEFINED;
    mutable bool frame_aliasing = false;
};

class BufferNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    BufferNode() SKR_NOEXCEPT;

    inline bool reimport(CGPUBufferId buffer)
    {
        if (!imported) return false;
        frame_buffer = buffer;
        return true;
    }
    inline const BufferHandle get_handle() const SKR_NOEXCEPT { return BufferHandle(get_id()); }
    inline const CGPUBufferDescriptor& get_desc() const SKR_NOEXCEPT { return descriptor; }

protected:
    CGPUBufferDescriptor descriptor = {};
    // temporal handle with a lifespan of only one frame
    mutable CGPUBufferId frame_buffer = nullptr;
    mutable ECGPUResourceState init_state = CGPU_RESOURCE_STATE_UNDEFINED;
};
} // namespace render_graph
} // namespace skr