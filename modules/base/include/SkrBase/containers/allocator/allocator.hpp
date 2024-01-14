#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"

// 关于带状态 Allocator
//  copy constructor 行为应当复制内存来源，其语义是内存来源的增殖
//  move constructor 行为迎丹复制内存来源，其语义是内存来源的增殖
//  assign 行为只是检查容器内存是否能够正确的被复制，其语义是检查
//  move assign 行为只是检查容器内存是否能够被直接迁移（迁移后能否被正确销毁），其语义是检查
namespace skr::container
{

template <typename TDerived, typename TS>
struct AllocTemplate {
    using SizeType = TS;

    // impl it
    // SKR_INLINE void  free_raw(void* p, SizeType align) const;
    // SKR_INLINE void* alloc_raw(SizeType size, SizeType align) const;
    // SKR_INLINE void* realloc_raw(void* p, SizeType size, SizeType align) const;

    // helper
    template <typename T>
    SKR_INLINE void free(T* p) const
    {
        static_cast<const TDerived*>(this)->free_raw(p, alignof(T));
    }
    template <typename T>
    SKR_INLINE T* alloc(SizeType size) const
    {
        return (T*)static_cast<const TDerived*>(this)->alloc_raw(size * sizeof(T),
                                                                 alignof(T));
    }
    template <typename T>
    SKR_INLINE T* realloc(T* p, SizeType size) const
    {
        return (T*)static_cast<const TDerived*>(this)->realloc_raw(
        p, size * sizeof(T), alignof(T));
    }

    // size > capacity, calc grow
    SKR_INLINE SizeType get_grow(SizeType size, SizeType capacity) const
    {
        constexpr SizeType first_grow    = 4;
        constexpr SizeType constant_grow = 16;

        SKR_ASSERT(size > capacity && size > 0);

        // init data
        SizeType result = first_grow;

        // calc grow
        if (capacity || size > first_grow)
        {
            result = size + 3 * size / 8 + constant_grow;
        }

        // handle num over flow
        if (size > result)
            result = std::numeric_limits<SizeType>::max();

        return result;
    }
    // size < capacity, calc shrink
    SKR_INLINE SizeType get_shrink(SizeType size, SizeType capacity) const
    {
        SKR_ASSERT(size < capacity);

        SizeType result;
        if ((3 * size < 2 * capacity) && (capacity - size > 64 || !size))
        {
            result = size;
        }
        else
        {
            result = capacity;
        }

        return result;
    }
};

} // namespace skr::container

namespace skr::container
{
template <typename T, typename TS>
TS default_get_grow(TS expect_size, TS current_capacity);
template <typename T, typename TS>
TS default_get_shrink(TS expect_size, TS current_capacity);
} // namespace skr::container

namespace skr::container
{
template <typename T, typename TS>
inline TS default_get_grow(TS expect_size, TS current_capacity)
{
    constexpr TS first_grow    = 4;
    constexpr TS constant_grow = 16;

    SKR_ASSERT(expect_size > current_capacity && expect_size > 0);

    // init data
    TS result = first_grow;

    // calc grow
    if (current_capacity || expect_size > first_grow)
    {
        result = expect_size + 3 * expect_size / 8 + constant_grow;
    }

    // handle num over flow
    if (expect_size > result)
        result = std::numeric_limits<TS>::max();

    return result;
}
template <typename T, typename TS>
inline TS default_get_shrink(TS expect_size, TS current_capacity)
{
    SKR_ASSERT(expect_size <= current_capacity);

    TS result;
    if (((3 * expect_size) < (2 * current_capacity)) &&
        ((current_capacity - expect_size) > 64 || !expect_size))
    {
        result = expect_size;
    }
    else
    {
        result = current_capacity;
    }

    return result;
}
}; // namespace skr::container