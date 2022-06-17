#pragma once
#include <EASTL/string_map.h>
#include "render_graph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
class Blackboard
{
public:
    friend class RenderGraph;
    inline void clear()
    {
        named_passes.clear();
        named_textures.clear();
    }

protected:
    eastl::string_map<class PassNode*> named_passes;
    eastl::string_map<class TextureNode*> named_textures;
    eastl::string_map<class BufferNode*> named_buffers;
};
} // namespace render_graph
} // namespace skr