#pragma once
#include "render_graph/rg_config.h"
#include "utils/dependency_graph.hpp"
#include <EASTL/string.h>

namespace sakura
{
namespace render_graph
{
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
    SubResource,
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
struct ObjectHandle {
    ObjectHandle(handle_t hdl)
        : handle(hdl)
    {
    }
    inline operator handle_t() const { return handle; }

private:
    handle_t handle;
};

template <>
struct ObjectHandle<EObjectType::Buffer> {
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
    struct ShaderReadWriteHandle {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadWriteEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(_this); }

    protected:
        ShaderReadWriteHandle(const handle_t _this);
    };
    struct PipelineReferenceHandle {
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
using PipelineBufferHandle = BufferHandle::PipelineReferenceHandle;

template <>
struct ObjectHandle<EObjectType::Texture> {
    struct ShaderReadHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadEdge;
        ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
        ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
        ShaderReadHandle dimension(ECGpuTextureDimension dim) const;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

    protected:
        ShaderReadHandle(const handle_t _this,
            const uint32_t mip_base = 0, const uint32_t mip_count = 1,
            const uint32_t array_base = 0, const uint32_t array_count = 1);
        uint32_t mip_base = 0;
        uint32_t mip_count = 1;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
        ECGpuTextureDimension dim = TEX_DIMENSION_2D;
    };
    struct ShaderWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureRenderEdge;
        const handle_t _this;
        ShaderWriteHandle write_mip(uint32_t mip_level);
        ShaderWriteHandle write_array(uint32_t base, uint32_t count);
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

    protected:
        ShaderWriteHandle(const handle_t _this);
        uint32_t mip_level = 0;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
    };
    struct DepthStencilHandle : public ShaderWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureRenderEdge;

    protected:
        inline DepthStencilHandle(const handle_t _this)
            : ShaderWriteHandle(_this)
        {
        }
    };
    struct ShaderReadWriteHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadWriteEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

    protected:
        ShaderReadWriteHandle(const handle_t _this);
    };
    struct SubresourceHandle {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class RenderGraphBackend;
        friend class TextureCopyEdge;
        const handle_t _this;
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(_this); }

    protected:
        uint32_t mip_level = 0;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
        CGpuTextureViewAspects aspects = TVA_COLOR;
        SubresourceHandle(const handle_t _this);
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
    RenderGraphNode(EObjectType type)
        : type(type)
    {
    }
    inline void set_name(const char* n)
    {
        name = n;
    }
    inline const char* get_name() const
    {
        return name.c_str();
    }
    const EObjectType type;

protected:
    eastl::string name;
};

struct RenderGraphEdge : public DependencyGraphEdge {
    RenderGraphEdge(ERelationshipType type)
        : type(type)
    {
    }
    const ERelationshipType type;
};

inline TextureSRVHandle TextureSRVHandle::read_mip(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.mip_base = base;
    _.mip_count = count;
    return _;
}

inline TextureSRVHandle TextureSRVHandle::read_array(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

inline TextureSRVHandle TextureSRVHandle::dimension(ECGpuTextureDimension dim) const
{
    ShaderReadHandle _ = *this;
    _.dim = dim;
    return _;
}

inline TextureSRVHandle::ShaderReadHandle(const handle_t _this,
    const uint32_t mip_base,
    const uint32_t mip_count,
    const uint32_t array_base,
    const uint32_t array_count)
    : _this(_this)
    , mip_base(mip_base)
    , mip_count(mip_count)
    , array_base(array_base)
    , array_count(array_count)
{
}

inline TextureSRVHandle TextureHandle::read_mip(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.mip_base = base;
    _.mip_count = count;
    return _;
}

inline TextureSRVHandle TextureHandle::read_array(uint32_t base, uint32_t count) const
{
    ShaderReadHandle _ = *this;
    _.array_base = base;
    _.array_count = count;
    return _;
}

inline TextureRTVHandle::ShaderWriteHandle(const handle_t _this)
    : _this(_this)
{
}

inline TextureRTVHandle TextureRTVHandle::write_mip(uint32_t mip)
{
    TextureRTVHandle _ = *this;
    _.mip_level = mip;
    return _;
}

// UAV
inline TextureUAVHandle::ShaderReadWriteHandle(const handle_t _this)
    : _this(_this)
{
}

// Subresource
inline TextureSubresourceHandle::SubresourceHandle(const handle_t _this)
    : _this(_this)
{
}

// CBV
inline BufferCBVHandle::ShaderReadHandle(const handle_t _this)
    : _this(_this)
{
}

// UAV
inline BufferUAVHandle::ShaderReadWriteHandle(const handle_t _this)
    : _this(_this)
{
}

// VB/IB
inline PipelineBufferHandle::PipelineReferenceHandle(const handle_t _this)
    : _this(_this)
{
}
} // namespace render_graph
} // namespace sakura