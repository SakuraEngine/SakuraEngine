#pragma once
#include "SkrBase/algo/bit_algo.hpp"
#include "SkrBase/config.h"
#include "SkrBase/containers/bit_array/bit_iterator.hpp"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_def.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"

// helper function
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS>
inline void copy_sparse_array_data(SparseArrayData<T, TS>* dst, const SparseArrayData<T, TS>* src, const TBitBlock* src_bit_array, TS size) noexcept
{
    using BitAlgo  = algo::BitAlgo<TBitBlock>;
    using DataType = SparseArrayData<T, TS>;

    // copy data
    if constexpr (memory::MemoryTraits<T>::use_ctor)
    {
        for (TS i = 0; i < size; ++i)
        {
            DataType*       p_dst_data = dst + i;
            const DataType* p_src_data = src + i;

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
        std::memcpy(dst, src, sizeof(DataType) * size);
    }
}
template <typename T, typename TBitBlock, typename TS>
inline void move_sparse_array_data(SparseArrayData<T, TS>* dst, SparseArrayData<T, TS>* src, const TBitBlock* src_bit_array, TS size) noexcept
{
    using BitAlgo  = algo::BitAlgo<TBitBlock>;
    using DataType = SparseArrayData<T, TS>;

    // move data
    if constexpr (memory::MemoryTraits<T>::use_move)
    {
        for (TS i = 0; i < size; ++i)
        {
            DataType* p_dst_data = dst + i;
            DataType* p_src_data = src + i;
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
        std::memmove(dst, src, sizeof(DataType) * size);
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
        TrueBitIt<TBitBlock, TS, false> it(bit_array, size);
        for (; it; ++it)
        {
            memory::destruct<T>(data + it.index());
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
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // helper data
    using BitBlockType                    = TBitBlock;
    using DataType                        = SparseArrayData<T, SizeType>;
    using BitAlgo                         = algo::BitAlgo<TBitBlock>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // ctor & dtor
    inline SparseArrayMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~SparseArrayMemory() noexcept
    {
        if (_data)
        {
            free();
        }
    }

    // copy & move
    inline SparseArrayMemory(const SparseArrayMemory& other, AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
        if (other._capacity)
        {
            // reserve data
            realloc(other._capacity);

            // copy data
            copy_sparse_array_data(_data, other._data, other._bit_array, other._sparse_size);
            copy_sparse_array_bit_array(_bit_array, other._bit_array, other._sparse_size);
            _sparse_size    = other._sparse_size;
            _capacity       = other._capacity;
            _bit_array_size = other._bit_array_size;
            _freelist_head  = other._freelist_head;
            _hole_size      = other._hole_size;
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
        SKR_ASSERT(this != &rhs && "before call operator=, you must check this != &rhs");
        SKR_ASSERT(_sparse_size == 0 && "before copy memory, you must clean up the memory first");

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
    inline void operator=(SparseArrayMemory&& rhs) noexcept
    {
        SKR_ASSERT(this != &rhs && "before call operator=, you must check this != &rhs");
        SKR_ASSERT(_sparse_size == 0 && "before move memory, you must clean up the memory first");

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

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_sparse_size <= new_capacity);
        SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

        // realloc data array
        if constexpr (memory::MemoryTraits<T>::use_realloc && Allocator::support_realloc)
        {
            _data     = Allocator::template realloc<DataType>(_data, new_capacity);
            _capacity = new_capacity;
        }
        else
        {
            // alloc new memory
            DataType* new_memory = Allocator::template alloc<DataType>(new_capacity);

            // move items
            if (_sparse_size)
            {
                move_sparse_array_data(new_capacity, _data, _bit_array, _sparse_size);
            }

            // release old memory
            Allocator::template free<DataType>(_data);

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
            Allocator::template free<DataType>(_data);
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
            auto new_capacity = default_get_grow<T>(new_sparse_size, _capacity);
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
        SizeType new_capacity = default_get_shrink<T>(_sparse_size, _capacity);
        SKR_ASSERT(new_capacity >= _sparse_size);
        if (new_capacity < _capacity)
        {
            realloc(new_capacity);
        }
    }

    // getter
    inline T*                  data() noexcept { return _data; }
    inline const T*            data() const noexcept { return _data; }
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
    DataType*     _data           = nullptr;
    BitBlockType* _bit_array      = nullptr;
    SizeType      _sparse_size    = 0;
    SizeType      _capacity       = 0;
    SizeType      _bit_array_size = 0;
    SizeType      _freelist_head  = npos;
    SizeType      _hole_size      = 0;
};
} // namespace skr::container
