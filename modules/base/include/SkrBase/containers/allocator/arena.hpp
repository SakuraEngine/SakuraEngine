#pragma once
#include "SkrRT/base/config.hpp"

namespace skr
{
struct IArena {
public:
    virtual ~IArena() = default;

    // alloc
    virtual void* alloc(size_t size, size_t alignment)            = 0;
    virtual void* realloc(void* p, size_t size, size_t alignment) = 0;
    virtual void  free(void* p)                                   = 0;
    // TODO. max capacity

    // help
    template <typename T>
    SKR_INLINE T* alloc(size_t count = 1) { return (T*)alloc(count * sizeof(T), alignof(T)); }
    template <typename T>
    SKR_INLINE T* realloc(T* p, size_t count = 1) { return (T*)realloc(p, count * sizeof(T), alignof(T)); }
    template <typename T>
    SKR_INLINE void free(T* p) { free(p); }
};
} // namespace skr

// default allocator
namespace skr
{
IArena* default_arena();
}