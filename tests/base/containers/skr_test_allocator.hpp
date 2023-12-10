#pragma once
#include "SkrBase/containers/allocator/allocator.hpp"
#include <new>

namespace skr::container
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
        SKR_UNREACHABLE_CODE();
        return nullptr;
    }
};

struct SkrTestAllocator_New {
    struct DummyParam {
    };
    using CtorParam                       = DummyParam; // dummy ctor param
    static constexpr bool support_realloc = false;      // realloc not supported

    inline SkrTestAllocator_New(DummyParam) noexcept {}
    inline SkrTestAllocator_New() noexcept {}
    inline ~SkrTestAllocator_New() noexcept {}
    inline SkrTestAllocator_New(const SkrTestAllocator_New&) {}
    inline SkrTestAllocator_New(SkrTestAllocator_New&&) noexcept {}
    inline SkrTestAllocator_New& operator=(const SkrTestAllocator_New&) { return *this; }
    inline SkrTestAllocator_New& operator=(SkrTestAllocator_New&&) noexcept { return *this; }

    template <typename T>
    inline static T* alloc(size_t size)
    {
        void* mem = ::operator new(size * sizeof(T), std::align_val_t(alignof(T)));
        return reinterpret_cast<T*>(mem);
    }

    template <typename T>
    inline static void free(T* p)
    {
        ::operator delete(reinterpret_cast<void*>(p), std::align_val_t(alignof(T)));
    }
};
} // namespace skr::container