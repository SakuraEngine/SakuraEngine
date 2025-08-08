#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/misc/bit.hpp"
#include <limits>
#include <type_traits>
#include <memory>
#include "SkrBase/misc/debug.h"

namespace skr::algo
{
template <typename T>
struct BitAlgo {
private:
    static inline constexpr T _per_block_size_log2()
    {
        static_assert(PerBlockSize <= 512);

        if constexpr (PerBlockSize == 8)
        {
            return 3;
        }
        else if constexpr (PerBlockSize == 16)
        {
            return 4;
        }
        else if constexpr (PerBlockSize == 32)
        {
            return 5;
        }
        else if constexpr (PerBlockSize == 64)
        {
            return 6;
        }
        else if constexpr (PerBlockSize == 128)
        {
            return 7;
        }
        else if constexpr (PerBlockSize == 256)
        {
            return 8;
        }
        else if constexpr (PerBlockSize == 512)
        {
            return 9;
        }
        else
        {
            return static_cast<T>(-1);
        }
    }

public:
    static_assert(std::is_integral_v<T> && !std::is_signed_v<T>);
    using BlockType = T;

    // constant
    static inline constexpr T PerBlockSize     = std::numeric_limits<T>::digits;
    static inline constexpr T PerBlockSizeMask = PerBlockSize - 1;
    static inline constexpr T PerBlockSizeLog2 = _per_block_size_log2();
    static inline constexpr T EmptyMask        = 0;
    static inline constexpr T FullMask         = ~EmptyMask;

    // 存储 bit 需要的 block 数量
    template <typename TSize>
    static TSize num_blocks(TSize num_bits);
    // 如果从 index 开始搜索 bit 需要对 block 进行 mask
    // e.g. index = 3, 8 位，mask 为 11111000
    // 注意，存储计数从低位开始
    template <typename TSize>
    static T first_block_mask(TSize index);
    // 如果搜索到 num_bits 为止需要对 block 进行 mask
    // e.g. index = 11, 8 位，mask 位 00000111
    // 注意，存储计数从低位开始
    template <typename TSize>
    static T last_block_mask(TSize num_bits);

    // set & get
    static void set_block_masked(T& b, T mask, bool v);
    template <typename TSize>
    static void set_blocks(T* data, TSize start, TSize num, bool v);
    template <typename TSize>
    static void set_range(T* data, TSize start, TSize num, bool v);
    template <typename TSize>
    static void set(T* data, TSize index, bool v);
    template <typename TSize>
    static bool get(const T* data, TSize index);

    // find
    static T find_block_masked(T b, T mask, bool v);
    static T find_last_block_masked(T b, T mask, bool v);
    template <typename TSize>
    static TSize find(const T* data, TSize start, TSize num, bool v);
    template <typename TSize>
    static TSize find_last(const T* data, TSize start, TSize num, bool v);

    // find then flip
    static T find_flip_block_masked(T& b, T mask, bool v);
    static T find_flip_last_block_masked(T& b, T mask, bool v);
    template <typename TSize>
    static TSize find_flip(T* data, TSize start, TSize num, bool v);
    template <typename TSize>
    static TSize find_last_flip(T* data, TSize start, TSize num, bool v);
};
} // namespace skr::algo

// help functions
namespace skr::algo
{
template <typename T>
template <typename TSize>
SKR_INLINE TSize BitAlgo<T>::num_blocks(TSize num_bits)
{
    SKR_ASSERT(num_bits >= 0);
    return int_div_ceil(num_bits, static_cast<TSize>(PerBlockSize));
}
template <typename T>
template <typename TSize>
SKR_INLINE T BitAlgo<T>::first_block_mask(TSize index)
{
    SKR_ASSERT(index >= 0);
    return FullMask << (index & static_cast<TSize>(PerBlockSizeMask));
}
template <typename T>
template <typename TSize>
SKR_INLINE T BitAlgo<T>::last_block_mask(TSize num_bits)
{
    SKR_ASSERT(num_bits >= 0);

    // 两次 & 是为了防止 num_bits 为 0 的情况
    return FullMask >> ((static_cast<TSize>(PerBlockSize) - num_bits & static_cast<TSize>(PerBlockSizeMask)) & static_cast<TSize>(PerBlockSizeMask));
}
} // namespace skr::algo

