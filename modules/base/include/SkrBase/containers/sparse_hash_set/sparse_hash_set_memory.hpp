#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_traits.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_memory.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_iterator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_helper.hpp"

// util sparse hash set memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename HashTraits, typename TS, typename Allocator>
struct SparseHashSetMemory : public SparseArrayMemory<SparseHashSetData<T, TS, typename HashTraits::HashType>, TBitBlock, TS, Allocator> {
    using Super = SparseArrayMemory<SparseHashSetData<T, TS, typename HashTraits::HashType>, TBitBlock, TS, Allocator>;

    // sparse array configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType       = typename HashTraits::HashType;
    using HasherType     = typename HashTraits::HasherType;
    using SetDataType    = T;
    using SetStorageType = SparseHashSetData<T, TS, HashType>;

    // ctor & dtor
    inline SparseHashSetMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
    }
    inline ~SparseHashSetMemory() noexcept
    {
        free_bucket();
    }

    // copy & move
    inline SparseHashSetMemory(const SparseHashSetMemory& rhs) noexcept
        : Super(rhs)
    {
        resize_bucket();
        clean_bucket();
        build_bucket();
    }
    inline SparseHashSetMemory(SparseHashSetMemory&& rhs) noexcept
        : Super(std::move(rhs))
        , _bucket(rhs._bucket)
        , _bucket_size(rhs._bucket_size)
        , _bucket_mask(rhs._bucket_mask)
    {
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const SparseHashSetMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(rhs);
            resize_bucket();
            clean_bucket();
            build_bucket();
        }
    }
    inline void operator=(SparseHashSetMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(std::move(rhs));

            // clean up bucket
            free_bucket();

            // move data
            _bucket      = rhs._bucket;
            _bucket_size = rhs._bucket_size;
            _bucket_mask = rhs._bucket_mask;

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline bool resize_bucket() noexcept
    {
        SizeType new_bucket_size = sparse_hash_set_calc_bucket_size(Super::capacity());
        if (new_bucket_size != _bucket_size)
        {
            if (new_bucket_size)
            {
                if constexpr (memory::MemoryTraits<SizeType>::use_realloc && Allocator::support_realloc)
                {
                    _bucket = Allocator::template realloc<SizeType>(_bucket, new_bucket_size);
                }
                else
                {
                    // alloc new memory
                    SizeType* new_memory = Allocator::template alloc<SizeType>(new_bucket_size);

                    // release old memory
                    Allocator::template free<SizeType>(_bucket);

                    _bucket = new_memory;
                }

                _bucket_size = new_bucket_size;
                _bucket_mask = new_bucket_size - 1;
            }
            else
            {
                free_bucket();
            }
            return true;
        }
        return false;
    }
    inline void free_bucket() noexcept
    {
        if (_bucket)
        {
            Allocator::template free<SizeType>(_bucket);
            _bucket      = nullptr;
            _bucket_size = 0;
            _bucket_mask = 0;
        }
    }
    inline SizeType bucket_index(SizeType hash) const noexcept
    {
        return hash & _bucket_mask;
    }
    inline void build_bucket() noexcept
    {
        if (Super::sparse_size() - Super::hole_size())
        {
            auto cursor = TrueBitCursor<BitBlockType, SizeType, false>::Begin(Super::bit_array(), Super::sparse_size());
            sparse_hash_set_build_bucket(Super::data(), _bucket, _bucket_mask, cursor);
        }
    }
    inline void clean_bucket() noexcept
    {
        sparse_hash_set_clean_bucket(_bucket, _bucket_size);
    }

    // getter
    inline const SizeType* bucket() const noexcept { return _bucket; }
    inline SizeType*       bucket() noexcept { return _bucket; }

private:
    static inline constexpr SizeType npos = npos_of<SizeType>;

    inline void _reset() noexcept
    {
        _bucket      = nullptr;
        _bucket_size = 0;
        _bucket_mask = 0;
    }

private:
    SizeType* _bucket      = nullptr;
    SizeType  _bucket_size = 0;
    SizeType  _bucket_mask = 0;
};
} // namespace skr::container

// fixed sparse hash set memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename HashTraits, typename TS, uint64_t kCount>
struct FixedSparseHashSetMemory : public FixedSparseArrayMemory<SparseHashSetData<T, TS, typename HashTraits::HashType>, TBitBlock, TS, kCount> {
    using Super = FixedSparseArrayMemory<SparseHashSetData<T, TS, typename HashTraits::HashType>, TBitBlock, TS, kCount>;

