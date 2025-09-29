#pragma once
#include "SkrGraphics/api.h"
#include "SkrDependencyGraph/dependency_graph.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/span.hpp"

enum
{
    kRenderGraphInvalidResourceTag = 0x00,
    kRenderGraphDefaultResourceTag = 0x01,
    // see D3D11 DynamicBuffer, some sync problems are dealed under render graph implementation with D3D12/Vulkan
    kRenderGraphDynamicResourceTag = 0x02
};

struct CGPUXBindTable;
struct CGPUXMergedBindTable;
namespace skr::RG
{
// fwd declartions
class ResourceNode;
class TextureNode;
class BufferNode;
class AccelerationStructureNode;
class RenderGraphBackend;

struct PassContext;
struct ComputePassContext;
struct RenderPassContext;
struct CopyPassContext;

class PassNode;
class RenderPassNode;
class ComputePassNode;
class CopyPassNode;
class PresentPassNode;

using CopyPassExecuteFunction = skr::stl_function<void(class RenderGraph&, CopyPassContext&)>;
using ComputePassExecuteFunction = skr::stl_function<void(class RenderGraph&, ComputePassContext&)>;
using RenderPassExecuteFunction = skr::stl_function<void(class RenderGraph&, RenderPassContext&)>;

enum class EObjectType : uint8_t
{
    Pass,
    Texture,
    Buffer,
    AccelerationStructure,
    Count
};

enum class ERelationshipType : uint8_t
{
    TextureRead,                // SRV
    TextureWrite,               // RTV/DSV
    TextureReadWrite,           // UAV
    PipelineBuffer,             // VB/IB...
    BufferRead,                 // CBV
    BufferReadWrite,            // UAV
    AccelerationStructureRead,  // For raytracing shaders
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

enum class EPassFlags : uint32_t
{
    None = 0x0,
    SeperateFromCommandBuffer = 0x1,  // Pass 会从主命令缓冲区提取命令
    PreferAsyncCompute = 0x2,         // Pass 倾向于在异步计算队列运行
 
    ComputeIntensive = 0x10,          // 长时间运行的计算，需要特殊调度
    VertexBoundIntensive = 0x20,      // VS/图元装配瓶颈（仅渲染 Pass）
    PixelBoundIntensive = 0x40,       // 片元/ROP 瓶颈（仅渲染 Pass）
    BandwidthIntensive = 0x80,        // 内存带宽密集型 Pass

    SmallWorkingSet = 0x100,          // 小 working set
    LargeWorkingSet = 0x200,          // 大 working set
    RandomAccess = 0x400,             // 随机内存访问（缓存不友好）
    StreamingAccess = 0x800,          // 流式访问（绕过缓存）
};

struct HandleStorage
{
public:
    HandleStorage() = default;
    HandleStorage(uint64_t id, uint64_t frame_index)
        : _id(id), _frame_index((uint32_t)frame_index)
    {
    }

    uint64_t id() const { return _id; }
    uint32_t frame_index() const { return _frame_index; }
    
    inline bool operator!=(const HandleStorage& other) const
    {
        return _id != other._id || _frame_index != other._frame_index;
    }

    inline bool operator==(const HandleStorage& other) const
    {
        return _id == other._id && _frame_index == other._frame_index;
    }

private:
    uint64_t _id = UINT16_MAX;
    uint32_t _frame_index = UINT32_MAX;
    uint32_t _reserved = UINT16_MAX;
};

inline constexpr HandleStorage kInvalidHandle = HandleStorage();

template <EObjectType type>
struct SKR_RENDER_GRAPH_API ObjectHandle {
    ObjectHandle(uint64_t id, uint64_t frame_index)
        : handle(id, frame_index)
    {
    }
    ObjectHandle(HandleStorage hdl)
        : handle(hdl)
    {
    }
    inline operator HandleStorage() const { return handle; }

    const HandleStorage handle = kInvalidHandle;
};

template <>
struct SKR_RENDER_GRAPH_API ObjectHandle<EObjectType::Buffer> {
    inline operator HandleStorage() const { return handle; }
    struct ShaderReadHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadEdge;
        const HandleStorage _this;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        ShaderReadHandle(const HandleStorage _this);
    };
    
    struct SKR_RENDER_GRAPH_API ShaderReadWriteHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadWriteEdge;
        const HandleStorage _this;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        ShaderReadWriteHandle(const HandleStorage _this);
    };

    struct SKR_RENDER_GRAPH_API RangeHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadEdge;
        const HandleStorage _this;
        const uint64_t from;
        const uint64_t to;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        inline RangeHandle(const HandleStorage _this, uint64_t from, uint64_t to)
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
        const HandleStorage _this;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        PipelineReferenceHandle(const HandleStorage _this);
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
    ObjectHandle(uint64_t id, uint64_t frame_index)
        : handle(id, frame_index)
    {
    }

