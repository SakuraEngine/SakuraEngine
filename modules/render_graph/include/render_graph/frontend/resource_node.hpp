#pragma once
#include <atomic>
#include "render_graph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
class SKR_RENDER_GRAPH_API ResourceNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    inline ResourceNode(EObjectType type) SKR_NOEXCEPT
        : RenderGraphNode(type),
          imported(false)
    {
    }
    virtual ~ResourceNode() SKR_NOEXCEPT = default;
    struct LifeSpan {
        uint32_t from;
        uint32_t to;
    };
    inline const bool is_imported() const SKR_NOEXCEPT { return imported; }
    inline const bool allow_lone() const SKR_NOEXCEPT { return canbe_lone; }
    const LifeSpan lifespan() const SKR_NOEXCEPT;

protected:
    bool imported : 1;
    bool canbe_lone : 1;
    mutable LifeSpan frame_lifespan = { UINT32_MAX, UINT32_MAX };
};

class SKR_RENDER_GRAPH_API TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    TextureNode() SKR_NOEXCEPT
        : ResourceNode(EObjectType::Texture)
    {
    }
    inline const TextureHandle get_handle() const SKR_NOEXCEPT { return TextureHandle(get_id()); }
    inline const CGPUTextureDescriptor& get_desc() const SKR_NOEXCEPT { return descriptor; }
    inline const uint32_t get_size() const SKR_NOEXCEPT
    {
        return descriptor.array_size * descriptor.mip_levels *
               descriptor.width * descriptor.depth * descriptor.height *
               FormatUtil_BitSizeOfBlock(descriptor.format);
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

class SKR_RENDER_GRAPH_API BufferNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    BufferNode() SKR_NOEXCEPT
        : ResourceNode(EObjectType::Buffer)
    {
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