#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/bit_algo.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_def.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/misc/default_capicity_policy.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_helper.hpp"
#include <algorithm>

// generic fwd decl
namespace skr
{
struct GenericSparseVector;
} // namespace skr

// sparse vector memory base
namespace skr::container
{
template <typename TSize>
struct SparseVectorMemoryBase {
    using SizeType = TSize;
    friend struct ::skr::GenericSparseVector;

    // getter
    inline SizeType sparse_size() const noexcept { return _sparse_size; }
    inline SizeType capacity() const noexcept { return _capacity; }
    inline SizeType bit_size() const noexcept { return algo::BitAlgo<TSize>::num_blocks(_capacity) * algo::BitAlgo<TSize>::PerBlockSize; }
    inline SizeType freelist_head() const noexcept { return _freelist_head; }
    inline SizeType hole_size() const noexcept { return _hole_size; }

    // setter
    inline void set_sparse_size(SizeType value) noexcept { _sparse_size = value; }
    inline void set_freelist_head(SizeType value) noexcept { _freelist_head = value; }
    inline void set_hole_size(SizeType value) noexcept { _hole_size = value; }

protected:
    // helper for generic sparse vector
    inline void _reset()
    {
        _data          = nullptr;
        _bit_data      = nullptr;
        _sparse_size   = 0;
        _capacity      = 0;
        _freelist_head = npos_of<SizeType>;
        _hole_size     = 0;
    }
    inline static SizeType _storage_size(SizeType item_size)
    {
        constexpr SizeType freelist_node_size = sizeof(SparseVectorFreeListNode<TSize>);
        return std::max(freelist_node_size, item_size);
    }
    inline void* _storage_at(SizeType index, SizeType item_size) const
    {
        return ::skr::memory::offset_item(_data, _storage_size(item_size), index);
    }
    inline SparseVectorFreeListNode<TSize>* _freelist_node_at(SizeType index, SizeType item_size) const
    {
        return reinterpret_cast<SparseVectorFreeListNode<TSize>*>(
            _storage_at(index, _storage_size(item_size))
        );
    }
    inline uint64_t* _typed_bit_data() const
    {
        return reinterpret_cast<uint64_t*>(_bit_data);
    }

protected:
    void*    _data          = nullptr;
    void*    _bit_data      = nullptr;
    SizeType _sparse_size   = 0;
    SizeType _capacity      = 0;
    SizeType _freelist_head = npos_of<SizeType>;
    SizeType _hole_size     = 0;
};
} // namespace skr::container