    ObjectHandle(HandleStorage hdl)
        : handle(hdl)
    {
    }

private:
    HandleStorage handle = kInvalidHandle;
};
using BufferHandle = ObjectHandle<EObjectType::Buffer>;
using BufferCBVHandle = BufferHandle::ShaderReadHandle;
using BufferUAVHandle = BufferHandle::ShaderReadWriteHandle;
using BufferRangeHandle = BufferHandle::RangeHandle;
using PipelineBufferHandle = BufferHandle::PipelineReferenceHandle;

template <>
struct SKR_RENDER_GRAPH_API ObjectHandle<EObjectType::Texture> {
    struct SKR_RENDER_GRAPH_API SubresourceHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class RenderGraphBackend;
        friend class TextureCopyEdge;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

        SubresourceHandle(const HandleStorage _this);

        uint32_t get_mip_level() const { return mip_level; }
        uint32_t get_array_base() const { return array_base; }
        uint32_t get_array_count() const { return array_count; }
        CGPUTextureViewAspects get_aspects() const { return aspects; }

    protected:
        HandleStorage _this = kInvalidHandle;
        uint32_t mip_level = 0;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
        CGPUTextureViewAspects aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
    };

    struct SKR_RENDER_GRAPH_API ShaderReadHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadEdge;
        ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
        ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
        ShaderReadHandle read_cube(uint32_t base, uint32_t count) const;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

        ShaderReadHandle(const HandleStorage _this,
            const uint32_t mip_base = 0, const uint32_t mip_count = 1,
            const uint32_t array_base = 0, const uint32_t array_count = 1);
    protected:
        HandleStorage _this = kInvalidHandle;
        uint32_t mip_base = 0;
        uint32_t mip_count = 1;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
    };

    struct SKR_RENDER_GRAPH_API ShaderWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureRenderEdge;
        ShaderWriteHandle write_mip(uint32_t mip_level) const;
        ShaderWriteHandle write_array(uint32_t base, uint32_t count) const;
        ShaderWriteHandle write_cube(uint32_t base, uint32_t count) const;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

        ShaderWriteHandle(const HandleStorage _this);            
    protected:
        HandleStorage _this = kInvalidHandle;
        uint32_t mip_level = 0;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
    };

    struct SKR_RENDER_GRAPH_API DepthStencilHandle : public ShaderWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureRenderEdge;

        DepthStencilHandle clear_depth(float depth) const;
    protected:
        inline DepthStencilHandle(const HandleStorage _this)
            : ShaderWriteHandle(_this)
        {
        }

        float cleardepth = 0.f;
    };

    struct SKR_RENDER_GRAPH_API ShaderReadWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadWriteEdge;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }
        ShaderReadWriteHandle readwrite_mip(uint32_t mip) const;
        ShaderReadWriteHandle readwrite_array(uint32_t base, uint32_t count) const;
        ShaderReadWriteHandle readwrite_cube(uint32_t base, uint32_t count) const;
        ShaderReadWriteHandle(const HandleStorage _this);

    protected:
        HandleStorage _this = kInvalidHandle;
        uint32_t mip_level = 0;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
    };

    inline operator HandleStorage() const { return handle; }
    // read
    inline operator ShaderReadHandle() const { return ShaderReadHandle(handle); }
    ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
    ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
    ShaderReadHandle read_cube(uint32_t base, uint32_t count) const;
    // write
    inline operator ShaderWriteHandle() const { return ShaderWriteHandle(handle); }
    ShaderWriteHandle write_mip(uint32_t mip_level) const;
    ShaderWriteHandle write_array(uint32_t base, uint32_t count) const;
    ShaderWriteHandle write_cube(uint32_t base, uint32_t count) const;
    // readwrite
    inline operator ShaderReadWriteHandle() const { return ShaderReadWriteHandle(handle); }
    ShaderReadWriteHandle readwrite_mip(uint32_t mip) const;
    ShaderReadWriteHandle readwrite_array(uint32_t base, uint32_t count) const;
    ShaderReadWriteHandle readwrite_cube(uint32_t base, uint32_t count) const;
    // ds
    inline operator DepthStencilHandle() const { return DepthStencilHandle(handle); }
    DepthStencilHandle clear_depth(float depth) const;
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
    ObjectHandle(HandleStorage hdl)
        : handle(hdl)
    {
    }
    ObjectHandle(uint64_t id, uint64_t frame_index)
        : handle(id, frame_index)
    {
    }

private:
    HandleStorage handle = kInvalidHandle;
}; // ObjectHandle<EObjectType::Texture>
using PassHandle = ObjectHandle<EObjectType::Pass>;
using TextureHandle = ObjectHandle<EObjectType::Texture>;
using BufferHandle = ObjectHandle<EObjectType::Buffer>;
using TextureSRVHandle = TextureHandle::ShaderReadHandle;
using TextureRTVHandle = TextureHandle::ShaderWriteHandle;
using TextureDSVHandle = TextureHandle::DepthStencilHandle;
using TextureUAVHandle = TextureHandle::ShaderReadWriteHandle;
using TextureSubresourceHandle = TextureHandle::SubresourceHandle;

