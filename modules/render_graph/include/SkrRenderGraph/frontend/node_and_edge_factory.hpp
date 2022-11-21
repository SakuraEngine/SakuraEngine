#pragma once
#include "platform/memory.h"
#include "SkrRenderGraph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
struct SKR_RENDER_GRAPH_API NodeAndEdgeFactory
{
    virtual ~NodeAndEdgeFactory() SKR_NOEXCEPT = default;
    static NodeAndEdgeFactory* Create();
    static void Destroy(NodeAndEdgeFactory* factory);

    template<typename T, typename... Args>
    T* Allocate(Args&&... args) SKR_NOEXCEPT
    {
        if (auto allocated = InternalRealloc<T>())
        {
            new (allocated) T(std::forward<Args>(args)...);
            return allocated;
        }
        return SkrNew<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    void Dealloc(T* object) SKR_NOEXCEPT
    {
        if (InternalFree<T>(object)) return;
        SkrDelete(object);
    }

    template<typename T>
    T* InternalRealloc() SKR_NOEXCEPT
    {
        // 1.fetch object from pool
        // 2.memzero
        return nullptr;
    }
    
    template<typename T>
    bool InternalFree(T* object) SKR_NOEXCEPT
    {
        // 1.dtor
        // 2.return to pool
        return false;
    }
}; 
} // namespace render_graph
} // namespace skr