// util sparse vector memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename Base, typename Allocator>
struct SparseVectorMemory : public Base, public Allocator {
    // configure
    using SizeType           = typename Base::SizeType;
    using DataType           = T;
    using StorageType        = SparseVectorStorage<T, SizeType>;
    using BitBlockType       = TBitBlock;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline SparseVectorMemory(AllocatorCtorParam param) noexcept
        : Base()
        , Allocator(std::move(param))
    {
    }
    inline ~SparseVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline SparseVectorMemory(const SparseVectorMemory& rhs) noexcept
        : Base()
        , Allocator(rhs)
    {
        if (rhs._sparse_size)
        {
            // reserve data
            realloc(rhs._sparse_size);

            // copy data
            copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs._sparse_size);
            copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs._sparse_size);
            Base::_sparse_size   = rhs._sparse_size;
            Base::_freelist_head = rhs._freelist_head;
            Base::_hole_size     = rhs._hole_size;
        }
    }
    inline SparseVectorMemory(SparseVectorMemory&& rhs) noexcept
        : Base(std::move(rhs))
        , Allocator(std::move(rhs))
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
                if (Base::_capacity < rhs._sparse_size)
                {
                    realloc(rhs._sparse_size);
                }

                // copy data
                copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs._sparse_size);
                copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs._sparse_size);
                Base::_sparse_size   = rhs._sparse_size;
                Base::_freelist_head = rhs._freelist_head;
                Base::_hole_size     = rhs._hole_size;
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
            Base::_data          = rhs._data;
            Base::_bit_data      = rhs._bit_data;
            Base::_sparse_size   = rhs._sparse_size;
            Base::_capacity      = rhs._capacity;
            Base::_freelist_head = rhs._freelist_head;
            Base::_hole_size     = rhs._hole_size;

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != Base::_capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(Base::_sparse_size <= new_capacity);
        SKR_ASSERT((Base::_capacity > 0 && Base::_data != nullptr) || (Base::_capacity == 0 && Base::_data == nullptr));

        // realloc bit data
        SizeType new_block_size = BitAlgo::num_blocks(new_capacity);
        SizeType old_block_size = BitAlgo::num_blocks(Base::_capacity);
        if (new_block_size != old_block_size)
        {
            if constexpr (::skr::memory::MemoryTraits<BitBlockType>::use_realloc && Allocator::support_realloc)
            {
                Base::_bit_data = Allocator::template realloc<BitBlockType>(bit_data(), new_block_size);
            }
            else
            {
                // alloc new memory
                BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_size);

                // move data
                if (old_block_size)
                {
                    ::skr::memory::move(new_memory, bit_data(), std::min(new_block_size, old_block_size));
                }

                // release old memory
                Allocator::template free<BitBlockType>(bit_data());

                // update data
                Base::_bit_data = new_memory;
            }
        }

        // realloc data data
        if constexpr (::skr::memory::MemoryTraits<DataType>::use_realloc && Allocator::support_realloc)
        {
            Base::_data = Allocator::template realloc<StorageType>(data(), new_capacity);
        }
        else
        {
            // alloc new memory
            StorageType* new_memory = Allocator::template alloc<StorageType>(new_capacity);

            // move items
            if (Base::_sparse_size)
            {
                move_sparse_vector_data(new_memory, data(), bit_data(), Base::_sparse_size);
            }

            // release old memory
            Allocator::template free<StorageType>(data());

            // update data
            Base::_data = new_memory;
        }

        // update capacity
        Base::_capacity = new_capacity;
    }
    inline void free() noexcept
    {
        if (data())
        {
            // release memory
            Allocator::template free<StorageType>(data());
            Allocator::template free<BitBlockType>(bit_data());

            // reset data
            Base::_data     = nullptr;
            Base::_bit_data = nullptr;
            Base::_capacity = 0;
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size        = Base::_sparse_size;
        SizeType new_sparse_size = Base::_sparse_size + grow_size;

        if (new_sparse_size > Base::_capacity)
        {
            auto new_capacity = default_get_grow<DataType>(new_sparse_size, Base::_capacity);
            SKR_ASSERT(new_capacity >= Base::_capacity);
            if (new_capacity > Base::_capacity)
            {
                realloc(new_capacity);
            }
        }

        Base::_sparse_size = new_sparse_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<DataType>(Base::_sparse_size, Base::_capacity);
        SKR_ASSERT(new_capacity >= Base::_sparse_size);
        if (new_capacity < Base::_capacity)
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
        if (Base::_sparse_size)
        {
            // destruct items
            destruct_sparse_vector_data(data(), bit_data(), Base::_sparse_size);

            // clean up bit data
            if (bit_data())
            {
                BitAlgo::set_blocks(bit_data(), SizeType(0), BitAlgo::num_blocks(Base::_sparse_size), false);
            }

            // clean up data
            Base::_hole_size     = 0;
            Base::_sparse_size   = 0;
            Base::_freelist_head = npos;
        }
    }

    // getter
    inline StorageType*        data() noexcept { return reinterpret_cast<StorageType*>(Base::_data); }
    inline const StorageType*  data() const noexcept { return reinterpret_cast<const StorageType*>(Base::_data); }
    inline BitBlockType*       bit_data() noexcept { return reinterpret_cast<BitBlockType*>(Base::_bit_data); }
    inline const BitBlockType* bit_data() const noexcept { return reinterpret_cast<const BitBlockType*>(Base::_bit_data); }

private:
    // algo
    using BitAlgo                         = algo::BitAlgo<TBitBlock>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    inline void _reset() noexcept
    {
        Base::_data          = nullptr;
        Base::_bit_data      = nullptr;
        Base::_sparse_size   = 0;
        Base::_capacity      = 0;
        Base::_freelist_head = npos;
        Base::_hole_size     = 0;
    }
};
} // namespace skr::container

// fixed sparse vector memory
namespace skr::container
{
template <typename T, typename TBitBlock, uint64_t kCount, typename Base>
struct FixedSparseVectorMemory : public Base {
    static_assert(kCount > 0, "FixedSparseVectorMemory must have a capacity larger than 0");
    struct DummyParam {
    };

