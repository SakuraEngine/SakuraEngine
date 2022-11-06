#pragma once
#include "platform/configure.h"

RUNTIME_EXTERN_C RUNTIME_API void* sakura_malloc(size_t size) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_calloc(size_t count, size_t size) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_calloc_aligned(size_t count, size_t size, size_t alignment) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_malloc_aligned(size_t size, size_t alignment) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_new_n(size_t count, size_t size);
RUNTIME_EXTERN_C RUNTIME_API void* sakura_new_aligned(size_t size, size_t alignment);
RUNTIME_EXTERN_C RUNTIME_API void sakura_free(void* p) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void sakura_free_aligned(void* p, size_t alignment) SKR_NOEXCEPT;
RUNTIME_EXTERN_C RUNTIME_API void* sakura_realloc(void* p, size_t newsize) SKR_NOEXCEPT;

#ifdef _CRTDBG_MAP_ALLOC
    #define DEBUG_NEW_SOURCE_LINE (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
    #define DEBUG_NEW_SOURCE_LINE
#endif

#if defined(__cplusplus)
#include "platform/debug.h"
#include <cstddef>     // std::size_t
#include <cstdint>     // PTRDIFF_MAX
#if (__cplusplus >= 201103L) || (_MSC_VER > 1900)  // C++11
#include <type_traits> // std::true_type
#include <utility>     // std::forward
#endif

template <typename T, typename... TArgs>
[[nodiscard]] FORCEINLINE T* SkrNew(TArgs&&... params)
{
    void* pMemory = sakura_new_aligned(sizeof(T), alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T{ std::forward<TArgs>(params)... };
}

template <typename T>
[[nodiscard]] FORCEINLINE T* SkrNew()
{
    void* pMemory = sakura_new_aligned(sizeof(T), alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T();
}

template <typename T, typename... TArgs>
[[nodiscard]] FORCEINLINE T* SkrNewSized(size_t size, TArgs&&... params)
{
    SKR_ASSERT(size >= sizeof(T));
    void* pMemory = sakura_new_aligned(size, alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T{ std::forward<TArgs>(params)... };
}

template <typename T>
[[nodiscard]] FORCEINLINE T* SkrNewSized(size_t size)
{
    SKR_ASSERT(size >= sizeof(T));
    void* pMemory = sakura_new_aligned(size, alignof(T));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE T();
}

template <typename F>
[[nodiscard]] FORCEINLINE F* SkrNewLambda(F&& lambda)
{
    using ValueType = std::remove_reference_t<F>;
    void* pMemory = sakura_new_aligned(sizeof(ValueType), alignof(ValueType));
    SKR_ASSERT(pMemory != nullptr);
    return new (pMemory) DEBUG_NEW_SOURCE_LINE auto(std::forward<F>(lambda));
}

template <typename T>
void SkrDelete(T* pType)
{
    if (pType != nullptr)
    {
        pType->~T();
        sakura_free_aligned((void*)pType, alignof(T));
    }
}

template<class T> 
struct skr_stl_allocator 
{
    typedef T                 value_type;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;
    typedef value_type&       reference;
    typedef value_type const& const_reference;
    typedef value_type*       pointer;
    typedef value_type const* const_pointer;
    template <class U> struct rebind { typedef skr_stl_allocator<U> other; };

    skr_stl_allocator()                                              SKR_NOEXCEPT = default;
    skr_stl_allocator(const skr_stl_allocator&)                      SKR_NOEXCEPT = default;
    template<class U> skr_stl_allocator(const skr_stl_allocator<U>&) SKR_NOEXCEPT { }
    skr_stl_allocator  select_on_container_copy_construction() const { return *this; }
    void              deallocate(T* p, size_type) { sakura_free(p); }

#if (__cplusplus >= 201703L)  // C++17
    [[nodiscard]] T* allocate(size_type count) { return static_cast<T*>(sakura_new_n(count, sizeof(T))); }
    [[nodiscard]] T* allocate(size_type count, const void*) { return allocate(count); }
#else
    [[nodiscard]] pointer allocate(size_type count, const void* = 0) { return static_cast<pointer>(sakura_new_n(count, sizeof(value_type))); }
#endif

#if ((__cplusplus >= 201103L) || (_MSC_VER > 1900))  // C++11
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap            = std::true_type;
    using is_always_equal                        = std::true_type;
    template <class U, class ...Args> void construct(U* p, Args&& ...args) { ::new(p) U(std::forward<Args>(args)...); }
    template <class U> void destroy(U* p) SKR_NOEXCEPT { p->~U(); }
#else
    void construct(pointer p, value_type const& val) { ::new(p) value_type(val); }
    void destroy(pointer p) { p->~value_type(); }
#endif

    size_type     max_size() const SKR_NOEXCEPT { return (PTRDIFF_MAX / sizeof(value_type)); }
    pointer       address(reference x) const        { return &x; }
    const_pointer address(const_reference x) const  { return &x; }
};

template<class T1,class T2> bool operator==(const skr_stl_allocator<T1>& , const skr_stl_allocator<T2>& ) SKR_NOEXCEPT { return true; }
template<class T1,class T2> bool operator!=(const skr_stl_allocator<T1>& , const skr_stl_allocator<T2>& ) SKR_NOEXCEPT { return false; }
#endif // __cplusplus