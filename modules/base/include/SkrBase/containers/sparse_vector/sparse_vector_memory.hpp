#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/bit_algo.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_def.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_helper.hpp"
#include <algorithm>

// util sparse vector memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, typename Allocator>
struct SparseVectorMemory : public Allocator {
    // configure
    using SizeType           = TS;
    using DataType           = T;
    using StorageType        = SparseVectorData<T, SizeType>;
    using BitBlockType       = TBitBlock;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline SparseVectorMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~SparseVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline SparseVectorMemory(const SparseVectorMemory& rhs) noexcept
        : Allocator(rhs)
    {
        if (rhs._sparse_size)
        {
            // reserve data
            realloc(rhs._sparse_size);

            // copy data
            copy_sparse_vector_data(_data, rhs._data, rhs._bit_data, rhs._sparse_size);
            copy_sparse_vector_bit_data(_bit_data, rhs._bit_data, rhs._sparse_size);
            _sparse_size   = rhs._sparse_size;
            _freelist_head = rhs._freelist_head;
            _hole_size     = rhs._hole_size;
        }
    }
    inline SparseVectorMemory(SparseVectorMemory&& rhs) noexcept
        : Allocator(std::move(rhs))
        , _data(rhs._data)
        , _bit_data(rhs._bit_data)
        , _sparse_size(rhs._sparse_size)
        , _capacity(rhs._capacity)
        , _freelist_head(rhs._freelist_head)
        , _hole_size(rhs._hole_size)
    {
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const SparseVectorMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // copy allocator
            Allocator::operator=(rhs);

            // clean up self
            clear();

            // copy data
            if ((rhs._sparse_size - rhs._hole_size) > 0)
            {
                // reserve data
                if (_capacity < rhs._sparse_size)
                {
                    realloc(rhs._sparse_size);
                }

                // copy data
                copy_sparse_vector_data(_data, rhs._data, rhs._bit_data, rhs._sparse_size);
                copy_sparse_vector_bit_data(_bit_data, rhs._bit_data, rhs._sparse_size);
                _sparse_size   = rhs._sparse_size;
                _capacity      = rhs._capacity;
                _freelist_head = rhs._freelist_head;
                _hole_size     = rhs._hole_size;
            }
        }
    }
    inline void operator=(SparseVectorMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // move allocator
            Allocator::operator=(std::move(rhs));

            // clean up self
            clear();

            // free
            free();

            // move data
            _data          = rhs._data;
            _bit_data      = rhs._bit_data;
            _sparse_size   = rhs._sparse_size;
            _capacity      = rhs._capacity;
            _freelist_head = rhs._freelist_head;
            _hole_size     = rhs._hole_size;

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_sparse_size <= new_capacity);
        SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

        // realloc bit data
        SizeType new_block_size = BitAlgo::num_blocks(new_capacity);
        SizeType old_block_size = BitAlgo::num_blocks(_capacity);
        if (new_block_size != old_block_size)
        {
            if constexpr (memory::MemoryTraits<BitBlockType>::use_realloc && Allocator::support_realloc)
            {
                _bit_data = Allocator::template realloc<BitBlockType>(_bit_data, new_block_size);
            }
            else
            {
                // alloc new memory
                BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_size);

                // move data
                if (old_block_size)
                {
                    memory::move(new_memory, _bit_data, std::min(new_block_size, old_block_size));
                }

                // release old memory
                Allocator::template free<BitBlockType>(_bit_data);

                // update data
                _bit_data = new_memory;
            }
        }

        // realloc data data
        if constexpr (memory::MemoryTraits<DataType>::use_realloc && Allocator::support_realloc)
        {
            _data = Allocator::template realloc<StorageType>(_data, new_capacity);
        }
        else
        {
            // alloc new memory
            StorageType* new_memory = Allocator::template alloc<StorageType>(new_capacity);

            // move items
            if (_sparse_size)
            {
                move_sparse_vector_data(new_memory, _data, _bit_data, _sparse_size);
            }

            // release old memory
            Allocator::template free<StorageType>(_data);

            // update data
            _data = new_memory;
        }

        // update capacity
        _capacity = new_capacity;
    }
    inline void free() noexcept
    {
        if (_data)
        {
            // release memory
            Allocator::template free<StorageType>(_data);
            Allocator::template free<BitBlockType>(_bit_data);

            // reset data
            _data     = nullptr;
            _bit_data = nullptr;
            _capacity = 0;
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size        = _sparse_size;
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
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<DataType>(_sparse_size, _capacity);
        SKR_ASSERT(new_capacity >= _sparse_size);
        if (new_capacity < _capacity)
        {
            if (new_capacity)
            {
                realloc(new_capacity);
            }
            else
            {
                free();
            }
        }
    }
    inline void clear() noexcept
    {
        if (_sparse_size)
        {
            // destruct items
            destruct_sparse_vector_data(_data, _bit_data, _sparse_size);

            // clean up bit data
            if (_bit_data)
            {
                BitAlgo::set_blocks(_bit_data, SizeType(0), BitAlgo::num_blocks(_sparse_size), false);
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
    inline BitBlockType*       bit_data() noexcept { return _bit_data; }
    inline const BitBlockType* bit_data() const noexcept { return _bit_data; }
    inline SizeType            sparse_size() const noexcept { return _sparse_size; }
    inline SizeType            capacity() const noexcept { return _capacity; }
    inline SizeType            bit_size() const noexcept { return BitAlgo::num_blocks(_capacity) * BitAlgo::PerBlockSize; }
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

    inline void _reset() noexcept
    {
        _data          = nullptr;
        _bit_data      = nullptr;
        _sparse_size   = 0;
        _capacity      = 0;
        _freelist_head = npos;
        _hole_size     = 0;
    }

private:
    StorageType*  _data          = nullptr;
    BitBlockType* _bit_data      = nullptr;
    SizeType      _sparse_size   = 0;
    SizeType      _capacity      = 0;
    SizeType      _freelist_head = npos;
    SizeType      _hole_size     = 0;
};
} // namespace skr::container

// fixed sparse vector memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, uint64_t kCount>
struct FixedSparseVectorMemory {
    static_assert(kCount > 0, "FixedSparseVectorMemory must have a capacity larger than 0");
    struct DummyParam {
    };

    // configure
    using SizeType           = TS;
    using DataType           = T;
    using StorageType        = SparseVectorData<T, SizeType>;
    using BitBlockType       = TBitBlock;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedSparseVectorMemory(AllocatorCtorParam) noexcept
    {
    }
    inline ~FixedSparseVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline FixedSparseVectorMemory(const FixedSparseVectorMemory& rhs) noexcept
    {
        if (rhs._sparse_size)
        {
            // copy data
            copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            _sparse_size   = rhs._sparse_size;
            _freelist_head = rhs._freelist_head;
            _hole_size     = rhs._hole_size;
        }
    }
    inline FixedSparseVectorMemory(FixedSparseVectorMemory&& rhs) noexcept
    {
        if (rhs._sparse_size)
        {
            move_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            _sparse_size   = rhs._sparse_size;
            _freelist_head = rhs._freelist_head;
            _hole_size     = rhs._hole_size;

            rhs._reset();
        }
    }

    // assign & move assign
    inline void operator=(const FixedSparseVectorMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy data
            if ((rhs._sparse_size - rhs._hole_size) > 0)
            {
                // copy data
                copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
                copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
                _sparse_size   = rhs._sparse_size;
                _freelist_head = rhs._freelist_head;
                _hole_size     = rhs._hole_size;
            }
        }
    }
    inline void operator=(FixedSparseVectorMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // move data
            if ((rhs.sparse_size() - rhs.hole_size()) > 0)
            {
                move_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
                move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
                _sparse_size   = rhs._sparse_size;
                _freelist_head = rhs._freelist_head;
                _hole_size     = rhs._hole_size;
            }

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity <= kCount && "FixedSparseVectorMemory can't alloc memory that larger than kCount");
    }
    inline void free() noexcept
    {
        // do noting
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SKR_ASSERT((_sparse_size + grow_size) <= kCount && "FixedSparseVectorMemory can't alloc memory that larger than kCount");

        SizeType old_size = _sparse_size;
        _sparse_size += grow_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        // do noting
    }
    inline void clear() noexcept
    {
        if (_sparse_size)
        {
            // destruct items
            destruct_sparse_vector_data(data(), bit_data(), sparse_size());

            // clean up bit data
            BitAlgo::set_blocks(bit_data(), SizeType(0), BitAlgo::num_blocks(_sparse_size), false);

            // clean up data
            _hole_size     = 0;
            _sparse_size   = 0;
            _freelist_head = npos;
        }
    }

    // getter
    inline StorageType*        data() noexcept { return _data_placeholder.data_typed(); }
    inline const StorageType*  data() const noexcept { return _data_placeholder.data_typed(); }
    inline BitBlockType*       bit_data() noexcept { return _bit_data_placeholder.data_typed(); }
    inline const BitBlockType* bit_data() const noexcept { return _bit_data_placeholder.data_typed(); }
    inline SizeType            sparse_size() const noexcept { return _sparse_size; }
    inline SizeType            capacity() const noexcept { return kCount; }
    inline SizeType            bit_data_size() const noexcept { return fixed_bit_count; }
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

    // helper functions
    static constexpr SizeType fixed_block_count = int_div_ceil(kCount, BitAlgo::PerBlockSize);
    static constexpr SizeType fixed_bit_count   = fixed_block_count * BitAlgo::PerBlockSize;

    inline void _reset() noexcept
    {
        _sparse_size   = 0;
        _hole_size     = 0;
        _freelist_head = npos;
    }

private:
    Placeholder<StorageType, kCount>             _data_placeholder;
    Placeholder<BitBlockType, fixed_block_count> _bit_data_placeholder;
    SizeType                                     _sparse_size   = 0;
    SizeType                                     _hole_size     = 0;
    SizeType                                     _freelist_head = npos;
};
} // namespace skr::container

