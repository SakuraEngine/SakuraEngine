#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_traits.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_memory.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector_iterator.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_helper.hpp"

// generic fwd decl
namespace skr
{
struct GenericSparseHashBase;
}

// sparse hash set memory base
namespace skr::container
{
template <typename TSize>
struct SparseHashSetMemoryBase : public SparseVectorMemoryBase<TSize> {
    using SizeType = TSize;
    friend struct ::skr::GenericSparseHashBase;

private:
    using Super = SparseVectorMemoryBase<TSize>;
    inline void _reset()
    {
        Super::_reset();
        _bucket      = nullptr;
        _bucket_size = 0;
        _bucket_mask = 0;
    }

protected:
    SizeType* _bucket      = nullptr;
    SizeType  _bucket_size = 0;
    SizeType  _bucket_mask = 0;
};
} // namespace skr::container

// util sparse hash set memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename HashTraits, typename Base, typename Allocator>
struct SparseHashSetMemory : public SparseVectorMemory<SparseHashSetStorage<T, typename Base::SizeType, typename HashTraits::HashType>, TBitBlock, Base, Allocator> {
    using Super = SparseVectorMemory<SparseHashSetStorage<T, typename Base::SizeType, typename HashTraits::HashType>, TBitBlock, Base, Allocator>;

    // sparse vector configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType       = typename HashTraits::HashType;
    using HasherType     = typename HashTraits::HasherType;
    using SetDataType    = T;
    using SetStorageType = SparseHashSetStorage<T, SizeType, HashType>;

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
            Base::_bucket      = rhs._bucket;
            Base::_bucket_size = rhs._bucket_size;
            Base::_bucket_mask = rhs._bucket_mask;

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline bool resize_bucket() noexcept
    {
        SizeType new_bucket_size = sparse_hash_set_calc_bucket_size(Super::capacity());
        if (new_bucket_size != Base::_bucket_size)
        {
            if (new_bucket_size)
            {
                if constexpr (::skr::memory::MemoryTraits<SizeType>::use_realloc && Allocator::support_realloc)
                {
                    Base::_bucket = Allocator::template realloc<SizeType>(bucket(), new_bucket_size);
                }
                else
                {
                    // alloc new memory
                    SizeType* new_memory = Allocator::template alloc<SizeType>(new_bucket_size);

                    // release old memory
                    Allocator::template free<SizeType>(bucket());

                    Base::_bucket = new_memory;
                }

                Base::_bucket_size = new_bucket_size;
                Base::_bucket_mask = new_bucket_size - 1;
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
        if (Base::_bucket)
        {
            Allocator::template free<SizeType>(bucket());
            Base::_bucket      = nullptr;
            Base::_bucket_size = 0;
            Base::_bucket_mask = 0;
        }
    }
    inline SizeType bucket_index(SizeType hash) const noexcept
    {
        return hash & Base::_bucket_mask;
    }
    inline void build_bucket() noexcept
    {
        if (Super::sparse_size() - Super::hole_size())
        {
            auto cursor = TrueBitCursor<BitBlockType, SizeType, false>::Begin(Super::bit_data(), Super::sparse_size());
            sparse_hash_set_build_bucket(Super::data(), bucket(), Base::_bucket_mask, cursor);
        }
    }
    inline void clean_bucket() noexcept
    {
        sparse_hash_set_clean_bucket(bucket(), Base::_bucket_size);
    }

    // getter
    inline const SizeType* bucket() const noexcept { return reinterpret_cast<const SizeType*>(Base::_bucket); }
    inline SizeType*       bucket() noexcept { return reinterpret_cast<SizeType*>(Base::_bucket); }

private:
    static inline constexpr SizeType npos = npos_of<SizeType>;

    inline void _reset() noexcept
    {
        Base::_bucket      = nullptr;
        Base::_bucket_size = 0;
        Base::_bucket_mask = 0;
    }
};
} // namespace skr::container

// fixed sparse hash set memory
namespace skr::container
{
template <typename T, typename TBitBlock, typename HashTraits, uint64_t kCount, typename Base>
struct FixedSparseHashSetMemory : public FixedSparseVectorMemory<SparseHashSetStorage<T, typename Base::SizeType, typename HashTraits::HashType>, TBitBlock, kCount, Base> {
    using Super = FixedSparseVectorMemory<SparseHashSetStorage<T, typename Base::SizeType, typename HashTraits::HashType>, TBitBlock, kCount, Base>;

