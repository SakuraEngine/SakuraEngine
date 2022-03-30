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
    TextureRead,  // SRV
    TextureWrite, // RTV/DSV
    BufferRead,   // CBV
    BufferWrite,  // UAV
    Count
};

enum class EPassType : uint8_t
{
    None,
    Render,
    Compute,
    Present,
    Count
};

template <EObjectType type>
struct ObjectHandle {
    ObjectHandle(handle_t hdl)
        : handle(hdl)
    {
    }
    const handle_t handle;
};
template <>
struct ObjectHandle<EObjectType::Texture> {
    struct ShaderReadHandle {
        ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
        ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
        friend class ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadEdge;

    protected:
        ShaderReadHandle(const handle_t _this,
            const uint32_t mip_base = 0, const uint32_t mip_count = 1,
            const uint32_t array_base = 0, const uint32_t array_count = 1);
        const handle_t _this;
        uint32_t mip_base = 0;
        uint32_t mip_count = 1;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
    };
    struct ShaderWriteHandle {
        friend class ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureWriteEdge;
        ShaderWriteHandle load_action(ECGpuLoadAction action) const;
        ShaderWriteHandle store_action(ECGpuStoreAction action) const;

    protected:
        ShaderWriteHandle(const handle_t _this);
        const handle_t _this;
        ECGpuLoadAction load_act = LOAD_ACTION_DONTCARE;
        ECGpuStoreAction store_act = STORE_ACTION_STORE;
    };
    // read
    inline operator ShaderReadHandle() const { return ShaderReadHandle(handle); }
    ShaderReadHandle read_mip(uint32_t base, uint32_t count) const;
    ShaderReadHandle read_array(uint32_t base, uint32_t count) const;
    // write
    inline operator ShaderWriteHandle() const { return ShaderWriteHandle(handle); }
    ShaderWriteHandle load_action(ECGpuLoadAction action) const;
    ShaderWriteHandle store_action(ECGpuStoreAction action) const;
    friend class RenderGraph;
    friend class TextureNode;
    friend class TextureReadEdge;
    friend class TextureRenderEdge;

protected:
    ObjectHandle(handle_t hdl)
        : handle(hdl)
    {
    }
    const handle_t handle;
};
using PassHandle = ObjectHandle<EObjectType::Pass>;
using TextureHandle = ObjectHandle<EObjectType::Texture>;
using TextureSRVHandle = TextureHandle::ShaderReadHandle;
using TextureRTVHandle = TextureHandle::ShaderWriteHandle;
using BufferHandle = ObjectHandle<EObjectType::Buffer>;

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

inline TextureRTVHandle TextureRTVHandle::load_action(ECGpuLoadAction action) const
{
    TextureRTVHandle _ = *this;
    _.load_act = action;
    return _;
}

inline TextureRTVHandle TextureRTVHandle::store_action(ECGpuStoreAction action) const
{
    TextureRTVHandle _ = *this;
    _.store_act = action;
    return _;
}

inline TextureRTVHandle TextureHandle::load_action(ECGpuLoadAction action) const
{
    TextureRTVHandle _ = *this;
    _.load_act = action;
    return _;
}

inline TextureRTVHandle TextureHandle::store_action(ECGpuStoreAction action) const
{
    TextureRTVHandle _ = *this;
    _.store_act = action;
    return _;
}

} // namespace render_graph
} // namespace sakura