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
        ShaderReadHandle mip(uint32_t base, uint32_t count) const
        {
            ShaderReadHandle _ = *this;
            _.mip_base = base;
            _.mip_count = count;
            return _;
        }
        ShaderReadHandle array(uint32_t base, uint32_t count) const
        {
            ShaderReadHandle _ = *this;
            _.array_base = base;
            _.array_count = count;
            return _;
        }
        friend class ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadEdge;

    protected:
        ShaderReadHandle(const handle_t _this,
            const uint32_t mip_base = 0,
            const uint32_t mip_count = 1,
            const uint32_t array_base = 0,
            const uint32_t array_count = 1)
            : _this(_this)
            , mip_base(mip_base)
            , mip_count(mip_count)
            , array_base(array_base)
            , array_count(array_count)
        {
        }
        const handle_t _this;
        uint32_t mip_base = 0;
        uint32_t mip_count = 1;
        uint32_t array_base = 0;
        uint32_t array_count = 1;
    };
    operator ShaderReadHandle() const
    {
        return ShaderReadHandle(handle);
    }
    ShaderReadHandle mip(uint32_t base, uint32_t count) const
    {
        ShaderReadHandle _ = *this;
        _.mip_base = base;
        _.mip_count = count;
        return _;
    }
    ShaderReadHandle array(uint32_t base, uint32_t count) const
    {
        ShaderReadHandle _ = *this;
        _.array_base = base;
        _.array_count = count;
        return _;
    }
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
} // namespace render_graph
} // namespace sakura