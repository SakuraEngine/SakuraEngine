#pragma once
#include "SkrRenderGraph/frontend/base_types.hpp"

namespace skr
{
namespace RG
{

struct RenderGraphStateTracker;

class ResourceNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    friend class BarrierGenerationPhase;
    ResourceNode(EObjectType type, uint64_t frame_index) SKR_NOEXCEPT;
    virtual ~ResourceNode() SKR_NOEXCEPT = default;
    struct LifeSpan {
        uint32_t from;
        uint32_t to;
    };
    inline const bool is_imported() const SKR_NOEXCEPT { return imported; }
    inline ECGPUResourceState get_init_state() const SKR_NOEXCEPT { return init_state; }
    inline const bool allow_lone() const SKR_NOEXCEPT { return canbe_lone; }
    virtual EObjectType get_type() const SKR_NOEXCEPT = 0;
    uint32_t get_tags() const SKR_NOEXCEPT { return tags; }

protected:
    bool imported = false;
    bool canbe_lone = false;
    const uint64_t frame_index;
    uint32_t tags = kRenderGraphInvalidResourceTag;
    mutable ECGPUResourceState init_state = CGPU_RESOURCE_STATE_UNDEFINED;
    skr::InlineVector<RenderGraphStateTracker*, 1> trackers; 
};

class TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    TextureNode(uint64_t frame_index) SKR_NOEXCEPT;
    inline bool reimport(CGPUTextureId texture)
    {
        if (!imported) return false;
        imported_texture = texture;
        return true;
    }
    inline const TextureHandle get_handle() const SKR_NOEXCEPT { return TextureHandle(get_id(), frame_index); }
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
    EObjectType get_type() const SKR_NOEXCEPT override;
    CGPUTextureId get_imported() const SKR_NOEXCEPT
    {
        if (imported)
        {
            return imported_texture;
        }
        return nullptr;
    }

protected:
    CGPUTextureDescriptor descriptor = {};
    mutable CGPUTextureId imported_texture = nullptr;
};

class BufferNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    BufferNode(uint64_t frame_index) SKR_NOEXCEPT;

    inline bool reimport(CGPUBufferId buffer)
    {
        if (!imported) return false;
        imported_buffer = buffer;
        return true;
    }
    inline const BufferHandle get_handle() const SKR_NOEXCEPT { return BufferHandle(get_id(), frame_index); }
    inline const CGPUBufferDescriptor& get_desc() const SKR_NOEXCEPT { return descriptor; }
    inline const CGPUBufferViewDescriptor& get_view_desc() const SKR_NOEXCEPT { return view_desc; }
    EObjectType get_type() const SKR_NOEXCEPT override;
    CGPUBufferId get_imported() const SKR_NOEXCEPT
    {
        if (imported)
        {
            return imported_buffer;
        }
        return nullptr;
    }

protected:
    CGPUBufferDescriptor descriptor = {};
    CGPUBufferViewDescriptor view_desc = {};
    // temporal handle with a lifespan of only one frame
    mutable CGPUBufferId imported_buffer = nullptr;
};

class AccelerationStructureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    AccelerationStructureNode(uint64_t frame_index) SKR_NOEXCEPT;

    inline bool reimport(CGPUAccelerationStructureId acceleration_structure)
    {
        if (!imported) return false;
        imported_as = acceleration_structure;
        return true;
    }
    inline const AccelerationStructureHandle get_handle() const SKR_NOEXCEPT { return AccelerationStructureHandle(get_id(), frame_index); }
    EObjectType get_type() const SKR_NOEXCEPT override;
    CGPUAccelerationStructureId get_imported() const SKR_NOEXCEPT
    {
        if (imported)
        {
            return imported_as;
        }
        return nullptr;
    }

protected:
    // temporal handle with a lifespan of only one frame
    mutable CGPUAccelerationStructureId imported_as = nullptr;
};
} // namespace RG
} // namespace skr