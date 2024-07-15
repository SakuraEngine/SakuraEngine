#pragma once
#include <new>

namespace skr::test_container
{
struct SkrTestAllocator {
    struct DummyParam {
    };
    using CtorParam                       = DummyParam; // dummy ctor param
    static constexpr bool support_realloc = false;      // realloc not supported

    inline SkrTestAllocator(DummyParam) noexcept {}
    inline SkrTestAllocator() noexcept {}
    inline ~SkrTestAllocator() noexcept {}
    inline SkrTestAllocator(const SkrTestAllocator&) {}
    inline SkrTestAllocator(SkrTestAllocator&&) noexcept {}
    inline SkrTestAllocator& operator=(const SkrTestAllocator&) { return *this; }
    inline SkrTestAllocator& operator=(SkrTestAllocator&&) noexcept { return *this; }

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
} // namespace skr::test_container