    // sparse vector configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType       = typename HashTraits::HashType;
    using HasherType     = typename HashTraits::HasherType;
    using SetDataType    = T;
    using SetStorageType = SparseHashSetStorage<T, SizeType, HashType>;

    // ctor & dtor
    inline FixedSparseHashSetMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
        _init_setup();
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
        _init_setup();
        ::skr::memory::copy(bucket(), rhs.bucket(), kBucketSize);
    }
    inline FixedSparseHashSetMemory(FixedSparseHashSetMemory&& rhs) noexcept
        : Super(std::move(rhs))
    {
        _init_setup();
        ::skr::memory::copy(bucket(), rhs.bucket(), kBucketSize);
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const FixedSparseHashSetMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(rhs);
            ::skr::memory::copy(bucket(), rhs.bucket(), kBucketSize);
        }
    }
    inline void operator=(FixedSparseHashSetMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Super::operator=(std::move(rhs));
            ::skr::memory::copy(bucket(), rhs.bucket(), kBucketSize);
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
            auto cursor = TrueBitCursor<BitBlockType, SizeType, false>::Begin(Super::bit_data(), Super::sparse_size());
            sparse_hash_set_build_bucket(Super::data(), bucket(), kBucketMask, cursor);
        }
    }
    inline void clean_bucket() noexcept
    {
        sparse_hash_set_clean_bucket(bucket(), kBucketSize);
    }

    // getter
    inline const SizeType* bucket() const noexcept { return reinterpret_cast<const SizeType*>(Base::_bucket); }
    inline SizeType*       bucket() noexcept { return reinterpret_cast<SizeType*>(Base::_bucket); }

private:
    static constexpr SizeType npos        = npos_of<SizeType>;
    static constexpr SizeType kBucketSize = sparse_hash_set_calc_bucket_size(kCount);
    static constexpr SizeType kBucketMask = kBucketSize - 1;

    inline void _init_setup()
    {
        Base::_bucket      = _bucket_placeholder.data_typed();
        Base::_bucket_size = kBucketSize;
        Base::_bucket_mask = kBucketMask;
    }

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
template <typename T, typename TBitBlock, typename HashTraits, uint64_t kInlineCount, typename Base, typename Allocator>
struct InlineSparseHashSetMemory : public InlineSparseVectorMemory<SparseHashSetStorage<T, typename Base::SizeType, typename HashTraits::HashType>, TBitBlock, kInlineCount, Base, Allocator> {
    using Super = InlineSparseVectorMemory<SparseHashSetStorage<T, typename Base::SizeType, typename HashTraits::HashType>, TBitBlock, kInlineCount, Base, Allocator>;

    // sparse data configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using HashType       = typename HashTraits::HashType;
    using HasherType     = typename HashTraits::HasherType;
    using SetDataType    = T;
    using SetStorageType = SparseHashSetStorage<T, SizeType, HashType>;

    // ctor & dtor
    inline InlineSparseHashSetMemory(AllocatorCtorParam param) noexcept
        : Super(std::move(param))
    {
        _reset();
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
        _reset();
        resize_bucket();
        clean_bucket();
        build_bucket();
    }
    inline InlineSparseHashSetMemory(InlineSparseHashSetMemory&& rhs) noexcept
        : Super(std::move(rhs))
    {
        _reset();
        if (rhs._is_using_inline_bucket())
        {
            ::skr::memory::copy(bucket(), rhs._bucket_placeholder.data_typed(), rhs._bucket_size);
        }
        else
        {
            Base::_bucket = rhs._bucket;
        }

        rhs._reset_and_clean();
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
                ::skr::memory::copy(bucket(), rhs.bucket(), rhs._bucket_size);
            }
            else
            {
                Base::_bucket      = rhs._bucket;
                Base::_bucket_size = rhs._bucket_size;
                Base::_bucket_mask = rhs._bucket_mask;
            }