// set
namespace skr::algo
{
template <typename T>
SKR_INLINE void BitAlgo<T>::set_block_masked(T& b, T mask, bool v) { b = v ? b | mask : b & ~mask; }
template <typename T>
template <typename TSize>
SKR_INLINE void BitAlgo<T>::set_blocks(T* data, TSize start, TSize num, bool v)
{
    if (num == 0)
    {
        return;
    }
    SKR_ASSERT(data != nullptr);

    data += start;

    if (num > 8)
    {
        std::memset(data, v ? 0xff : 0, num * sizeof(T));
    }
    else
    {
        T block = v ? FullMask : EmptyMask;
        for (TSize i = 0; i < num; ++i)
        {
            data[i] = block;
        }
    }
}
template <typename T>
template <typename TSize>
SKR_INLINE void BitAlgo<T>::set_range(T* data, TSize start, TSize num, bool v)
{
    if (num == 0)
    {
        return;
    }
    SKR_ASSERT(data != nullptr);

    // calculate block index and count
    TSize block_start = start >> PerBlockSizeLog2;
    TSize block_count = int_div_ceil(start + num, static_cast<TSize>(PerBlockSize)) - block_start;

    // calculate mask
    TSize end        = start + num;
    T     start_mask = first_block_mask(start);
    T     end_mask   = last_block_mask(end);

    T& block_v_start = *(data + block_start);
    T& block_v_end   = *(data + block_start + block_count - 1);
    if (block_count == 1)
    {
        T mask = start_mask & end_mask;
        set_block_masked(block_v_start, mask, v);
    }
    else
    {
        // 掐头去尾
        set_block_masked(block_v_start, start_mask, v);
        set_block_masked(block_v_end, end_mask, v);

        // 设中间
        set_blocks(data, block_start + 1, block_count - 2, v);
    }
}
template <typename T>
template <typename TSize>
SKR_INLINE void BitAlgo<T>::set(T* data, TSize index, bool v)
{
    T& block = data[index >> PerBlockSizeLog2];
    T  mask  = T(1) << (index & static_cast<TSize>(PerBlockSizeMask));
    set_block_masked(block, mask, v);
}
template <typename T>
template <typename TSize>
SKR_INLINE bool BitAlgo<T>::get(const T* data, TSize index)
{
    return data[index >> PerBlockSizeLog2] & (T(1) << (index & static_cast<TSize>(PerBlockSizeMask)));
}
} // namespace skr::algo

