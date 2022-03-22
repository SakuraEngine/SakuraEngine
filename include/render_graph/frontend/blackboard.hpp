#pragma once
#include <EASTL/string_map.h>

namespace sakura
{
namespace render_graph
{
class Blackboard
{
protected:
    eastl::string_map<class PassNode*> named_passes;
    eastl::string_map<class TextureNode*> named_textures;
};
} // namespace render_graph
} // namespace sakura