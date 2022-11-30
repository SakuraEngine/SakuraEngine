#pragma once
#include "platform/memory.h"
#include "SkrRenderGraph/frontend/base_types.hpp"

#include "tracy/Tracy.hpp"

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
#ifdef TRACY_ENABLE
        ZoneScopedN("RennderGraph::AllocateObject");
#endif
        if (auto allocated = InternalAlloc<T>())
        {
            new (allocated) T(std::forward<Args>(args)...);
            auto& pooled_size = const_cast<uint32_t&>(allocated->pooled_size);
            pooled_size = (uint32_t)sizeof(T);
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
    T* InternalAlloc() SKR_NOEXCEPT
    {
        return (T*)internalAllocateMemory(sizeof(T));
    }
    
    template<typename T>
    bool InternalFree(T* object) SKR_NOEXCEPT
    {
        if (object->pooled_size)
        {
            object->~T();
            return internalFreeMemory(object, object->pooled_size);
        }
        SkrDelete(object);
        return true;
    }

    virtual bool internalFreeMemory(void* memory, size_t size) = 0;
    virtual void* internalAllocateMemory(size_t size) = 0;
}; 
} // namespace render_graph
} // namespace skr