// find
namespace skr::algo
{
template <typename T>
SKR_INLINE T BitAlgo<T>::find_block_masked(T b, T mask, bool v)
{
    b = v ? b & mask : ~b & mask;
    return b ? countr_zero(b) : npos_of<T>;
}
template <typename T>
SKR_INLINE T BitAlgo<T>::find_last_block_masked(T b, T mask, bool v)
{
    b = v ? b & mask : ~b & mask;
    return b ? PerBlockSize - countl_zero(b) - 1 : npos_of<T>;
}
template <typename T>
template <typename TSize>
SKR_INLINE TSize BitAlgo<T>::find(const T* data, TSize start, TSize num, bool v)
{
    if (num == 0)
    {
        return npos_of<TSize>;
    }
    SKR_ASSERT(data != nullptr);

    // calculate block index and count
    TSize block_start = start >> PerBlockSizeLog2;
    TSize block_count = int_div_ceil(start + num, static_cast<TSize>(PerBlockSize)) - block_start;

    // calculate mask
    TSize end        = start + num;
    T     start_mask = first_block_mask(start);
    T     end_mask   = last_block_mask(end);

    if (block_count == 1)
    {
        T idx = find_block_masked(data[block_start], start_mask & end_mask, v);
        return idx == npos_of<T> ? npos_of<TSize> : static_cast<TSize>(idx) + (block_start << PerBlockSizeLog2);
    }
    else
    {
        // head
        {
            T idx = find_block_masked(data[block_start], start_mask, v);
            if (idx != npos_of<T>)
            {
                return static_cast<TSize>(idx) + (block_start << PerBlockSizeLog2);
            }
        }

        // center
        const T test = v ? EmptyMask : FullMask;
        for (TSize i = 1; i < block_count - 1; ++i)
        {
            TSize block_idx   = block_start + i;
            T     block_value = data[block_idx];
            if (block_value != test)
            {
                T idx = find_block_masked(block_value, FullMask, v);
                return static_cast<TSize>(idx) + (block_idx << PerBlockSizeLog2);
            }
        }

        // tail
        {
            TSize block_last = block_start + block_count - 1;
            T     idx        = find_block_masked(data[block_last], end_mask, v);
            return idx == npos_of<T> ? npos_of<TSize> : static_cast<TSize>(idx) + (block_last << PerBlockSizeLog2);
        }
    }
}
template <typename T>
template <typename TSize>
SKR_INLINE TSize BitAlgo<T>::find_last(const T* data, TSize start, TSize num, bool v)
{
    if (num == 0)
    {
        return npos_of<TSize>;
    }
    SKR_ASSERT(data != nullptr);

    // calculate block index and count
    TSize block_start = start >> PerBlockSizeLog2;
    TSize block_count = int_div_ceil(start + num, static_cast<TSize>(PerBlockSize)) - block_start;

    // calculate mask
    TSize end        = start + num;
    T     start_mask = first_block_mask(start);
    T     end_mask   = last_block_mask(end);

    if (block_count == 1)
    {
        T idx = find_last_block_masked(data[block_start], start_mask & end_mask, v);
        return idx == npos_of<T> ? npos_of<TSize> : static_cast<TSize>(idx) + (block_start << PerBlockSizeLog2);
    }
    else
    {
        // tail
        {
            TSize block_last = block_start + block_count - 1;
            T     idx        = find_last_block_masked(data[block_last], end_mask, v);
            if (idx != npos_of<T>)
            {
                return static_cast<TSize>(idx) + (block_last << PerBlockSizeLog2);
            }
        }

        // center
        const T test = v ? EmptyMask : FullMask;
        for (TSize i = block_count - 2; i >= 1; --i)
        {
            TSize block_idx   = block_start + i;
            T     block_value = data[block_idx];
            if (block_value != test)
            {
                T idx = find_last_block_masked(block_value, FullMask, v);
                return static_cast<TSize>(idx) + (block_idx << PerBlockSizeLog2);
            }
        }

        // head
        {
            T idx = find_last_block_masked(data[block_start], start_mask, v);
            return idx == npos_of<T> ? npos_of<TSize> : static_cast<TSize>(idx) + (block_start << PerBlockSizeLog2);
        }
    }
}
} // namespace skr::algo

// find then flip
namespace skr::algo
{
template <typename T>
SKR_INLINE T BitAlgo<T>::find_flip_block_masked(T& b, T mask, bool v)
{
    T idx = find_block_masked(b, mask, v);
    if (idx != npos_of<T>)
    {
        T set_mask = T(1) << (idx & static_cast<T>(PerBlockSizeMask));
        b          = v ? b & ~set_mask : b | set_mask;
    }
    return idx;
}
template <typename T>
SKR_INLINE T BitAlgo<T>::find_flip_last_block_masked(T& b, T mask, bool v)
{
    T idx = find_last_block_masked(b, mask, v);
    if (idx != npos_of<T>)
    {
        T set_mask = T(1) << (idx & static_cast<T>(PerBlockSizeMask));
        b          = v ? b & ~set_mask : b | set_mask;
    }
    return idx;
}
template <typename T>
template <typename TSize>
SKR_INLINE TSize BitAlgo<T>::find_flip(T* data, TSize start, TSize num, bool v)
{
    TSize idx = find(data, start, num, v);
    if (idx != npos_of<TSize>)
    {
        set(data, idx, !v);
    }
    return idx;
}
template <typename T>
template <typename TSize>
SKR_INLINE TSize BitAlgo<T>::find_last_flip(T* data, TSize start, TSize num, bool v)
{
    TSize idx = find_last(data, start, num, v);
    if (idx != npos_of<TSize>)
    {
        set(data, idx, !v);
    }
    return idx;
}
} // namespace skr::algo