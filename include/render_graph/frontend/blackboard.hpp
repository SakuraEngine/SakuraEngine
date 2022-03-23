#pragma once
#include <EASTL/string_map.h>
#include "render_graph/frontend/base_types.hpp"

namespace sakura
{
namespace render_graph
{
class Blackboard
{
    friend class RenderGraph;

public:
protected:
    eastl::string_map<class PassNode*> named_passes;
    eastl::string_map<class TextureNode*> named_textures;
};
} // namespace render_graph
} // namespace sakura