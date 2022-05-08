#pragma once
#include <atomic>
#include "render_graph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
class RUNTIME_API ResourceNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    inline ResourceNode(EObjectType type)
        : RenderGraphNode(type)
        , imported(false)
    {
    }
    virtual ~ResourceNode() = default;
    struct LifeSpan {
        uint32_t from;
        uint32_t to;
    };
    inline const bool is_imported() const { return imported; }
    inline const bool allow_lone() const { return canbe_lone; }
    const LifeSpan lifespan() const;

protected:
    bool imported : 1;
    bool canbe_lone : 1;
    mutable LifeSpan frame_lifespan = { UINT32_MAX, UINT32_MAX };
};

class RUNTIME_API TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    TextureNode()
        : ResourceNode(EObjectType::Texture)
    {
    }
    inline const TextureHandle get_handle() const { return TextureHandle(get_id()); }
    inline const CGPUTextureDescriptor& get_desc() const { return descriptor; }
    inline const uint32_t get_size() const
    {
        return descriptor.array_size * descriptor.mip_levels *
               descriptor.width * descriptor.depth * descriptor.height *
               FormatUtil_BitSizeOfBlock(descriptor.format);
    }
    inline const ECGPUSampleCount get_sample_count() const { return descriptor.sample_count; }
    inline const TextureNode* get_aliasing_parent() const { return frame_aliasing_source; }

protected:
    CGPUTextureDescriptor descriptor = {};
    // temporal handle with a lifespan of only one frame
    TextureNode* frame_aliasing_source = nullptr;
    mutable CGPUTextureId frame_texture = nullptr;
    mutable ECGPUResourceState init_state = CGPU_RESOURCE_STATE_UNDEFINED;
    mutable bool frame_aliasing = false;
};

class RUNTIME_API BufferNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    BufferNode()
        : ResourceNode(EObjectType::Buffer)
    {
    }
    inline const BufferHandle get_handle() const { return BufferHandle(get_id()); }
    inline const CGPUBufferDescriptor& get_desc() const { return descriptor; }

protected:
    CGPUBufferDescriptor descriptor = {};
    // temporal handle with a lifespan of only one frame
    mutable CGPUBufferId frame_buffer = nullptr;
    mutable ECGPUResourceState init_state = CGPU_RESOURCE_STATE_UNDEFINED;
};
} // namespace render_graph
} // namespace skr