    // sparse array configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType       = typename HashTraits::HashType;
    using HasherType     = typename HashTraits::HasherType;
    using SetDataType    = T;
    using SetStorageType = SparseHashSetData<T, TS, HashType>;

    // ctor & dtor
    inline FixedSparseHashSetMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
        clean_bucket();
    }
    inline ~FixedSparseHashSetMemory() noexcept
    {
        free_bucket();
    }

    // copy & move
    inline FixedSparseHashSetMemory(const FixedSparseHashSetMemory& rhs) noexcept
        : Super(rhs)
    {
        memory::copy(bucket(), rhs.bucket(), kBucketSize);
    }
    inline FixedSparseHashSetMemory(FixedSparseHashSetMemory&& rhs) noexcept
        : Super(std::move(rhs))
    {
        memory::copy(bucket(), rhs.bucket(), kBucketSize);
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const FixedSparseHashSetMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(rhs);
            memory::copy(bucket(), rhs.bucket(), kBucketSize);
        }
    }
    inline void operator=(FixedSparseHashSetMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(std::move(rhs));
            memory::copy(bucket(), rhs.bucket(), kBucketSize);
            rhs._reset();
        }
    }

    // memory operations
    inline bool resize_bucket() noexcept
    {
        // do noting
        return false;
    }
    inline void free_bucket() noexcept
    {
        // do noting
    }
    inline SizeType bucket_index(SizeType hash) const noexcept
    {
        return hash & kBucketMask;
    }
    inline void build_bucket() noexcept
    {
        if (Super::sparse_size() - Super::hole_size())
        {
            auto cursor = TrueBitCursor<BitBlockType, SizeType, false>::Begin(Super::bit_array(), Super::sparse_size());
            sparse_hash_set_build_bucket(Super::data(), bucket(), kBucketMask, cursor);
        }
    }
    inline void clean_bucket() noexcept
    {
        sparse_hash_set_clean_bucket(bucket(), kBucketSize);
    }

    // getter
    inline const SizeType* bucket() const noexcept { return _bucket_placeholder.data_typed(); }
    inline SizeType*       bucket() noexcept { return _bucket_placeholder.data_typed(); }

private:
    static constexpr SizeType npos        = npos_of<SizeType>;
    static constexpr SizeType kBucketSize = sparse_hash_set_calc_bucket_size(kCount);
    static constexpr SizeType kBucketMask = kBucketSize - 1;

    inline void _reset()
    {
        clean_bucket();
    }

private:
    Placeholder<SizeType, kBucketSize> _bucket_placeholder;
};
} // namespace skr::container

// inline sparse hash set memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename HashTraits, typename TS, uint64_t kInlineCount, typename Allocator>
struct InlineSparseHashSetMemory : public InlineSparseArrayMemory<SparseHashSetData<T, TS, typename HashTraits::HashType>, TBitBlock, TS, kInlineCount, Allocator> {
    using Super = InlineSparseArrayMemory<SparseHashSetData<T, TS, typename HashTraits::HashType>, TBitBlock, TS, kInlineCount, Allocator>;

    // sparse array configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType       = typename HashTraits::HashType;
    using HasherType     = typename HashTraits::HasherType;
    using SetDataType    = T;
    using SetStorageType = SparseHashSetData<T, TS, HashType>;

