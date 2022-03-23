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
    TextureRead,  // SRV
    TextureWrite, // RTV/DSV
    BufferRead,   // CBV
    BufferWrite,  // UAV
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
using PassHandle = ObjectHandle<EObjectType::Pass>;
using TextureHandle = ObjectHandle<EObjectType::Texture>;
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