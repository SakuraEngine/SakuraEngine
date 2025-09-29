#pragma once
#include "./skr_new_delete.hpp"

template <class T>
struct skr_stl_allocator
{
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef value_type const& const_reference;
    typedef value_type* pointer;
    typedef value_type const* const_pointer;
    template <class U>
    struct rebind
    {
        typedef skr_stl_allocator<U> other;
    };

    inline skr_stl_allocator() SKR_NOEXCEPT = default;
    inline skr_stl_allocator(const skr_stl_allocator&) SKR_NOEXCEPT = default;
    template <class U>
    inline skr_stl_allocator(const skr_stl_allocator<U>&) SKR_NOEXCEPT
    {
    }
    inline skr_stl_allocator select_on_container_copy_construction() const
    {
        return *this;
    }
    inline void deallocate(T* p, size_type)
    {
        sakura_free(p);
    }

#if (__cplusplus >= 201703L) // C++17
    [[nodiscard]] inline T* allocate(size_type count)
    {
        return static_cast<T*>(sakura_new_n(count, sizeof(T)));
    }
    [[nodiscard]] inline T* allocate(size_type count, const void*)
    {
        return allocate(count);
    }
#else
    [[nodiscard]] inline pointer allocate(size_type count, const void* = 0)
    {
        return static_cast<pointer>(sakura_new_n(count, sizeof(value_type)));
    }
#endif

#if ((__cplusplus >= 201103L) || (_MSC_VER > 1900)) // C++11
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::true_type;
    template <class U, class... Args>
    inline void construct(U* p, Args&&... args)
    {
        ::new (p) U(std::forward<Args>(args)...);
    }
    template <class U>
    inline void destroy(U* p) SKR_NOEXCEPT
    {
        p->~U();
    }
#else
    inline void construct(pointer p, value_type const& val)
    {
        ::new (p) value_type(val);
    }
    inline void destroy(pointer p)
    {
        p->~value_type();
    }
#endif

    inline size_type max_size() const SKR_NOEXCEPT
    {
        return (PTRDIFF_MAX / sizeof(value_type));
    }
    inline pointer address(reference x) const
    {
        return &x;
    }
    inline const_pointer address(const_reference x) const
    {
        return &x;
    }
};

template <class T1, class T2>
inline bool operator==(const skr_stl_allocator<T1>&, const skr_stl_allocator<T2>&) SKR_NOEXCEPT
{
    return true;
}
template <class T1, class T2>
inline bool operator!=(const skr_stl_allocator<T1>&, const skr_stl_allocator<T2>&) SKR_NOEXCEPT
{
    return false;
}
