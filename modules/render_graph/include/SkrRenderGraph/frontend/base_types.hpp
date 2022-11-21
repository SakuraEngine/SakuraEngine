#pragma once
#include "SkrRenderGraph/rg_config.h"
#include "utils/dependency_graph.hpp"
#include "containers/span.hpp"
#include <EASTL/string.h>

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
// fwd declartions
class ResourceNode;
class TextureNode;
class BufferNode;

struct PassContext;
struct ComputePassContext;
struct RenderPassContext;
struct CopyPassContext;

class PassNode;
class RenderPassNode;
class ComputePassNode;
class CopyPassNode;
class PresentPassNode;

using CopyPassExecuteFunction = eastl::function<void(class RenderGraph&, CopyPassContext&)>;
using ComputePassExecuteFunction = eastl::function<void(class RenderGraph&, ComputePassContext&)>;
using RenderPassExecuteFunction = eastl::function<void(class RenderGraph&, RenderPassContext&)>;

typedef uint64_t handle_t;
enum class EObjectType : uint8_t
{
    Pass,
    Texture,
    Buffer,
    Count
};

enum class ERelationshipType : uint8_t
{
    TextureRead,      // SRV
    TextureWrite,     // RTV/DSV
    TextureReadWrite, // UAV
    PipelineBuffer,   // VB/IB...
    BufferRead,       // CBV
    BufferReadWrite,  // UAV
    Count
};

enum class EPassType : uint8_t
{
    None,
    Render,
    Compute,
    Copy,
    Present,
    Count
};

template <EObjectType type>
struct SKR_RENDER_GRAPH_API ObjectHandle {
    ObjectHandle(handle_t hdl)
        : handle(hdl)
    {
    }
    inline operator handle_t() const { return handle; }

private:
    handle_t handle;
};

template <>
struct SKR_RENDER_GRAPH_API ObjectHandle<EObjectType::Buffer> {
    inline operator handle_t() const { return handle; }
    struct ShaderReadHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        ShaderReadHandle(const handle_t _this);
    };
    
    struct SKR_RENDER_GRAPH_API ShaderReadWriteHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadWriteEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        ShaderReadWriteHandle(const handle_t _this);
    };

    struct SKR_RENDER_GRAPH_API RangeHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadEdge;
        const handle_t _this;
        const uint64_t from;
        const uint64_t to;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        inline RangeHandle(const handle_t _this, uint64_t from, uint64_t to)
            : _this(_this)
            , from(from)
            , to(to)
        {
        }
    };

    struct SKR_RENDER_GRAPH_API PipelineReferenceHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class PipelineBufferEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        PipelineReferenceHandle(const handle_t _this);
    };
    // read
    inline operator ShaderReadHandle() const { return ShaderReadHandle(handle); }
    // readwrite
    inline operator ShaderReadWriteHandle() const { return ShaderReadWriteHandle(handle); }
    // pipeline
    inline operator PipelineReferenceHandle() const { return PipelineReferenceHandle(handle); }
    // range
    inline RangeHandle range(uint64_t from, uint64_t to) const { return RangeHandle(handle, from, to); }

    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class BufferNode;
    friend class BufferReadEdge;
    friend class BufferReadWriteEdge;
    friend struct ShaderReadHandle;
    friend struct ShaderReadWriteHandle;
    ObjectHandle(){};

protected:
    ObjectHandle(handle_t hdl)
        : handle(hdl)
    {
    }

private:
    handle_t handle = UINT64_MAX;
};
using BufferHandle = ObjectHandle<EObjectType::Buffer>;
using BufferCBVHandle = BufferHandle::ShaderReadHandle;
using BufferUAVHandle = BufferHandle::ShaderReadWriteHandle;
using BufferRangeHandle = BufferHandle::RangeHandle;
using PipelineBufferHandle = BufferHandle::PipelineReferenceHandle;

template <>
struct SKR_RENDER_GRAPH_API ObjectHandle<EObjectType::Texture> {
    struct SKR_RENDER_GRAPH_API ShaderReadHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadEdge;
        ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
        ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
        ShaderReadHandle dimension(ECGPUTextureDimension dim) const;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

