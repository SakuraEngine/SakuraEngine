#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/bit_tools/bit_iterator.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_def.hpp"

// helper function
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS>
inline void copy_sparse_vector_data(SparseVectorData<T, TS>* dst, const SparseVectorData<T, TS>* src, const TBitBlock* src_bit_data, TS size) noexcept
{
    using BitAlgo     = algo::BitAlgo<TBitBlock>;
    using StorageType = SparseVectorData<T, TS>;

    // copy data
    if constexpr (memory::MemoryTraits<T>::use_ctor)
    {
        for (TS i = 0; i < size; ++i)
        {
            StorageType*       p_dst_data = dst + i;
            const StorageType* p_src_data = src + i;

            if (BitAlgo::get(src_bit_data, i))
            {
                memory::copy(&p_dst_data->_sparse_vector_data, &p_src_data->_sparse_vector_data);
            }
            else
            {
                p_dst_data->_sparse_vector_freelist_prev = p_src_data->_sparse_vector_freelist_prev;
                p_dst_data->_sparse_vector_freelist_next = p_src_data->_sparse_vector_freelist_next;
            }
        }
    }
    else
    {
        std::memcpy(dst, src, sizeof(StorageType) * size);
    }
}
template <typename T, typename TBitBlock, typename TS>
inline void move_sparse_vector_data(SparseVectorData<T, TS>* dst, SparseVectorData<T, TS>* src, const TBitBlock* src_bit_data, TS size) noexcept
{
    using BitAlgo     = algo::BitAlgo<TBitBlock>;
    using StorageType = SparseVectorData<T, TS>;

    // move data
    if constexpr (memory::MemoryTraits<T>::use_move)
    {
        for (TS i = 0; i < size; ++i)
        {
            StorageType* p_dst_data = dst + i;
            StorageType* p_src_data = src + i;
            if (BitAlgo::get(src_bit_data, i))
            {
                memory::move(&p_dst_data->_sparse_vector_data, &p_src_data->_sparse_vector_data);
            }
            else
            {
                p_dst_data->_sparse_vector_freelist_prev = p_src_data->_sparse_vector_freelist_prev;
                p_dst_data->_sparse_vector_freelist_next = p_src_data->_sparse_vector_freelist_next;
            }
        }
    }
    else
    {
        std::memmove(dst, src, sizeof(StorageType) * size);
    }
}
template <typename TBitBlock, typename TS>
inline void copy_sparse_vector_bit_data(TBitBlock* dst, const TBitBlock* src, TS size) noexcept
{
    using BitAlgo = algo::BitAlgo<TBitBlock>;
    std::memcpy(dst, src, sizeof(TBitBlock) * BitAlgo::num_blocks(size));
}
template <typename TBitBlock, typename TS>
inline void move_sparse_vector_bit_data(TBitBlock* dst, TBitBlock* src, TS size) noexcept
{
    using BitAlgo       = algo::BitAlgo<TBitBlock>;
    const TS byte_count = sizeof(TBitBlock) * BitAlgo::num_blocks(size);
    std::memcpy(dst, src, byte_count);
}
template <typename T, typename TBitBlock, typename TS>
inline void destruct_sparse_vector_data(SparseVectorData<T, TS>* data, const TBitBlock* bit_data, TS size) noexcept
{
    if constexpr (memory::MemoryTraits<T>::use_dtor)
    {
        auto cursor = TrueBitCursor<TBitBlock, TS, true>::Begin(bit_data, size);

        while (!cursor.reach_end())
        {
            memory::destruct<T>(&data[cursor.index()]._sparse_vector_data);
            cursor.move_next();
        }
    }
}
} // namespace skr::container