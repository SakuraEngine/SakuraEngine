#pragma once
#include "SkrBase/containers/allocator/allocator.hpp"
#include <new>

namespace skr
{
struct SkrTestAllocator : AllocTemplate<SkrTestAllocator, size_t> {
    static void* alloc_raw(size_t size, size_t align)
    {
        return ::operator new(size, std::align_val_t(align));
    }
    static void free_raw(void* p, size_t align)
    {
        ::operator delete(p, std::align_val_t(align));
    }
    static void* realloc_raw(void* p, size_t size, size_t align)
    {
        void* new_mem = ::operator new(size, std::align_val_t(align));
        if (p)
        {
            memcpy(new_mem, p, size);
            ::operator delete(p, std::align_val_t(align));
        }
        return new_mem;
    }
};
} // namespace skr