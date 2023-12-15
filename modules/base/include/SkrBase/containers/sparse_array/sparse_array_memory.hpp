#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/bit_algo.hpp"
#include "SkrBase/containers/bit_array/bit_iterator.hpp"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_def.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include <algorithm>

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

// util sparse array memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, typename Allocator>
struct SparseArrayMemory : public Allocator {
    // configure
    using SizeType           = TS;
    using DataType           = T;
    using StorageType        = SparseArrayData<T, SizeType>;
    using BitBlockType       = TBitBlock;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline SparseArrayMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~SparseArrayMemory() noexcept
    {
        free();
    }

    // copy & move
    inline SparseArrayMemory(const SparseArrayMemory& other, AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
        if (other._sparse_size)
        {
            // reserve data
            realloc(other._sparse_size);

            // copy data
            copy_sparse_array_data(_data, other._data, other._bit_array, other._sparse_size);
            copy_sparse_array_bit_array(_bit_array, other._bit_array, other._sparse_size);
            _sparse_size   = other._sparse_size;
            _freelist_head = other._freelist_head;
            _hole_size     = other._hole_size;
        }
    }
    inline SparseArrayMemory(SparseArrayMemory&& other) noexcept
        : Allocator(std::move(other))
        , _data(other._data)
        , _bit_array(other._bit_array)
        , _sparse_size(other._sparse_size)
        , _capacity(other._capacity)
        , _bit_array_size(other._bit_array_size)
        , _freelist_head(other._freelist_head)
        , _hole_size(other._hole_size)
    {
        other._data           = nullptr;
        other._bit_array      = nullptr;
        other._sparse_size    = 0;
        other._capacity       = 0;
        other._bit_array_size = 0;
        other._freelist_head  = npos;
        other._hole_size      = 0;
    }

