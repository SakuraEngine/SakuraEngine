#pragma once
#include "SkrRenderGraph/frontend/base_types.hpp"
#include <containers/hashmap.hpp>

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
        named_buffers.clear();
        named_textures.clear();
    }

protected:
    template<typename T>
    using FlatStringMap = skr::flat_hash_map<eastl::string, T, eastl::hash<eastl::string>>;

    FlatStringMap<class PassNode*> named_passes;
    FlatStringMap<class TextureNode*> named_textures;
    FlatStringMap<class BufferNode*> named_buffers;
};
} // namespace render_graph
} // namespace skr