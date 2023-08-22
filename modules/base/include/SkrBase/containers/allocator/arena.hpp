#pragma once
#include "SkrBase/config.h"

namespace skr
{
struct IArena {
public:
    virtual ~IArena() = default;

    // alloc
    virtual void* alloc(size_t size, size_t alignment) const            = 0;
    virtual void* realloc(void* p, size_t size, size_t alignment) const = 0;
    virtual void  free(void* p) const                                   = 0;
    // TODO. max capacity

    // help
    template <typename T>
    SKR_INLINE T* alloc(size_t count = 1) const { return (T*)alloc(count * sizeof(T), alignof(T)); }
    template <typename T>
    SKR_INLINE T* realloc(T* p, size_t count = 1) const { return (T*)realloc(p, count * sizeof(T), alignof(T)); }
    template <typename T>
    SKR_INLINE void free(T* p) const { free(p); }
};
} // namespace skr

// default allocator
namespace skr
{
IArena* default_arena();
}