    // configure
    using SizeType           = typename Base::SizeType;
    using DataType           = T;
    using StorageType        = SparseVectorStorage<T, SizeType>;
    using BitBlockType       = TBitBlock;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedSparseVectorMemory(AllocatorCtorParam) noexcept
    {
        _init_setup();
    }
    inline ~FixedSparseVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline FixedSparseVectorMemory(const FixedSparseVectorMemory& rhs) noexcept
    {
        _init_setup();

        if (rhs._sparse_size)
        {
            // copy data
            copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            Base::_sparse_size   = rhs._sparse_size;
            Base::_freelist_head = rhs._freelist_head;
            Base::_hole_size     = rhs._hole_size;
        }
    }
    inline FixedSparseVectorMemory(FixedSparseVectorMemory&& rhs) noexcept
    {
        _init_setup();

        if (rhs._sparse_size)
        {
            move_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            Base::_sparse_size   = rhs._sparse_size;
            Base::_freelist_head = rhs._freelist_head;
            Base::_hole_size     = rhs._hole_size;

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
                Base::_sparse_size   = rhs._sparse_size;
                Base::_freelist_head = rhs._freelist_head;
                Base::_hole_size     = rhs._hole_size;
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
                Base::_sparse_size   = rhs._sparse_size;
                Base::_freelist_head = rhs._freelist_head;
                Base::_hole_size     = rhs._hole_size;
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
        SKR_ASSERT((Base::_sparse_size + grow_size) <= kCount && "FixedSparseVectorMemory can't alloc memory that larger than kCount");

        SizeType old_size = Base::_sparse_size;
        Base::_sparse_size += grow_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        // do noting
    }
    inline void clear() noexcept
    {
        if (Base::_sparse_size)
        {
            // destruct items
            destruct_sparse_vector_data(data(), bit_data(), Base::_sparse_size);

            // clean up bit data
            BitAlgo::set_blocks(bit_data(), SizeType(0), BitAlgo::num_blocks(Base::_sparse_size), false);

            // clean up data
            Base::_hole_size     = 0;
            Base::_sparse_size   = 0;
            Base::_freelist_head = npos;
        }
    }

    // getter
    inline StorageType*        data() noexcept { return reinterpret_cast<StorageType*>(Base::_data); }
    inline const StorageType*  data() const noexcept { return reinterpret_cast<const StorageType*>(Base::_data); }
    inline BitBlockType*       bit_data() noexcept { return reinterpret_cast<BitBlockType*>(Base::_bit_data); }
    inline const BitBlockType* bit_data() const noexcept { return reinterpret_cast<const BitBlockType*>(Base::_bit_data); }

private:
    // algo
    using BitAlgo                         = algo::BitAlgo<TBitBlock>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // helper functions
    static constexpr SizeType fixed_block_count = int_div_ceil(kCount, BitAlgo::PerBlockSize);
    static constexpr SizeType fixed_bit_count   = fixed_block_count * BitAlgo::PerBlockSize;

    inline void _init_setup() noexcept
    {
        Base::_data     = _data_placeholder.data_typed();
        Base::_bit_data = _bit_data_placeholder.data_typed();
        Base::_capacity = kCount;
    }
    inline void _reset() noexcept
    {
        Base::_sparse_size   = 0;
        Base::_freelist_head = npos;
        Base::_hole_size     = 0;
    }

private:
    Placeholder<StorageType, kCount>             _data_placeholder;
    Placeholder<BitBlockType, fixed_block_count> _bit_data_placeholder;
};
} // namespace skr::container

// inline sparse vector memory
namespace skr::container
{
template <typename T, typename TBitBlock, uint64_t kInlineCount, typename Base, typename Allocator>
struct InlineSparseVectorMemory : public Base, public Allocator {
    // configure
    using SizeType           = typename Base::SizeType;
    using DataType           = T;
    using StorageType        = SparseVectorStorage<T, SizeType>;
    using BitBlockType       = TBitBlock;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline InlineSparseVectorMemory(AllocatorCtorParam param) noexcept
        : Base()
        , Allocator(std::move(param))
    {
        _reset();
    }
    inline ~InlineSparseVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline InlineSparseVectorMemory(const InlineSparseVectorMemory& rhs) noexcept
        : Base()
        , Allocator(rhs)
    {
        _reset();

        if (rhs.sparse_size())
        {
            // reserve data
            realloc(rhs.sparse_size());

            // copy data
            copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            Base::_sparse_size   = rhs._sparse_size;
            Base::_freelist_head = rhs._freelist_head;
            Base::_hole_size     = rhs._hole_size;
        }
    }
    inline InlineSparseVectorMemory(InlineSparseVectorMemory&& rhs) noexcept
        : Base()
        , Allocator(std::move(rhs))
    {
        _reset();

        if (rhs._is_using_inline_data())
        {
            // move data, when use inline data, bit data will also be inline
            move_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs.sparse_size());
            move_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs.sparse_size());
            Base::_sparse_size   = rhs._sparse_size;
            Base::_freelist_head = rhs._freelist_head;
            Base::_hole_size     = rhs._hole_size;
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
                Base::_bit_data = rhs._bit_data;
            }

