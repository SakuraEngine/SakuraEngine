#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/bit_tools/bit_iterator.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_def.hpp"

// helper function
namespace skr::container
{
template <typename T, typename TBitBlock, typename TSize>
inline void copy_sparse_vector_data(SparseVectorStorage<T, TSize>* dst, const SparseVectorStorage<T, TSize>* src, const TBitBlock* src_bit_data, TSize size) noexcept
{
    using BitAlgo     = algo::BitAlgo<TBitBlock>;
    using StorageType = SparseVectorStorage<T, TSize>;

    // copy data
    if constexpr (::skr::memory::MemoryTraits<T>::use_copy)
    {
        for (TSize i = 0; i < size; ++i)
        {
            StorageType*       p_dst_data = dst + i;
            const StorageType* p_src_data = src + i;

            if (BitAlgo::get(src_bit_data, i))
            {
                ::skr::memory::copy(&p_dst_data->_sparse_vector_data, &p_src_data->_sparse_vector_data);
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
template <typename T, typename TBitBlock, typename TSize>
inline void move_sparse_vector_data(SparseVectorStorage<T, TSize>* dst, SparseVectorStorage<T, TSize>* src, const TBitBlock* src_bit_data, TSize size) noexcept
{
    using BitAlgo     = algo::BitAlgo<TBitBlock>;
    using StorageType = SparseVectorStorage<T, TSize>;

    // move data
    if constexpr (::skr::memory::MemoryTraits<T>::use_move)
    {
        for (TSize i = 0; i < size; ++i)
        {
            StorageType* p_dst_data = dst + i;
            StorageType* p_src_data = src + i;
            if (BitAlgo::get(src_bit_data, i))
            {
                ::skr::memory::move(&p_dst_data->_sparse_vector_data, &p_src_data->_sparse_vector_data);
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
template <typename TBitBlock, typename TSize>
inline void copy_sparse_vector_bit_data(TBitBlock* dst, const TBitBlock* src, TSize size) noexcept
{
    using BitAlgo = algo::BitAlgo<TBitBlock>;
    std::memcpy(dst, src, sizeof(TBitBlock) * BitAlgo::num_blocks(size));
}
template <typename TBitBlock, typename TSize>
inline void move_sparse_vector_bit_data(TBitBlock* dst, TBitBlock* src, TSize size) noexcept
{
    using BitAlgo          = algo::BitAlgo<TBitBlock>;
    const TSize byte_count = sizeof(TBitBlock) * BitAlgo::num_blocks(size);
    std::memcpy(dst, src, byte_count);
}
template <typename T, typename TBitBlock, typename TSize>
inline void destruct_sparse_vector_data(SparseVectorStorage<T, TSize>* data, const TBitBlock* bit_data, TSize size) noexcept
{
    if constexpr (::skr::memory::MemoryTraits<T>::use_dtor)
    {
        auto cursor = TrueBitCursor<TBitBlock, TSize, true>::Begin(bit_data, size);

        while (!cursor.reach_end())
        {
            ::skr::memory::destruct<T>(&data[cursor.index()]._sparse_vector_data);
            cursor.move_next();
        }
    }
}
} // namespace skr::container