        ShaderReadHandle(const handle_t _this,
            const uint32_t mip_base = 0, const uint32_t mip_count = 1,
            const uint32_t array_base = 0, const uint32_t array_count = 1);
    protected:
        uint32_t mip_base = 0;
        uint32_t mip_count = 1;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
        ECGPUTextureDimension dim = CGPU_TEX_DIMENSION_2D;
    };

    struct SKR_RENDER_GRAPH_API ShaderWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureRenderEdge;
        const handle_t _this;
        ShaderWriteHandle write_mip(uint32_t mip_level);
        ShaderWriteHandle write_array(uint32_t base, uint32_t count);
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

        ShaderWriteHandle(const handle_t _this);            
    protected:
        uint32_t mip_level = 0;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
    };

    struct SKR_RENDER_GRAPH_API DepthStencilHandle : public ShaderWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureRenderEdge;

    protected:
        inline DepthStencilHandle(const handle_t _this)
            : ShaderWriteHandle(_this)
        {
        }
    };

    struct SKR_RENDER_GRAPH_API ShaderReadWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadWriteEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

        ShaderReadWriteHandle(const handle_t _this);
    };

    struct SKR_RENDER_GRAPH_API SubresourceHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class RenderGraphBackend;
        friend class TextureCopyEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

        SubresourceHandle(const handle_t _this);
    protected:
        uint32_t mip_level = 0;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
        CGPUTextureViewAspects aspects = CGPU_TVA_COLOR;
    };

    inline operator handle_t() const { return handle; }
    // read
    inline operator ShaderReadHandle() const { return ShaderReadHandle(handle); }
    ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
    ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
    // write
    inline operator ShaderWriteHandle() const { return ShaderWriteHandle(handle); }
    ShaderWriteHandle write_mip(uint32_t mip_level);
    ShaderWriteHandle write_array(uint32_t base, uint32_t count);
    // readwrite
    inline operator ShaderReadWriteHandle() const { return ShaderReadWriteHandle(handle); }
    // ds
    inline operator DepthStencilHandle() const { return DepthStencilHandle(handle); }
    // subresource
    inline operator SubresourceHandle() const { return SubresourceHandle(handle); }
    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class TextureNode;
    friend class TextureReadEdge;
    friend class TextureRenderEdge;
    friend struct ShaderReadHandle;
    friend struct ShaderWriteHandle;
    friend struct ShaderReadWriteHandle;
    friend struct SubresourceHandle;
    ObjectHandle(){};

protected:
    ObjectHandle(handle_t hdl)
        : handle(hdl)
    {
    }

private:
    handle_t handle = UINT64_MAX;
};
using PassHandle = ObjectHandle<EObjectType::Pass>;
using TextureHandle = ObjectHandle<EObjectType::Texture>;
using TextureSRVHandle = TextureHandle::ShaderReadHandle;
using TextureRTVHandle = TextureHandle::ShaderWriteHandle;
using TextureDSVHandle = TextureHandle::DepthStencilHandle;
using TextureUAVHandle = TextureHandle::ShaderReadWriteHandle;
using TextureSubresourceHandle = TextureHandle::SubresourceHandle;

struct RenderGraphNode : public DependencyGraphNode {
    RenderGraphNode(EObjectType type);
    SKR_RENDER_GRAPH_API void set_name(const char* n);
    SKR_RENDER_GRAPH_API const char* get_name() const;
    const EObjectType type;
protected:
    eastl::string name;
};

struct RenderGraphEdge : public DependencyGraphEdge {
    RenderGraphEdge(ERelationshipType type);
    const ERelationshipType type;
};

struct SKR_RENDER_GRAPH_API PassContext {
    CGPUCommandBufferId cmd;
    skr::span<eastl::pair<BufferHandle, CGPUBufferId>> resolved_buffers;
    skr::span<eastl::pair<TextureHandle, CGPUTextureId>> resolved_textures;

    inline CGPUBufferId resolve(BufferHandle buffer_handle) const
    {
        for (auto iter : resolved_buffers)
        {
            if (iter.first == buffer_handle) return iter.second;
        }
        return nullptr;
    }
    inline CGPUTextureId resolve(TextureHandle tex_handle) const
    {
        for (auto iter : resolved_textures)
        {
            if (iter.first == tex_handle) return iter.second;
        }
        return nullptr;
    }
};

struct RenderPassContext : public PassContext {
    CGPURenderPassEncoderId encoder;
    skr::span<CGPUDescriptorSetId> desc_sets;
};

struct SKR_RENDER_GRAPH_API ComputePassContext : public PassContext {
    CGPUComputePassEncoderId encoder;
    skr::span<CGPUDescriptorSetId> desc_sets;
};

struct CopyPassContext : public PassContext {
    CGPUCommandBufferId cmd;
};
} // namespace render_graph
} // namespace skr