            // reset rhs
            rhs._reset_and_clean();
        }
    }

    // memory operations
    inline bool resize_bucket() noexcept
    {
        SizeType new_bucket_size = sparse_hash_set_calc_bucket_size(Super::capacity());
        SKR_ASSERT(new_bucket_size > 0);
        if (new_bucket_size != Base::_bucket_size)
        {
            if (new_bucket_size > kInlineBucketSize)
            {
                if (_is_using_inline_bucket()) // inline -> heap
                {
                    // alloc heap memory
                    SizeType* new_memory = Allocator::template alloc<SizeType>(new_bucket_size);

                    // needn't copy items here, because we always rehash after resize bucket
                    // ::skr::memory::copy(new_memory, _bucket_placeholder.data_typed(), _bucket_size);

                    // update
                    Base::_bucket      = new_memory;
                    Base::_bucket_size = new_bucket_size;
                    Base::_bucket_mask = new_bucket_size - 1;
                }
                else // heap -> heap
                {
                    if constexpr (::skr::memory::MemoryTraits<SizeType>::use_realloc && Allocator::support_realloc)
                    {
                        Base::_bucket = Allocator::template realloc<SizeType>(bucket(), new_bucket_size);
                    }
                    else
                    {
                        // alloc new memory
                        SizeType* new_memory = Allocator::template alloc<SizeType>(new_bucket_size);

                        // needn't move items here, because we always rehash after resize bucket
                        // if (_bucket_size)
                        // {
                        //     ::skr::memory::move(new_memory, _bucket, std::min(new_bucket_size, _bucket_size));
                        // }

                        // release old memory
                        Allocator::template free<SizeType>(bucket());

                        Base::_bucket = new_memory;
                    }

                    Base::_bucket_size = new_bucket_size;
                    Base::_bucket_mask = new_bucket_size - 1;
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
                    Allocator::template free<SizeType>(bucket());

                    _reset();
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
            Allocator::template free<SizeType>(bucket());
            _reset_and_clean();
        }
    }
    inline SizeType bucket_index(SizeType hash) const noexcept
    {
        return hash & Base::_bucket_mask;
    }
    inline void build_bucket() noexcept
    {
        if (Super::sparse_size() - Super::hole_size())
        {
            auto cursor = TrueBitCursor<BitBlockType, SizeType, false>::Begin(Super::bit_data(), Super::sparse_size());
            sparse_hash_set_build_bucket(Super::data(), bucket(), Base::_bucket_mask, cursor);
        }
    }
    inline void clean_bucket() noexcept
    {
        sparse_hash_set_clean_bucket(bucket(), Base::_bucket_size);
    }

    // getter
    inline const SizeType* bucket() const noexcept { return reinterpret_cast<const SizeType*>(Base::_bucket); }
    inline SizeType*       bucket() noexcept { return reinterpret_cast<SizeType*>(Base::_bucket); }

private:
    static inline constexpr SizeType npos              = npos_of<SizeType>;
    static inline constexpr SizeType kInlineBucketSize = sparse_hash_set_calc_bucket_size(kInlineCount);
    static inline constexpr SizeType kInlineBucketMask = kInlineBucketSize - 1;

    inline bool _is_using_inline_bucket() const noexcept { return Base::_bucket == _bucket_placeholder.data_typed(); }

    inline void _reset()
    {
        Base::_bucket      = _bucket_placeholder.data_typed();
        Base::_bucket_size = kInlineBucketSize;
        Base::_bucket_mask = kInlineBucketMask;
    }

    inline void _reset_and_clean()
    {
        _reset();
        clean_bucket();
    }

private:
    Placeholder<SizeType, kInlineBucketSize> _bucket_placeholder;
};
} // namespace skr::container