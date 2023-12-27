#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/bit_array/bit_iterator.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_def.hpp"

// helper function
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS>
inline void copy_sparse_array_data(SparseArrayData<T, TS>* dst, const SparseArrayData<T, TS>* src, const TBitBlock* src_bit_array, TS size) noexcept
{
    using BitAlgo     = algo::BitAlgo<TBitBlock>;
    using StorageType = SparseArrayData<T, TS>;

    // copy data
    if constexpr (memory::MemoryTraits<T>::use_ctor)
    {
        for (TS i = 0; i < size; ++i)
        {
            StorageType*       p_dst_data = dst + i;
            const StorageType* p_src_data = src + i;

            if (BitAlgo::get(src_bit_array, i))
            {
                memory::copy(&p_dst_data->_sparse_array_data, &p_src_data->_sparse_array_data);
            }
            else
            {
                p_dst_data->_sparse_array_freelist_prev = p_src_data->_sparse_array_freelist_prev;
                p_dst_data->_sparse_array_freelist_next = p_src_data->_sparse_array_freelist_next;
            }
        }
    }
    else
    {
        std::memcpy(dst, src, sizeof(StorageType) * size);
    }
}
template <typename T, typename TBitBlock, typename TS>
inline void move_sparse_array_data(SparseArrayData<T, TS>* dst, SparseArrayData<T, TS>* src, const TBitBlock* src_bit_array, TS size) noexcept
{
    using BitAlgo     = algo::BitAlgo<TBitBlock>;
    using StorageType = SparseArrayData<T, TS>;

    // move data
    if constexpr (memory::MemoryTraits<T>::use_move)
    {
        for (TS i = 0; i < size; ++i)
        {
            StorageType* p_dst_data = dst + i;
            StorageType* p_src_data = src + i;
            if (BitAlgo::get(src_bit_array, i))
            {
                memory::move(&p_dst_data->_sparse_array_data, &p_src_data->_sparse_array_data);
            }
            else
            {
                p_dst_data->_sparse_array_freelist_prev = p_src_data->_sparse_array_freelist_prev;
                p_dst_data->_sparse_array_freelist_next = p_src_data->_sparse_array_freelist_next;
            }
        }
    }
    else
    {
        std::memmove(dst, src, sizeof(StorageType) * size);
    }
}
template <typename TBitBlock, typename TS>
inline void copy_sparse_array_bit_array(TBitBlock* dst, const TBitBlock* src, TS size) noexcept
{
    using BitAlgo = algo::BitAlgo<TBitBlock>;
    std::memcpy(dst, src, sizeof(TBitBlock) * BitAlgo::num_blocks(size));
}
template <typename TBitBlock, typename TS>
inline void move_sparse_array_bit_array(TBitBlock* dst, TBitBlock* src, TS size) noexcept
{
    using BitAlgo       = algo::BitAlgo<TBitBlock>;
    const TS byte_count = sizeof(TBitBlock) * BitAlgo::num_blocks(size);
    std::memcpy(dst, src, byte_count);
}
template <typename T, typename TBitBlock, typename TS>
inline void destruct_sparse_array_data(SparseArrayData<T, TS>* data, const TBitBlock* bit_array, TS size) noexcept
{
    if constexpr (memory::MemoryTraits<T>::use_dtor)
    {
        TrueBitIt<TBitBlock, TS, true> it(bit_array, size);
        for (; it; ++it)
        {
            memory::destruct<T>(&data[it.index()]._sparse_array_data);
        }
    }
}
} // namespace skr::container