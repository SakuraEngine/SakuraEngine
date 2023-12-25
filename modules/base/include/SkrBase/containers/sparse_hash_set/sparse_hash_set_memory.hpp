#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/key_traits.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_memory.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"

// helpers
namespace skr::container
{
template <typename TS>
inline constexpr TS sparse_hash_set_calc_bucket_size(TS capacity) noexcept
{
    constexpr TS min_size_to_hash    = 4;
    constexpr TS basic_bucket_size   = 8;
    constexpr TS avg_bucket_capacity = 2;

    if (capacity >= min_size_to_hash)
    {
        return bit_ceil(TS(capacity / avg_bucket_capacity) + basic_bucket_size);
    }
    else if (capacity)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
} // namespace skr::container

// util sparse hash set memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename TS, typename Allocator>
struct SparseHashSetMemory : public SparseArrayMemory<SparseHashSetData<T, TS, THash>, TBitBlock, TS, Allocator> {
    using Super = SparseArrayMemory<SparseHashSetData<T, TS, THash>, TBitBlock, TS, Allocator>;

    // sparse array configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType                        = THash;
    using HasherType                      = THasher;
    using KeyType                         = typename KeyTraits<T>::KeyType;
    using KeyMapperType                   = typename KeyTraits<T>::KeyMapperType;
    using ComparerType                    = TComparer;
    using SetDataType                     = T;
    using SetStorageType                  = SparseHashSetData<T, TS, THash>;
    static constexpr bool allow_multi_key = AllowMultiKey;

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
    inline SparseHashSetMemory(const SparseHashSetMemory& other, AllocatorCtorParam param) noexcept
        : Super(other, std::move(param))
    {
        if (other._bucket_size)
        {
            _realloc_bucket(other._bucket_size);
            memory::copy(_bucket, other._bucket, other._bucket_size);
            _bucket_size = other._bucket_size;
            _bucket_mask = other._bucket_mask;
        }
    }
    inline SparseHashSetMemory(SparseHashSetMemory&& other) noexcept
        : Super(std::move(other))
        , _bucket(other._bucket)
        , _bucket_size(other._bucket_size)
        , _bucket_mask(other._bucket_mask)
    {
        other._bucket      = nullptr;
        other._bucket_size = 0;
        other._bucket_mask = 0;
    }

    // assign & move assign
    inline void operator=(const SparseHashSetMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(rhs);

            // copy bucket
            if (rhs._bucket_size)
            {
                if (_bucket_size != rhs._bucket_size)
                {
                    _realloc_bucket(rhs._bucket_size);
                }
                memory::copy(_bucket, rhs._bucket, _bucket_size);
                _bucket_size = rhs._bucket_size;
                _bucket_mask = rhs._bucket_mask;
            }
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
            _bucket          = rhs._bucket;
            _bucket_size     = rhs._bucket_size;
            _bucket_mask     = rhs._bucket_mask;
            rhs._bucket      = nullptr;
            rhs._bucket_size = 0;
            rhs._bucket_mask = 0;
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
                _realloc_bucket(new_bucket_size);
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
    inline void clean_bucket() noexcept
    {
        if (_bucket)
        {
            for (SizeType i = 0; i < _bucket_size; ++i)
            {
                _bucket[i] = npos;
            }
        }
    }
    inline SizeType bucket_index(SizeType hash) const noexcept
    {
        return hash & _bucket_mask;
    }
    inline bool need_rehash() const noexcept
    {
        return (Super::sparse_size() - Super::hole_size()) && sparse_hash_set_calc_bucket_size(Super::capacity()) != _bucket_size;
    }

    // getter
    inline const SizeType* bucket() const noexcept { return _bucket; }
    inline SizeType*       bucket() noexcept { return _bucket; }

private:
    static inline constexpr SizeType npos = npos_of<SizeType>;

    inline void _realloc_bucket(SizeType capacity)
    {
        SKR_ASSERT(pop_count(capacity) == 1 && "capacity must be power of 2");

        if constexpr (memory::MemoryTraits<SizeType>::use_realloc && Allocator::support_realloc)
        {
            _bucket = Allocator::template realloc<SizeType>(_bucket, capacity);
        }
        else
        {
            // alloc new memory
            SizeType* new_memory = Allocator::template alloc<SizeType>(capacity);

            // needn't move items here, because we always rehash after resize bucket
            // if (_bucket_size)
            // {
            //     memory::move(new_memory, _bucket, std::min(new_bucket_size, _bucket_size));
            // }

            // release old memory
            Allocator::template free<SizeType>(_bucket);

            _bucket = new_memory;
        }

        _bucket_size = capacity;
        _bucket_mask = capacity - 1;
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename TS, uint64_t kCount>
struct FixedSparseHashSetMemory : public FixedSparseArrayMemory<SparseHashSetData<T, TS, THash>, TBitBlock, TS, kCount> {
    using Super = FixedSparseArrayMemory<SparseHashSetData<T, TS, THash>, TBitBlock, TS, kCount>;

    // sparse array configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType                        = THash;
    using HasherType                      = THasher;
    using KeyType                         = typename KeyTraits<T>::KeyType;
    using KeyMapperType                   = typename KeyTraits<T>::KeyMapperType;
    using ComparerType                    = TComparer;
    using SetDataType                     = T;
    using SetStorageType                  = SparseHashSetData<T, TS, THash>;
    static constexpr bool allow_multi_key = AllowMultiKey;

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
    inline FixedSparseHashSetMemory(const FixedSparseHashSetMemory& other, AllocatorCtorParam param) noexcept
        : Super(other, std::move(param))
    {
        memory::copy(bucket(), other.bucket(), kBucketSize);
    }
    inline FixedSparseHashSetMemory(FixedSparseHashSetMemory&& other) noexcept
        : Super(std::move(other))
    {
        memory::copy(bucket(), other.bucket(), kBucketSize);
        other.clean_bucket();
    }

    // assign & move assign
    inline void
    operator=(const FixedSparseHashSetMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(rhs);

            // copy bucket
            memory::copy(bucket(), rhs.bucket(), kBucketSize);
        }
    }
    inline void operator=(FixedSparseHashSetMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(std::move(rhs));

            // copy bucket
            memory::copy(bucket(), rhs.bucket(), kBucketSize);
            rhs.clean_bucket();
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
    inline void clean_bucket() noexcept
    {
        for (SizeType i = 0; i < kBucketSize; ++i)
        {
            bucket()[i] = npos;
        }
    }
    inline SizeType bucket_index(SizeType hash) const noexcept
    {
        return hash & kBucketMask;
    }
    inline bool need_rehash() const noexcept
    {
        return false;
    }

    // getter
    inline const SizeType* bucket() const noexcept { return _bucket_placeholder.data_typed(); }
    inline SizeType*       bucket() noexcept { return _bucket_placeholder.data_typed(); }

private:
    static constexpr SizeType npos        = npos_of<SizeType>;
    static constexpr SizeType kBucketSize = sparse_hash_set_calc_bucket_size(kCount);
    static constexpr SizeType kBucketMask = kBucketSize - 1;

private:
    Placeholder<SizeType, kBucketSize> _bucket_placeholder;
};
} // namespace skr::container

// inline sparse hash set memory
namespace skr::container
{
}