template <>
struct SKR_RENDER_GRAPH_API ObjectHandle<EObjectType::AccelerationStructure> {
    struct SKR_RENDER_GRAPH_API ShaderReadHandle {
        friend struct ObjectHandle<EObjectType::AccelerationStructure>;
        friend class RenderGraph;
        friend class AccelerationStructureReadEdge;
        const HandleStorage _this = kInvalidHandle;
        inline operator ObjectHandle<EObjectType::AccelerationStructure>() const { return ObjectHandle<EObjectType::AccelerationStructure>(_this); }

    protected:
        ShaderReadHandle(const HandleStorage _this);
    };

    inline operator HandleStorage() const { return handle; }
    // read access for raytracing shaders
    inline operator ShaderReadHandle() const { return ShaderReadHandle(handle); }

    friend class RenderGraph;
    friend class RenderGraphBackend;
    friend class AccelerationStructureNode;
    friend class AccelerationStructureReadEdge;
    friend struct ShaderReadHandle;
    ObjectHandle(){};

protected:
    ObjectHandle(uint64_t id, uint64_t frame_index)
        : handle(id, frame_index)
    {
    }
    ObjectHandle(HandleStorage hdl)
        : handle(hdl)
    {
    }

private:
    HandleStorage handle = kInvalidHandle;
};
using AccelerationStructureHandle = ObjectHandle<EObjectType::AccelerationStructure>;
using AccelerationStructureSRVHandle = AccelerationStructureHandle::ShaderReadHandle;

template <EObjectType Type>
inline bool operator!=(const ObjectHandle<Type>& hdl, HandleStorage storage) { return HandleStorage(hdl) != storage; }

template <EObjectType Type>
inline bool operator==(const ObjectHandle<Type>& hdl, HandleStorage storage) { return HandleStorage(hdl) == storage; }

struct RenderGraphNode : public DependencyGraphNode {
    RenderGraphNode(EObjectType type);
    SKR_RENDER_GRAPH_API void set_name(const char8_t* n);
    SKR_RENDER_GRAPH_API const char8_t* get_name() const;
    SKR_RENDER_GRAPH_API const skr::StringView get_name_view() const;
    const EObjectType type;
    const uint32_t pooled_size = 0;
protected:
    skr::String name = u8"";
};

struct RenderGraphEdge : public DependencyGraphEdge {
    RenderGraphEdge(ERelationshipType type);
    inline ERelationshipType get_type() const SKR_NOEXCEPT { return type; }
    const ERelationshipType type;
    const uint32_t pooled_size = 0;
};

struct SKR_RENDER_GRAPH_API PassContext {
    PassNode* pass = nullptr;
    skr::RG::RenderGraphBackend* graph = nullptr;
    CGPUCommandBufferId cmd;
    skr::Span<std::pair<BufferHandle, CGPUBufferId>> resolved_buffers;
    skr::Span<std::pair<TextureHandle, CGPUTextureId>> resolved_textures;
    skr::Span<std::pair<AccelerationStructureHandle, CGPUAccelerationStructureId>> resolved_acceleration_structures;

    CGPUBufferId resolve(BufferHandle buffer_handle) const;
    CGPUTextureId resolve(TextureHandle tex_handle) const;
    CGPUAccelerationStructureId resolve(AccelerationStructureHandle as_handle) const;
};

struct SKR_RENDER_GRAPH_API BindablePassContext : public PassContext {
    friend class RenderGraphBackend;
    friend struct PassExecutionPhase;

    const struct CGPUXMergedBindTable* merge_tables(const struct CGPUXBindTable** tables, uint32_t count) SKR_NOEXCEPT;

    const struct CGPUXBindTable* bind_table;
protected:
    class RenderGraphFrameExecutor* executor;
};

struct SKR_RENDER_GRAPH_API RenderPassContext : public BindablePassContext {
    friend class RenderGraphBackend;
    friend struct PassExecutionPhase;

    const CGPUXMergedBindTable* merge_and_bind_tables(const struct CGPUXBindTable** tables, uint32_t count) SKR_NOEXCEPT;
    void bind(const CGPUXMergedBindTable* tbl);

    CGPURenderPassEncoderId encoder;
};

struct SKR_RENDER_GRAPH_API ComputePassContext : public BindablePassContext {
    friend class RenderGraphBackend;
    friend struct PassExecutionPhase;

    const CGPUXMergedBindTable* merge_and_bind_tables(const struct CGPUXBindTable** tables, uint32_t count) SKR_NOEXCEPT;
    void bind(const CGPUXMergedBindTable* tbl);

    CGPUComputePassEncoderId encoder;
};

struct SKR_RENDER_GRAPH_API CopyPassContext : public PassContext {
    friend struct PassExecutionPhase;
    CGPUCommandBufferId cmd;
};
} // namespace skr::RG

namespace skr
{
template<RG::EObjectType type>
struct Hash<RG::ObjectHandle<type>>{
    inline size_t operator()(const RG::ObjectHandle<type>& handle) const SKR_NOEXCEPT
    {
        return Hash<RG::HandleStorage>()(handle);
    }
};
} // namespace skr