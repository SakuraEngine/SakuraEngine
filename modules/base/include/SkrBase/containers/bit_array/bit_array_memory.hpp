#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/memory/memory_ops.hpp"
#include "SkrBase/algo/bit_algo.hpp"

// util bit array memory
namespace skr::container
{
template <typename TBitBlock, typename TS, typename Allocator>
struct BitArrayMemory : public Allocator {
    using BitBlockType       = TBitBlock;
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline BitArrayMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~BitArrayMemory() noexcept
    {
        free();
    }

    // copy & move ctor
    inline BitArrayMemory(const BitArrayMemory& other) noexcept
        : Allocator(other)
    {
        if (other._size)
        {
            // alloc
            realloc(Algo::num_blocks(other._size));

            // copy
            memory::copy(_data, other._data, Algo::num_blocks(other._size));
            _size = other._size;
        }
    }
    inline BitArrayMemory(BitArrayMemory&& other) noexcept
        : Allocator(std::move(other))
        , _data(other._data)
        , _size(other._size)
        , _capacity(other._capacity)
    {
        other._data     = nullptr;
        other._size     = 0;
        other._capacity = 0;
    }

    // assign & move assign
    inline void operator=(const BitArrayMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy allocator
            Allocator::operator=(rhs);

            // copy data
            if (rhs._size)
            {
                // reserve memory
                if (_capacity < rhs._size)
                {
                    realloc(Algo::num_blocks(rhs._size));
                }

                // copy
                memory::copy(_data, rhs._data, Algo::num_blocks(rhs._size));
                _size = rhs._size;
            }
        }
    }
    inline void operator=(BitArrayMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();
            free();

            // move allocator
            Allocator::operator=(std::move(rhs));

            // move data
            _data         = rhs._data;
            _size         = rhs._size;
            _capacity     = rhs._capacity;
            rhs._data     = nullptr;
            rhs._size     = 0;
            rhs._capacity = 0;
        }
    }

    // memory operations
    inline void realloc(SizeType new_block_capacity) noexcept
    {
        SizeType old_block_capacity = Algo::num_blocks(_capacity);
        SKR_ASSERT(new_block_capacity != old_block_capacity);
        SKR_ASSERT(new_block_capacity > 0);
        SKR_ASSERT(_size <= new_block_capacity * Algo::PerBlockSize);
        SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

        SKR_ASSERT(new_block_capacity > 0);

        if (new_block_capacity != old_block_capacity)
        {
            if constexpr (memory::MemoryTraits<BitBlockType>::use_realloc && Allocator::support_realloc)
            {
                _data     = Allocator::template realloc<BitBlockType>(_data, new_block_capacity);
                _capacity = new_block_capacity * Algo::PerBlockSize;
            }
            else
            {
                // alloc new memory
                BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_capacity);

                // move items
                if (_size)
                {
                    memory::move<BitBlockType>(new_memory, _data, Algo::num_blocks(_size));
                }

                // release old memory
                Allocator::template free<BitBlockType>(_data);

                _data     = new_memory;
                _capacity = new_block_capacity * Algo::PerBlockSize;
            }
        }
    }
    inline void free() noexcept
    {
        if (_data)
        {
            Allocator::template free<BitBlockType>(_data);
            _data     = nullptr;
            _size     = 0;
            _capacity = 0;
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size = _size;
        SizeType new_size = old_size + grow_size;

        if (new_size > _capacity)
        {
            // as small as better
            SizeType new_capacity = default_get_grow<char>(new_size, _capacity);
            SKR_ASSERT(new_capacity > _capacity);
            if (new_capacity >= _capacity)
            {
                realloc(Algo::num_blocks(new_capacity));
            }
        }

        _size = new_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<char>(_size, _capacity);
        SKR_ASSERT(new_capacity >= _size);
        if (new_capacity < _capacity)
        {
            if (new_capacity)
            {
                SizeType new_block_capacity = Algo::num_blocks(new_capacity);
                if (new_block_capacity != Algo::num_blocks(_capacity))
                {
                    realloc(new_block_capacity);
                }
            }
            else
            {
                free();
            }
        }
    }
    inline void clear() noexcept
    {
        Algo::set_blocks(_data, SizeType(0), Algo::num_blocks(_size), false);
        _size = 0;
    }

    // getter
    inline BitBlockType*       data() noexcept { return _data; }
    inline const BitBlockType* data() const noexcept { return _data; }
    inline SizeType            size() const noexcept { return _size; }
    inline SizeType            capacity() const noexcept { return _capacity; }

    // setter
    inline void set_size(SizeType value) noexcept { _size = value; }

private:
    using Algo = algo::BitAlgo<TBitBlock>;

private:
    BitBlockType* _data     = nullptr;
    SizeType      _size     = 0;
    SizeType      _capacity = 0;
};
}; // namespace skr::container