// inline sparse vector memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, uint64_t kInlineCount, typename Allocator>
struct InlineSparseVectorMemory : public Allocator {
    // configure
    using SizeType           = TS;
    using DataType           = T;
    using StorageType        = SparseVectorData<T, SizeType>;
    using BitBlockType       = TBitBlock;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline InlineSparseVectorMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~InlineSparseVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline InlineSparseVectorMemory(const InlineSparseVectorMemory& rhs) noexcept
        : Allocator(rhs)
    {
        if (rhs.sparse_size())
        {
            // reserve data
            realloc(rhs.sparse_size());

            // copy data
            copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            _sparse_size   = rhs._sparse_size;
            _freelist_head = rhs._freelist_head;
            _hole_size     = rhs._hole_size;
        }
    }
    inline InlineSparseVectorMemory(InlineSparseVectorMemory&& rhs) noexcept
        : Allocator(std::move(rhs))
    {
        if (rhs._is_using_inline_data())
        {
            // move data
            move_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            _sparse_size   = rhs._sparse_size;
            _freelist_head = rhs._freelist_head;
            _hole_size     = rhs._hole_size;
        }
        else
        {
            // move bit data
            if (rhs._is_using_inline_bit_data())
            {
                move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            }
            else
            {
                _heap_bit_data     = rhs._heap_bit_data;
                rhs._heap_bit_data = nullptr;
            }

            // move data
            _heap_data     = rhs._heap_data;
            _sparse_size   = rhs._sparse_size;
            _capacity      = rhs._capacity;
            _freelist_head = rhs._freelist_head;
            _hole_size     = rhs._hole_size;
        }

        // reset rhs
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const InlineSparseVectorMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // copy allocator
            Allocator::operator=(rhs);

            // clean up self
            clear();

            // copy data
            if ((rhs._sparse_size - rhs._hole_size) > 0)
            {
                // reserve data
                if (_capacity < rhs._sparse_size)
                {
                    realloc(rhs._sparse_size);
                }

                // copy data
                copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs._sparse_size);
                copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs._sparse_size);
                _sparse_size   = rhs._sparse_size;
                _capacity      = rhs._capacity;
                _freelist_head = rhs._freelist_head;
                _hole_size     = rhs._hole_size;
            }
        }
    }
    inline void operator=(InlineSparseVectorMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // move allocator
            Allocator::operator=(std::move(rhs));

            // clean up self
            clear();

            // free memory
            free();

            // move data
            if (rhs._is_using_inline_data())
            {
                // move data
                move_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
                move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
                _sparse_size   = rhs._sparse_size;
                _freelist_head = rhs._freelist_head;
                _hole_size     = rhs._hole_size;
            }
            else
            {
                // move bit data
                if (rhs._is_using_inline_bit_data())
                {
                    move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
                }
                else
                {
                    _heap_bit_data     = rhs._heap_bit_data;
                    rhs._heap_bit_data = nullptr;
                }

                // move data
                _heap_data     = rhs._heap_data;
                _sparse_size   = rhs._sparse_size;
                _capacity      = rhs._capacity;
                _freelist_head = rhs._freelist_head;
                _hole_size     = rhs._hole_size;
            }

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_sparse_size <= new_capacity);
        new_capacity = new_capacity < kInlineCount ? kInlineCount : new_capacity;

        // realloc bit data
        BitBlockType* moved_bit_data = nullptr;
        {
            SizeType new_block_size = BitAlgo::num_blocks(new_capacity);
            SizeType old_block_size = BitAlgo::num_blocks(_capacity);

            if (new_block_size > kInlineBlockCount)
            {
                if (_is_using_inline_bit_data()) // inline -> heap
                {
                    // alloc new bit data
                    BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_size);

                    // move data
                    if (old_block_size)
                    {
                        memory::move(new_memory, _bit_data_placeholder.data_typed(), std::min(new_block_size, old_block_size));
                    }

                    // update data
                    _heap_bit_data = new_memory;
                }
                else // heap -> heap
                {
                    if (new_block_size != old_block_size)
                    {
                        if constexpr (memory::MemoryTraits<BitBlockType>::use_realloc && Allocator::support_realloc)
                        {
                            _heap_bit_data = Allocator::template realloc<BitBlockType>(_heap_bit_data, new_block_size);
                        }
                        else
                        {
                            // alloc new memory
                            BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_size);

                            // move data
                            if (old_block_size)
                            {
                                memory::move(new_memory, _heap_bit_data, std::min(new_block_size, old_block_size));
                            }

                            // release old memory
                            Allocator::template free<BitBlockType>(_heap_bit_data);

                            // update data
                            _heap_bit_data = new_memory;
                        }
                    }
                }

                moved_bit_data = _heap_bit_data;
            }
            else
            {
                if (_is_using_inline_bit_data()) // inline -> inline
                {
                    // do noting
                }
                else // heap -> inline
                {
                    BitBlockType* cached_heap_bit_data = _heap_bit_data;

                    // move items
                    if (_sparse_size - _hole_size)
                    {
                        move_sparse_vector_bit_data(_bit_data_placeholder.data_typed(), _heap_bit_data, _sparse_size);
                    }

                    // release old memory
                    Allocator::template free<BitBlockType>(cached_heap_bit_data);
                }

                moved_bit_data = _bit_data_placeholder.data_typed();
            }
        }

        // realloc element data
        if (new_capacity > kInlineCount)
        {
            if (_is_using_inline_data()) // inline -> heap
            {
                // alloc data
                {
                    // alloc new memory
                    StorageType* new_memory = Allocator::template alloc<StorageType>(new_capacity);

                    // move items
                    if (_sparse_size - _hole_size)
                    {
                        move_sparse_vector_data(new_memory, _data_placeholder.data_typed(), moved_bit_data, _sparse_size);
                    }

                    // update data
                    _heap_data = new_memory;
                }
            }
            else // heap -> heap
            {
                // realloc data
                if constexpr (memory::MemoryTraits<DataType>::use_realloc && Allocator::support_realloc)
                {
                    _heap_data = Allocator::template realloc<StorageType>(_heap_data, new_capacity);
                }
                else
                {
                    // alloc new memory
                    StorageType* new_memory = Allocator::template alloc<StorageType>(new_capacity);

                    // move items
                    if (_sparse_size)
                    {
                        move_sparse_vector_data(new_memory, _heap_data, moved_bit_data, _sparse_size);
                    }

                    // release old memory
                    Allocator::template free<StorageType>(_heap_data);

                    // update data
                    _heap_data = new_memory;
                }
            }
        }
        else
        {
            if (_is_using_inline_data()) // inline -> inline
            {
                // do noting
            }
            else // heap -> inline
            {
                StorageType* cached_heap_data = _heap_data;

                // move items
                if (_sparse_size - _hole_size)
                {
                    move_sparse_vector_data(_data_placeholder.data_typed(), _heap_data, moved_bit_data, _sparse_size);
                }

                // release old memory
                Allocator::template free<StorageType>(cached_heap_data);
            }
        }

        // update capacity
        _capacity = new_capacity;
    }
    inline void free() noexcept
    {
        if (!_is_using_inline_data())
        {
            // release memory
            Allocator::template free<StorageType>(_heap_data);

            // release bit data
            if (!_is_using_inline_bit_data())
            {
                Allocator::template free<BitBlockType>(_heap_bit_data);
            }

            // reset data
            _capacity = kInlineCount;
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size        = _sparse_size;
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
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<DataType>(_sparse_size, _capacity);
        SKR_ASSERT(new_capacity >= _sparse_size);
        if (new_capacity < _capacity)
        {
            if (new_capacity)
            {
                realloc(new_capacity);
            }
            else
            {
                free();
            }
        }
    }
    inline void clear() noexcept
    {
        if (_sparse_size)
        {
            // destruct items
            destruct_sparse_vector_data(data(), bit_data(), _sparse_size);

            // clean up bit data
            if (bit_data())
            {
                BitAlgo::set_blocks(bit_data(), SizeType(0), BitAlgo::num_blocks(_sparse_size), false);
            }

            // clean up data
            _hole_size     = 0;
            _sparse_size   = 0;
            _freelist_head = npos;
        }
    }

    // getter
    inline StorageType*        data() noexcept { return _is_using_inline_data() ? _data_placeholder.data_typed() : _heap_data; }
    inline const StorageType*  data() const noexcept { return _is_using_inline_data() ? _data_placeholder.data_typed() : _heap_data; }
    inline BitBlockType*       bit_data() noexcept { return _is_using_inline_bit_data() ? _bit_data_placeholder.data_typed() : _heap_bit_data; }
    inline const BitBlockType* bit_data() const noexcept { return _is_using_inline_bit_data() ? _bit_data_placeholder.data_typed() : _heap_bit_data; }
    inline SizeType            sparse_size() const noexcept { return _sparse_size; }
    inline SizeType            capacity() const noexcept { return _capacity; }
    inline SizeType            bit_data_size() const noexcept { return BitAlgo::num_blocks(_capacity) * BitAlgo::PerBlockSize; }
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

    // helper functions
    static constexpr SizeType kInlineBlockCount = int_div_ceil(kInlineCount, BitAlgo::PerBlockSize);

    inline bool _is_using_inline_data() const noexcept { return _capacity == kInlineCount; }
    inline bool _is_using_inline_bit_data() const noexcept { return BitAlgo::num_blocks(_capacity) == kInlineBlockCount; }

    inline void _reset() noexcept
    {
        _sparse_size   = 0;
        _capacity      = kInlineCount;
        _freelist_head = npos;
        _hole_size     = 0;
    }

private:
    union
    {
        Placeholder<StorageType, kInlineCount> _data_placeholder;
        StorageType*                           _heap_data;
    };
    union
    {
        Placeholder<BitBlockType, kInlineBlockCount> _bit_data_placeholder;
        BitBlockType*                                _heap_bit_data;
    };
    SizeType _sparse_size   = 0;
    SizeType _capacity      = kInlineCount;
    SizeType _freelist_head = npos;
    SizeType _hole_size     = 0;
};
} // namespace skr::container