    // ctor & dtor
    inline InlineSparseHashSetMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
        clean_bucket();
    }
    inline ~InlineSparseHashSetMemory() noexcept
    {
        free_bucket();
    }

    // copy & move
    inline InlineSparseHashSetMemory(const InlineSparseHashSetMemory& rhs) noexcept
        : Super(rhs)
    {
        resize_bucket();
        clean_bucket();
        build_bucket();
    }
    inline InlineSparseHashSetMemory(InlineSparseHashSetMemory&& rhs) noexcept
        : Super(std::move(rhs))
    {
        if (rhs._is_using_inline_bucket())
        {
            memory::copy(bucket(), rhs._bucket_placeholder.data_typed(), rhs._bucket_size);
        }
        else
        {
            _heap_bucket = rhs._heap_bucket;
        }

        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const InlineSparseHashSetMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(rhs);
            resize_bucket();
            clean_bucket();
            build_bucket();
        }
    }
    inline void operator=(InlineSparseHashSetMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(std::move(rhs));

            // clean up self
            free_bucket();

            // move data
            if (rhs._is_using_inline_bucket())
            {
                memory::copy(bucket(), rhs.bucket(), rhs._bucket_size);
            }
            else
            {
                _heap_bucket = rhs._heap_bucket;
                _bucket_size = rhs._bucket_size;
                _bucket_mask = rhs._bucket_mask;
            }

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline bool resize_bucket() noexcept
    {
        SizeType new_bucket_size = sparse_hash_set_calc_bucket_size(Super::capacity());
        SKR_ASSERT(new_bucket_size > 0);
        if (new_bucket_size != _bucket_size)
        {
            if (new_bucket_size > kInlineBucketSize)
            {
                if (_is_using_inline_bucket()) // inline -> heap
                {
                    // alloc heap memory
                    SizeType* new_memory = Allocator::template alloc<SizeType>(new_bucket_size);

                    // needn't copy items here, because we always rehash after resize bucket
                    // memory::copy(new_memory, _bucket_placeholder.data_typed(), _bucket_size);

                    // update
                    _heap_bucket = new_memory;
                    _bucket_size = new_bucket_size;
                    _bucket_mask = new_bucket_size - 1;
                }
                else // heap -> heap
                {
                    if constexpr (memory::MemoryTraits<SizeType>::use_realloc && Allocator::support_realloc)
                    {
                        _heap_bucket = Allocator::template realloc<SizeType>(_heap_bucket, new_bucket_size);
                    }
                    else
                    {
                        // alloc new memory
                        SizeType* new_memory = Allocator::template alloc<SizeType>(new_bucket_size);

                        // needn't move items here, because we always rehash after resize bucket
                        // if (_bucket_size)
                        // {
                        //     memory::move(new_memory, _bucket, std::min(new_bucket_size, _bucket_size));
                        // }

                        // release old memory
                        Allocator::template free<SizeType>(_heap_bucket);

                        _heap_bucket = new_memory;
                    }

                    _bucket_size = new_bucket_size;
                    _bucket_mask = new_bucket_size - 1;
                }
            }
            else
            {
                if (_is_using_inline_bucket()) // inline -> inline
                {
                    // do noting
                }
                else // heap -> inline
                {
                    // needn't copy items here, because we always rehash after resize bucket

                    // just free memory
                    Allocator::template free<SizeType>(_heap_bucket);

                    // update
                    _bucket_size = kInlineBucketSize;
                    _bucket_mask = kInlineBucketMask;
                }
            }
            return true;
        }
        return false;
    }
    inline void free_bucket() noexcept
    {
        if (!_is_using_inline_bucket())
        {
            Allocator::template free<SizeType>(_heap_bucket);
            _bucket_size = kInlineBucketSize;
            _bucket_mask = kInlineBucketMask;
        }
    }
    inline SizeType bucket_index(SizeType hash) const noexcept
    {
        return hash & _bucket_mask;
    }
    inline void build_bucket() noexcept
    {
        if (Super::sparse_size() - Super::hole_size())
        {
            auto cursor = TrueBitCursor<BitBlockType, SizeType, false>::Begin(Super::bit_array(), Super::sparse_size());
            sparse_hash_set_build_bucket(Super::data(), bucket(), _bucket_mask, cursor);
        }
    }
    inline void clean_bucket() noexcept
    {
        sparse_hash_set_clean_bucket(bucket(), _bucket_size);
    }

    // getter
    inline const SizeType* bucket() const noexcept { return _is_using_inline_bucket() ? _bucket_placeholder.data_typed() : _heap_bucket; }
    inline SizeType*       bucket() noexcept { return _is_using_inline_bucket() ? _bucket_placeholder.data_typed() : _heap_bucket; }

private:
    static inline constexpr SizeType npos              = npos_of<SizeType>;
    static inline constexpr SizeType kInlineBucketSize = sparse_hash_set_calc_bucket_size(kInlineCount);
    static inline constexpr SizeType kInlineBucketMask = kInlineBucketSize - 1;

    inline bool _is_using_inline_bucket() const noexcept { return _bucket_size == kInlineBucketSize; }

    inline void _reset()
    {
        _bucket_size = kInlineBucketSize;
        _bucket_mask = kInlineBucketMask;
        clean_bucket();
    }

private:
    union
    {
        Placeholder<SizeType, kInlineBucketSize> _bucket_placeholder;
        SizeType*                                _heap_bucket;
    };
    SizeType _bucket_size = kInlineBucketSize;
    SizeType _bucket_mask = kInlineBucketMask;
};
} // namespace skr::container