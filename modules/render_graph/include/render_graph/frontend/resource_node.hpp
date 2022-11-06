#pragma once
#include "render_graph/frontend/base_types.hpp"

enum
{
    kRenderGraphInvalidResourceTag = 0x00,
    kRenderGraphDefaultResourceTag = 0x01,
    // see D3D11 DynamicBuffer, some sync problems are dealed under render graph implementation with D3D12/Vulkan
    kRenderGraphDynamicResourceTag = 0x02
};

namespace skr
{
namespace render_graph
{
class ResourceNode : public RenderGraphNode
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
    SKR_RENDER_GRAPH_API const LifeSpan lifespan() const SKR_NOEXCEPT;

protected:
    bool imported : 1;
    bool canbe_lone : 1;
    uint32_t tags = kRenderGraphInvalidResourceTag;
    mutable LifeSpan frame_lifespan = { UINT32_MAX, UINT32_MAX };
};

class TextureNode : public ResourceNode
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
        uint32_t asize = cgpu_max(descriptor.array_size, 1);
        uint32_t mips = cgpu_max(descriptor.mip_levels, 1);
        uint32_t width = cgpu_max(descriptor.width, 1);
        uint32_t height = cgpu_max(descriptor.height, 1);
        uint32_t depth = cgpu_max(descriptor.depth, 1);
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