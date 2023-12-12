#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/misc/debug.h"

// TODO. allocator 行为规范
// allocator 需要关注 move 行为（主要是 pmr）来防止非法的内存转移
// 但是 allocator 不需要关注 copy 行为，因为 copy 不会引起非法的内存转移
// allocator 在 move assign 时不应该将自身状态 move 过去，只是检查内存转移是否合法
// 在 copy/move 构造中，应当提供有参与无参版本，其中有参版本不应赋予默认值
// TODO. 空容器下的 API 安全性试验
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
    SKR_ASSERT(expect_size < current_capacity);

    TS result;
    if ((3 * expect_size < 2 * current_capacity) && (current_capacity - expect_size > 64 || !expect_size))
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