            // move data
            Base::_data          = rhs._data;
            Base::_sparse_size   = rhs._sparse_size;
            Base::_capacity      = rhs._capacity;
            Base::_freelist_head = rhs._freelist_head;
            Base::_hole_size     = rhs._hole_size;
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
                if (Base::_capacity < rhs._sparse_size)
                {
                    realloc(rhs._sparse_size);
                }

                // copy data
                copy_sparse_vector_data(data(), rhs.data(), rhs.bit_data(), rhs._sparse_size);
                copy_sparse_vector_bit_data(bit_data(), rhs.bit_data(), rhs._sparse_size);
                Base::_sparse_size   = rhs._sparse_size;
                Base::_freelist_head = rhs._freelist_head;
                Base::_hole_size     = rhs._hole_size;
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
                Base::_sparse_size   = rhs._sparse_size;
                Base::_freelist_head = rhs._freelist_head;
                Base::_hole_size     = rhs._hole_size;
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
                    Base::_bit_data = rhs._bit_data;
                }

                // move data
                Base::_data          = rhs._data;
                Base::_sparse_size   = rhs._sparse_size;
                Base::_capacity      = rhs._capacity;
                Base::_freelist_head = rhs._freelist_head;
                Base::_hole_size     = rhs._hole_size;
            }

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != Base::_capacity || new_capacity <= kInlineCount);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(Base::_sparse_size <= new_capacity);
        new_capacity = new_capacity < kInlineCount ? kInlineCount : new_capacity;

        // realloc bit data
        BitBlockType* moved_bit_data = nullptr;
        {
            SizeType new_block_size = BitAlgo::num_blocks(new_capacity);
            SizeType old_block_size = BitAlgo::num_blocks(Base::_capacity);

            if (new_block_size > kInlineBlockCount)
            {
                if (_is_using_inline_bit_data()) // inline -> heap
                {
                    // alloc new bit data
                    BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_size);

                    // move data
                    if (old_block_size)
                    {
                        ::skr::memory::move(new_memory, _bit_data_placeholder.data_typed(), std::min(new_block_size, old_block_size));
                    }

                    // update data
                    Base::_bit_data = new_memory;
                }
                else // heap -> heap
                {
                    if (new_block_size != old_block_size)
                    {
                        if constexpr (::skr::memory::MemoryTraits<BitBlockType>::use_realloc && Allocator::support_realloc)
                        {
                            Base::_bit_data = Allocator::template realloc<BitBlockType>((BitBlockType*)Base::_bit_data, new_block_size);
                        }
                        else
                        {
                            // alloc new memory
                            BitBlockType* new_memory = Allocator::template alloc<BitBlockType>(new_block_size);

                            // move data
                            if (old_block_size)
                            {
                                ::skr::memory::move(new_memory, bit_data(), std::min(new_block_size, old_block_size));
                            }

                            // release old memory
                            Allocator::template free<BitBlockType>(bit_data());

                            // update data
                            Base::_bit_data = new_memory;
                        }
                    }
                }

                moved_bit_data = bit_data();
            }
            else
            {
                if (_is_using_inline_bit_data()) // inline -> inline
                {
                    // do noting
                }
                else // heap -> inline
                {
                    BitBlockType* cached_heap_bit_data = bit_data();

                    // move items
                    if (Base::_sparse_size - Base::_hole_size)
                    {
                        move_sparse_vector_bit_data(_bit_data_placeholder.data_typed(), bit_data(), Base::_sparse_size);
                    }

                    // release old memory
                    Allocator::template free<BitBlockType>(cached_heap_bit_data);

                    // reset data
                    Base::_bit_data = _bit_data_placeholder.data_typed();
                }

                moved_bit_data = bit_data();
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
                    if (Base::_sparse_size - Base::_hole_size)
                    {
                        move_sparse_vector_data(new_memory, _data_placeholder.data_typed(), moved_bit_data, Base::_sparse_size);
                    }

                    // update data
                    Base::_data = new_memory;
                }
            }
            else // heap -> heap
            {
                // realloc data
                if constexpr (::skr::memory::MemoryTraits<DataType>::use_realloc && Allocator::support_realloc)
                {
                    Base::_data = Allocator::template realloc<StorageType>(data(), new_capacity);
                }
                else
                {
                    // alloc new memory
                    StorageType* new_memory = Allocator::template alloc<StorageType>(new_capacity);

                    // move items
                    if (Base::_sparse_size)
                    {
                        move_sparse_vector_data(new_memory, data(), moved_bit_data, Base::_sparse_size);
                    }

                    // release old memory
                    Allocator::template free<StorageType>(data());

                    // update data
                    Base::_data = new_memory;
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
                StorageType* cached_heap_data = data();

                // move items
                if (Base::_sparse_size - Base::_hole_size)
                {
                    move_sparse_vector_data(_data_placeholder.data_typed(), data(), moved_bit_data, Base::_sparse_size);
                }

                // release old memory
                Allocator::template free<StorageType>(cached_heap_data);

                // reset data
                Base::_data = _data_placeholder.data_typed();
            }
        }

        // update capacity
        Base::_capacity = new_capacity;
    }
    inline void free() noexcept
    {
        if (!_is_using_inline_data())
        {
            // release memory
            Allocator::template free<StorageType>(data());

            // release bit data
            if (!_is_using_inline_bit_data())
            {
                Allocator::template free<BitBlockType>(bit_data());
            }

            _reset();
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size        = Base::_sparse_size;
        SizeType new_sparse_size = Base::_sparse_size + grow_size;

        if (new_sparse_size > Base::_capacity)
        {
            auto new_capacity = default_get_grow<DataType>(new_sparse_size, Base::_capacity);
            SKR_ASSERT(new_capacity >= Base::_capacity);
            if (new_capacity > Base::_capacity)
            {
                realloc(new_capacity);
            }
        }

        Base::_sparse_size = new_sparse_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<DataType>(Base::_sparse_size, Base::_capacity);
        SKR_ASSERT(new_capacity >= Base::_sparse_size);
        if (new_capacity < Base::_capacity)
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
        if (Base::_sparse_size)
        {
            // destruct items
            destruct_sparse_vector_data(data(), bit_data(), Base::_sparse_size);

            // clean up bit data
            if (bit_data())
            {
                BitAlgo::set_blocks(bit_data(), SizeType(0), BitAlgo::num_blocks(Base::_sparse_size), false);
            }

            // clean up data
            Base::_hole_size     = 0;
            Base::_sparse_size   = 0;
            Base::_freelist_head = npos;
        }
    }

    // getter
    inline StorageType*        data() noexcept { return reinterpret_cast<StorageType*>(Base::_data); }
    inline const StorageType*  data() const noexcept { return reinterpret_cast<const StorageType*>(Base::_data); }
    inline BitBlockType*       bit_data() noexcept { return reinterpret_cast<BitBlockType*>(Base::_bit_data); }
    inline const BitBlockType* bit_data() const noexcept { return reinterpret_cast<const BitBlockType*>(Base::_bit_data); }

private:
    // algo
    using BitAlgo                         = algo::BitAlgo<TBitBlock>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // helper functions
    static constexpr SizeType kInlineBlockCount = int_div_ceil(kInlineCount, BitAlgo::PerBlockSize);

    inline bool _is_using_inline_data() const noexcept { return Base::_data == _data_placeholder.data_typed(); }
    inline bool _is_using_inline_bit_data() const noexcept { return Base::_bit_data == _bit_data_placeholder.data_typed(); }

    inline void _reset() noexcept
    {
        Base::_data          = _data_placeholder.data_typed();
        Base::_bit_data      = _bit_data_placeholder.data_typed();
        Base::_sparse_size   = 0;
        Base::_capacity      = kInlineCount;
        Base::_freelist_head = npos;
        Base::_hole_size     = 0;
    }

private:
    Placeholder<StorageType, kInlineCount>       _data_placeholder;
    Placeholder<BitBlockType, kInlineBlockCount> _bit_data_placeholder;
};
} // namespace skr::container