    // assign & move assign
    inline void operator=(const SparseArrayMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy allocator
            Allocator::operator=(rhs);

            // copy data
            if ((rhs._sparse_size - rhs._hole_size) > 0)
            {
                // reserve data
                if (_capacity < rhs._sparse_size)
                {
                    realloc(rhs._sparse_size);
                }

                // copy data
                copy_sparse_array_data(_data, rhs._data, rhs._bit_array, rhs._sparse_size);
                copy_sparse_array_bit_array(_bit_array, rhs._bit_array, rhs._sparse_size);
                _sparse_size    = rhs._sparse_size;
                _capacity       = rhs._capacity;
                _bit_array_size = rhs._bit_array_size;
                _freelist_head  = rhs._freelist_head;
                _hole_size      = rhs._hole_size;
            }
        }
    }
    inline void operator=(SparseArrayMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // free
            free();

            // move allocator
            Allocator::operator=(std::move(rhs));

            // move data
            _data               = rhs._data;
            _bit_array          = rhs._bit_array;
            _sparse_size        = rhs._sparse_size;
            _capacity           = rhs._capacity;
            _bit_array_size     = rhs._bit_array_size;
            _freelist_head      = rhs._freelist_head;
            _hole_size          = rhs._hole_size;
            rhs._data           = nullptr;
            rhs._bit_array      = nullptr;
            rhs._sparse_size    = 0;
            rhs._capacity       = 0;
            rhs._bit_array_size = 0;
            rhs._freelist_head  = npos;
            rhs._hole_size      = 0;
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_sparse_size <= new_capacity);
        SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

        // realloc data array
        if constexpr (memory::MemoryTraits<DataType>::use_realloc && Allocator::support_realloc)
        {
            _data     = Allocator::template realloc<StorageType>(_data, new_capacity);
            _capacity = new_capacity;
        }
        else
        {
            // alloc new memory
            StorageType* new_memory = Allocator::template alloc<StorageType>(new_capacity);

            // move items
            if (_sparse_size)
            {
                move_sparse_array_data(new_memory, _data, _bit_array, _sparse_size);
            }

            // release old memory
            Allocator::template free<StorageType>(_data);

            // update data
            _data     = new_memory;
            _capacity = new_capacity;
        }

        // realloc bit array
        SizeType new_block_size = BitAlgo::num_blocks(_capacity);
        SizeType old_block_size = BitAlgo::num_blocks(_bit_array_size);

        if (new_block_size != old_block_size)
        {
            if constexpr (memory::MemoryTraits<BitBlockType>::use_realloc && Allocator::support_realloc)
            {
                _bit_array      = Allocator::template realloc<BitBlockType>(_bit_array, new_block_size);
                _bit_array_size = new_block_size * BitAlgo::PerBlockSize;
            }
            else
            {
                // alloc new memory
                BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_size);

                // move data
                if (old_block_size)
                {
                    memory::move(new_memory, _bit_array, std::min(new_block_size, old_block_size));
                }

                // release old memory
                Allocator::template free<BitBlockType>(_bit_array);

                // update data
                _bit_array      = new_memory;
                _bit_array_size = new_block_size * BitAlgo::PerBlockSize;
            }
        }

        // clean up new bit array memory
        if (new_block_size > old_block_size)
        {
            memset(_bit_array + old_block_size, 0, (new_block_size - old_block_size) * sizeof(BitBlockType));
        }
    }
    inline void free() noexcept
    {
        if (_data)
        {
            // destruct items
            destruct_sparse_array_data(_data, _bit_array, _sparse_size);

            // release memory
            Allocator::template free<StorageType>(_data);
            Allocator::template free<BitBlockType>(_bit_array);

            // reset data
            _data           = nullptr;
            _bit_array      = nullptr;
            _sparse_size    = 0;
            _capacity       = 0;
            _bit_array_size = 0;
            _freelist_head  = npos;
            _hole_size      = 0;
        }
    }
    inline void grow(SizeType grow_size) noexcept
    {
        SizeType new_sparse_size = _sparse_size + grow_size;
        if (new_sparse_size > _capacity)
        {
            auto new_capacity = default_get_grow<DataType>(new_sparse_size, _capacity);
            SKR_ASSERT(new_capacity >= _capacity);
            if (new_capacity > _capacity)
            {
                realloc(new_capacity);
            }
        }

        _sparse_size = new_sparse_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<DataType>(_sparse_size, _capacity);
        SKR_ASSERT(new_capacity >= _sparse_size);
        if (new_capacity < _capacity)
        {
            realloc(new_capacity);
        }
    }
    inline void clear() noexcept
    {
        if (_sparse_size)
        {
            // destruct items
            if constexpr (memory::MemoryTraits<DataType>::use_dtor)
            {
                TrueBitIt<TBitBlock, SizeType, false> it(_bit_array, _sparse_size);
                for (; it; ++it)
                {
                    memory::destruct<DataType>(&_data[it.index()]._sparse_array_data);
                }
            }

            // clean up bit array
            if (_bit_array)
            {
                BitAlgo::set_blocks(_bit_array, SizeType(0), BitAlgo::num_blocks(_sparse_size), false);
            }

            // clean up data
            _hole_size     = 0;
            _sparse_size   = 0;
            _freelist_head = npos;
        }
    }

    // getter
    inline StorageType*        data() noexcept { return _data; }
    inline const StorageType*  data() const noexcept { return _data; }
    inline BitBlockType*       bit_array() noexcept { return _bit_array; }
    inline const BitBlockType* bit_array() const noexcept { return _bit_array; }
    inline SizeType            sparse_size() const noexcept { return _sparse_size; }
    inline SizeType            capacity() const noexcept { return _capacity; }
    inline SizeType            bit_array_size() const noexcept { return _bit_array_size; }
    inline SizeType            freelist_head() const noexcept { return _freelist_head; }
    inline SizeType            hole_size() const noexcept { return _hole_size; }

    // setter
    inline void set_sparse_size(SizeType value) noexcept { _sparse_size = value; }
    inline void set_freelist_head(SizeType value) noexcept { _freelist_head = value; }
    inline void set_hole_size(SizeType value) noexcept { _hole_size = value; }

private:
    // algo
    using BitAlgo                         = algo::BitAlgo<TBitBlock>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

private:
    StorageType*  _data           = nullptr;
    BitBlockType* _bit_array      = nullptr;
    SizeType      _sparse_size    = 0;
    SizeType      _capacity       = 0;
    SizeType      _bit_array_size = 0;
    SizeType      _freelist_head  = npos;
    SizeType      _hole_size      = 0;
};
} // namespace skr::container
