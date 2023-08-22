#pragma once
#include "SkrBase/config.h"
#include "SkrBase/memory/memory_traits.hpp"
#include <memory>

// for single element
namespace skr::memory
{
// construct & destruct
template <typename T>
SKR_INLINE void construct(T* p)
{
    if constexpr (memory_traits<T>::use_ctor)
    {
        new (p) T();
    }
}
template <typename T>
SKR_INLINE void destruct(T* p)
{
    if constexpr (memory_traits<T>::use_dtor)
    {
        using DestructItemTTypeDef = T;
        p->DestructItemTTypeDef::~DestructItemTTypeDef();
    }
}

// copy & assign
template <typename Dst, typename Src>
SKR_INLINE void copy(Dst* dst, Src* src)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_copy)
        {
            new (dst) Dst(*src);
        }
        else
        {
            std::memcpy(dst, src, sizeof(Src));
        }
    }
}
template <typename Dst, typename Src>
SKR_INLINE void assign(Dst* dst, Src* src)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_assign)
        {
            *dst = *src;
        }
        else
        {
            std::memcpy(dst, src, sizeof(Src));
        }
    }
}

// move copy & move assign
template <typename Dst, typename Src>
SKR_INLINE void move(Dst* dst, Src* src)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_move)
        {
            new (dst) Dst(std::move(*src));
        }
        else
        {
            std::memcpy(dst, src, sizeof(Src));
        }

        if constexpr (memory_traits<Dst, Src>::need_dtor_after_move)
        {
            destruct(src);
        }
    }
}
template <typename Dst, typename Src>
SKR_INLINE void move_assign(Dst* dst, Src* src)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_move_assign)
        {
            *dst = std::move(*src);
        }
        else
        {
            std::memcpy(dst, src);
        }

        if constexpr (memory_traits<Dst, Src>::need_dtor_after_move)
        {
            destruct(src);
        }
    }
}

// compare
template <typename A, typename B>
SKR_INLINE bool compare(const A* a, const B* b)
{
    if constexpr (memory_traits<A, B>::use_compare)
    {
        return *a != *b;
    }
    else
    {
        static_assert(sizeof(A) == sizeof(B), "keep sizeof(A) == sizeof(B) if you want use memcmp");
        return !memcmp(a, b, sizeof(B));
    }
}
} // namespace skr::memory

// for multi elements
namespace skr::memory
{
// construct & destruct
template <typename T>
SKR_INLINE void construct(T* p, size_t count)
{
    if constexpr (memory_traits<T>::use_ctor)
    {
        while (count)
        {
            new (p) T();
            ++p;
            --count;
        }
    }
}
template <typename T>
SKR_INLINE void destruct(T* p, size_t count)
{
    if constexpr (memory_traits<T>::use_dtor)
    {
        while (count)
        {
            using DestructItemTTypeDef = T;

            p->DestructItemTTypeDef::~DestructItemTTypeDef();
            ++p;
            --count;
        }
    }
}

// copy & assign
template <typename Dst, typename Src>
SKR_INLINE void copy(Dst* dst, Src* src, size_t count)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_copy)
        {
            while (count)
            {
                new (dst) Dst(*src);
                ++dst;
                ++src;
                --count;
            }
        }
        else
        {
            std::memcpy(dst, src, sizeof(Src) * count);
        }
    }
}
template <typename Dst, typename Src>
SKR_INLINE void assign(Dst* dst, Src* src, size_t count)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_assign)
        {
            while (count)
            {
                *dst = *src;
                ++dst;
                ++src;
                --count;
            }
        }
        else
        {
            std::memcpy(dst, src, sizeof(Src) * count);
        }
    }
}

// move copy & move assign
template <typename Dst, typename Src>
SKR_INLINE void move(Dst* dst, Src* src, size_t count)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_move)
        {
            if (dst < src)
            {
                while (count)
                {
                    new (dst) Dst(std::move(*src));
                    if constexpr (memory_traits<Dst, Src>::need_dtor_after_move)
                    {
                        destruct(src);
                    }

                    ++dst;
                    ++src;
                    --count;
                }
            }
            else if (dst > src)
            {
                auto dst_end = dst + count;
                auto src_end = src + count;

                while (count)
                {
                    new (dst_end) Dst(std::move(*src_end));
                    if constexpr (memory_traits<Dst, Src>::need_dtor_after_move)
                    {
                        destruct(src_end);
                    }

                    --dst_end;
                    --src_end;
                    --count;
                }
            }
        }
        else
        {
            std::memmove(dst, src, sizeof(Src) * count);
        }
    }
}
template <typename Dst, typename Src>
SKR_INLINE void move_assign(Dst* dst, Src* src, size_t count)
{
    if (dst != src)
    {
        if constexpr (memory_traits<Dst, Src>::use_move_assign)
        {
            if (dst < src)
            {
                while (count)
                {
                    *dst = std::move(*src);
                    if constexpr (memory_traits<Dst, Src>::need_dtor_after_move)
                    {
                        destruct(src);
                    }

                    ++dst;
                    ++src;
                    --count;
                }
            }
            else if (dst > src)
            {
                auto dst_end = dst + count;
                auto src_end = src + count;

                while (count)
                {
                    *dst_end = std::move(*src_end);
                    if constexpr (memory_traits<Dst, Src>::need_dtor_after_move)
                    {
                        destruct(src_end);
                    }

                    --dst_end;
                    --src_end;
                    --count;
                }
            }
        }
        else
        {
            std::memmove(dst, src, sizeof(Src) * count);
        }
    }
}

// compare
template <typename A, typename B>
SKR_INLINE bool compare(const A* a, const B* b, size_t count)
{
    if constexpr (memory_traits<A, B>::use_compare)
    {
        while (count)
        {
            if (*a != *b)
            {
                return false;
            }

            ++a;
            ++b;
            --count;
        }
        return true;
    }
    else
    {
        return !memcmp(a, b, sizeof(B) * count);
    }
}

} // namespace skr::memory