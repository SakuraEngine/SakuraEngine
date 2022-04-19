#pragma once
#include <atomic>
#include "render_graph/frontend/base_types.hpp"

namespace sakura
{
namespace render_graph
{
class ResourceNode : public RenderGraphNode
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
        const uint32_t from;
        const uint32_t to;
    };
    inline const bool is_imported() const { return imported; }
    inline const bool allow_lone() const { return canbe_lone; }
    const LifeSpan lifespan() const;

protected:
    bool imported : 1;
    bool canbe_lone : 1;
};

class TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    TextureNode()
        : ResourceNode(EObjectType::Texture)
    {
    }
    inline const TextureHandle get_handle() const { return TextureHandle(get_id()); }
    inline const CGpuTextureDescriptor& get_desc() const { return descriptor; }

protected:
    CGpuTextureDescriptor descriptor = {};
    // temporal handle with a lifespan of only one frame
    CGpuTextureId frame_texture = nullptr;
    ECGpuResourceState init_state = RESOURCE_STATE_UNDEFINED;
};

class BufferNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    BufferNode()
        : ResourceNode(EObjectType::Buffer)
    {
    }
    inline const BufferHandle get_handle() const { return BufferHandle(get_id()); }
    inline const CGpuBufferDescriptor& get_desc() const { return descriptor; }

protected:
    CGpuBufferDescriptor descriptor = {};
    // temporal handle with a lifespan of only one frame
    CGpuBufferId frame_buffer = nullptr;
    ECGpuResourceState init_state = RESOURCE_STATE_UNDEFINED;
};
} // namespace render_graph
} // namespace sakura