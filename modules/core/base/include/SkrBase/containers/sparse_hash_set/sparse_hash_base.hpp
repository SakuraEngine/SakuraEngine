#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "sparse_hash_set_iterator.hpp"
#include "SkrBase/containers/sparse_vector/sparse_vector.hpp"
#include "SkrBase/containers/misc/transparent.hpp"

namespace skr::container
{
template <typename Memory>
struct SparseHashBase : protected SparseVector<Memory> {
    using Super = SparseVector<Memory>;

    // sparse vector configure
    using typename Memory::SizeType;
    using typename Memory::DataType;
    using typename Memory::StorageType;
    using typename Memory::BitBlockType;
    using typename Memory::AllocatorCtorParam;

    // sparse hash set configure
    using typename Memory::HashType;
    using typename Memory::HasherType;
    using typename Memory::SetDataType;
    using typename Memory::SetStorageType;

    // helper
    using DataVector                      = SparseVector<Memory>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // ctor & dtor
    SparseHashBase(AllocatorCtorParam param = {});
    ~SparseHashBase();

    // copy & move
    SparseHashBase(const SparseHashBase& rhs);
    SparseHashBase(SparseHashBase&& rhs) noexcept;

    // assign & move assign
    void operator=(const SparseHashBase& rhs);
    void operator=(SparseHashBase&& rhs) noexcept;

    // getter
    SizeType          size() const;
    SizeType          capacity() const;
    SizeType          slack() const;
    SizeType          sparse_size() const;
    SizeType          hole_size() const;
    SizeType          bit_size() const;
    SizeType          freelist_head() const;
    bool              is_compact() const;
    bool              is_empty() const;
    DataVector&       data_vector();
    const DataVector& data_vector() const;
    SizeType*         bucket();
    const SizeType*   bucket() const;
    Memory&           memory();
    const Memory&     memory() const;

    // validator
    bool has_data(SizeType idx) const;
    bool is_hole(SizeType idx) const;
    bool is_valid_index(SizeType idx) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    bool compact();
    bool compact_stable();
    bool compact_top();

    // rehash
    void rehash();
    bool rehash_if_need();

    // visitor
    const SetDataType& at(SizeType index) const;
    const SetDataType& at_last(SizeType index = 0) const;

    // sort
    template <typename Functor = Less<SetDataType>>
    void sort(Functor&& p = {});
    template <typename Functor = Less<SetDataType>>
    void sort_stable(Functor&& p = {});

    // remove
    void remove_at(SizeType index);
    void remove_at_unsafe(SizeType index);
    template <typename Pred>
    bool remove_if(Pred&& pred);
    template <typename Pred>
    bool remove_last_if(Pred&& pred);
    template <typename Pred>
    SizeType remove_all_if(Pred&& pred);

    // contains
    template <typename Pred>
    bool contains_if(Pred&& pred) const;
    template <typename Pred>
    SizeType count_if(Pred&& pred) const;

protected:
    // basic add/find/remove
    template <typename DataRef>
    DataRef _add_unsafe(HashType hash);
    template <typename DataRef, typename Pred>
    DataRef _find(HashType hash, Pred&& pred) const;
    template <typename DataRef, typename Pred>
    DataRef _find_next(DataRef ref, Pred&& pred) const;
    template <typename Pred>
    bool _remove(HashType hash, Pred&& pred);
    template <typename Pred>
    SizeType _remove_all(HashType hash, Pred&& pred);

    // template find_if
    template <typename DataRef, typename Pred>
    DataRef _find_if(Pred&& pred) const;
    template <typename DataRef, typename Pred>
    DataRef _find_last_if(Pred&& pred) const;

    // helpers
    SizeType _bucket_index(SizeType hash) const; // get bucket data index by hash
    void     _clean_bucket();                    // remove all elements from bucket
    bool     _resize_bucket();                   // resize hash bucket
    void     _build_bucket();                    // build hash bucket
    bool     _is_in_bucket(SizeType index) const;
    void     _add_to_bucket(const SetStorageType& data, SizeType index);
    void     _remove_from_bucket(SizeType index);
};
} // namespace skr::container

namespace skr::container
{
// helpers
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::_bucket_index(SizeType hash) const
{
    return Memory::bucket_index(hash);
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::_clean_bucket()
{
    Memory::clean_bucket();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::_resize_bucket()
{
    return Memory::resize_bucket();
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::_build_bucket()
{
    Memory::build_bucket();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::_is_in_bucket(SizeType index) const
{
    if (has_data(index))
    {
        const auto& node         = Super::at(index);
        auto        search_index = bucket()[_bucket_index(node._sparse_hash_set_hash)];

        while (search_index != npos)
        {
            if (search_index == index)
            {
                return true;
            }
            search_index = Super::at(search_index)._sparse_hash_set_next;
        }
        return false;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::_add_to_bucket(const SetStorageType& data, SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(!bucket() || !_is_in_bucket(index));

    if (!rehash_if_need())
    {
        SizeType& index_ref        = bucket()[_bucket_index(data._sparse_hash_set_hash)];
        data._sparse_hash_set_next = index_ref;
        index_ref                  = index;
    }
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::_remove_from_bucket(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(_is_in_bucket(index));

    SizeType* p_next_idx = &bucket()[_bucket_index(Super::at(index)._sparse_hash_set_hash)];

    while (*p_next_idx != npos)
    {
        auto& next_node = Super::at(*p_next_idx);

        if (*p_next_idx == index)
        {
            // update link
            *p_next_idx = next_node._sparse_hash_set_next;
            break;
        }
        else
        {
            // move to next node
            p_next_idx = &next_node._sparse_hash_set_next;
        }
    }
}

// ctor & dtor
template <typename Memory>
SKR_INLINE SparseHashBase<Memory>::SparseHashBase(AllocatorCtorParam param)
    : Super(std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashBase<Memory>::~SparseHashBase()
{
    // handled in memory
}

// copy & move
template <typename Memory>
SKR_INLINE SparseHashBase<Memory>::SparseHashBase(const SparseHashBase& rhs)
    : Super(rhs)
{
}
template <typename Memory>
SKR_INLINE SparseHashBase<Memory>::SparseHashBase(SparseHashBase&& rhs) noexcept
    : Super(std::move(rhs))
{
}

// assign & move assign
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::operator=(const SparseHashBase& rhs)
{
    Super::operator=(rhs);
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::operator=(SparseHashBase&& rhs) noexcept
{
    Super::operator=(std::move(rhs));
}

// getter
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::size() const
{
    return Super::size();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::capacity() const
{
    return Super::capacity();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::slack() const
{
    return Super::slack();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::sparse_size() const
{
    return Super::sparse_size();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::hole_size() const
{
    return Super::hole_size();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::bit_size() const
{
    return Super::bit_size();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::freelist_head() const
{
    return Super::freelist_head();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::is_compact() const
{
    return Super::is_compact();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::is_empty() const
{
    return Super::is_empty();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::DataVector& SparseHashBase<Memory>::data_vector()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const typename SparseHashBase<Memory>::DataVector& SparseHashBase<Memory>::data_vector() const
{
    return *this;
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType* SparseHashBase<Memory>::bucket()
{
    return Memory::bucket();
}
template <typename Memory>
SKR_INLINE const typename SparseHashBase<Memory>::SizeType* SparseHashBase<Memory>::bucket() const
{
    return Memory::bucket();
}
template <typename Memory>
SKR_INLINE Memory& SparseHashBase<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& SparseHashBase<Memory>::memory() const
{
    return *this;
}

// validator
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::has_data(SizeType idx) const
{
    return Super::has_data(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::is_hole(SizeType idx) const
{
    return Super::is_hole(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::is_valid_index(SizeType idx) const
{
    return Super::is_valid_index(idx);
}

// memory op
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::clear()
{
    Super::clear();
    _clean_bucket();
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::release(SizeType capacity)
{
    Super::release(capacity);
    _resize_bucket();
    _clean_bucket();
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::reserve(SizeType capacity)
{
    Super::reserve(capacity);
    rehash_if_need();
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::shrink()
{
    Super::shrink();
    rehash_if_need();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::compact()
{
    if (Super::compact())
    {
        rehash();
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::compact_stable()
{
    if (Super::compact_stable())
    {
        rehash();
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::compact_top()
{
    return Super::compact_top();
}

// rehash
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::rehash()
{
    _resize_bucket();
    _clean_bucket();
    _build_bucket();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::rehash_if_need()
{
    if (_resize_bucket())
    {
        _clean_bucket();
        _build_bucket();
        return true;
    }
    return false;
}

// visitor
template <typename Memory>
SKR_INLINE const typename SparseHashBase<Memory>::SetDataType& SparseHashBase<Memory>::at(SizeType index) const
{
    return Super::at(index)._sparse_hash_set_data;
}
template <typename Memory>
SKR_INLINE const typename SparseHashBase<Memory>::SetDataType& SparseHashBase<Memory>::at_last(SizeType index) const
{
    return Super::at_last(index)._sparse_hash_set_data;
}

// sort
template <typename Memory>
template <typename Functor>
SKR_INLINE void SparseHashBase<Memory>::sort(Functor&& p)
{
    data_vector().sort([&](const SetStorageType& a, const SetStorageType& b) {
        return p(a._sparse_hash_set_data, b._sparse_hash_set_data);
    });
    rehash();
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void SparseHashBase<Memory>::sort_stable(Functor&& p)
{
    data_vector().sort_stable([&](const SetStorageType& a, const SetStorageType& b) {
        return p(a._sparse_hash_set_data, b._sparse_hash_set_data);
    });
    rehash();
}

// basic add/find/remove
template <typename Memory>
template <typename DataRef>
SKR_INLINE DataRef SparseHashBase<Memory>::_add_unsafe(HashType hash)
{
    auto data_arr_ref                        = Super::add_unsafe();
    data_arr_ref.ref()._sparse_hash_set_hash = hash;
    _add_to_bucket(data_arr_ref.ref(), data_arr_ref.index());
    return {
        &data_arr_ref.ref()._sparse_hash_set_data,
        data_arr_ref.index(),
        hash,
        false
    };
}
template <typename Memory>
template <typename DataRef, typename Pred>
SKR_INLINE DataRef SparseHashBase<Memory>::_find(HashType hash, Pred&& pred) const
{
    if (!bucket()) return {
        nullptr,
        npos,
        hash,
        false
    };

    SizeType search_index = bucket()[_bucket_index(hash)];
    while (search_index != npos)
    {
        auto& node = Super::at(search_index);
        if (node._sparse_hash_set_hash == hash && pred(node._sparse_hash_set_data))
        {
            return {
                const_cast<SetDataType*>(&node._sparse_hash_set_data),
                search_index,
                hash,
                false
            };
        }
        search_index = node._sparse_hash_set_next;
    }
    return {
        nullptr,
        npos,
        hash,
        false
    };
}
template <typename Memory>
template <typename DataRef, typename Pred>
SKR_INLINE DataRef SparseHashBase<Memory>::_find_next(DataRef ref, Pred&& pred) const
{
    if (!bucket() || !ref.is_valid()) return {
        nullptr,
        npos,
        ref.hash(),
        false
    };

    SizeType search_index = Super::at(ref.index())._sparse_hash_set_next;
    while (search_index != npos)
    {
        auto& node = Super::at(search_index);
        if (node._sparse_hash_set_hash == ref.hash() && pred(node._sparse_hash_set_data))
        {
            return {
                const_cast<SetDataType*>(&node._sparse_hash_set_data),
                search_index,
                ref.hash(),
                false
            };
        }
        search_index = node._sparse_hash_set_next;
    }
    return {
        nullptr,
        npos,
        ref.hash(),
        false
    };
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseHashBase<Memory>::_remove(HashType hash, Pred&& pred)
{
    if (!bucket()) return false;

    SizeType* p_next_idx = bucket() + _bucket_index(hash);
    while (*p_next_idx != npos)
    {
        auto& next_node = Super::at(*p_next_idx);

        if (next_node._sparse_hash_set_hash == hash && pred(next_node._sparse_hash_set_data))
        {
            // update link
            SizeType index = *p_next_idx;
            *p_next_idx    = next_node._sparse_hash_set_next;

            // remove item
            Super::remove_at(index);
            return true;
        }
        p_next_idx = &next_node._sparse_hash_set_next;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::_remove_all(HashType hash, Pred&& pred)
{
    if (is_empty()) return 0;

    SizeType count = 0;

    SizeType* p_next_idx = bucket() + _bucket_index(hash);

    while (*p_next_idx != npos)
    {
        auto& next_node = Super::at(*p_next_idx);

        if (next_node._sparse_hash_set_hash == hash && pred(next_node._sparse_hash_set_data))
        {
            // update link
            SizeType index = *p_next_idx;
            *p_next_idx    = next_node._sparse_hash_set_next;

            // remove item
            Super::remove_at(index);
            ++count;
        }
        else
        {
            // move to next node
            p_next_idx = &next_node._sparse_hash_set_next;
        }
    }

    return count;
}

// remove
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::remove_at(SizeType index)
{
    _remove_from_bucket(index);
    Super::remove_at(index);
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::remove_at_unsafe(SizeType index)
{
    _remove_from_bucket(index);
    Super::remove_at_unsafe(index);
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseHashBase<Memory>::remove_if(Pred&& pred)
{
    if (auto ref = Super::find_if([&pred](const SetStorageType& data) { return pred(data._sparse_hash_set_data); }))
    {
        remove_at(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseHashBase<Memory>::remove_last_if(Pred&& pred)
{
    if (auto ref = Super::find_last_if([&pred](const SetStorageType& data) { return pred(data._sparse_hash_set_data); }))
    {
        remove_at(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::remove_all_if(Pred&& pred)
{
    SizeType count = 0;
    for (auto cursor = Super::cursor_begin(); !cursor.reach_end();)
    {
        if (pred(cursor.ref()._sparse_hash_set_data))
        {
            _remove_from_bucket(cursor.index());
            cursor.erase_and_move_next();
            ++count;
        }
        else
        {
            cursor.move_next();
        }
    }
    return count;
}

// template find_if
template <typename Memory>
template <typename DataRef, typename Pred>
DataRef SparseHashBase<Memory>::_find_if(Pred&& pred) const
{
    if (auto ref = Super::find_if([&pred](const SetStorageType& data) { return pred(data._sparse_hash_set_data); }))
    {
        return {
            const_cast<SetDataType*>(&ref.ref()._sparse_hash_set_data),
            ref.index(),
            ref.ref()._sparse_hash_set_hash,
            false,
        };
    }
    else
    {
        return {};
    }
}
template <typename Memory>
template <typename DataRef, typename Pred>
DataRef SparseHashBase<Memory>::_find_last_if(Pred&& pred) const
{
    if (auto ref = Super::find_last_if([&pred](const SetStorageType& data) { return pred(data._sparse_hash_set_data); }))
    {
        return {
            const_cast<SetDataType*>(&ref.ref()._sparse_hash_set_data),
            ref.index(),
            ref.ref()._sparse_hash_set_hash,
            false,
        };
    }
    else
    {
        return {};
    }
}

// constains
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseHashBase<Memory>::contains_if(Pred&& pred) const
{
    return Super::contains_if([&pred](const SetStorageType& data) { return pred(data._sparse_hash_set_data); });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::count_if(Pred&& pred) const
{
    return Super::count_if([&pred](const SetStorageType& data) { return pred(data._sparse_hash_set_data); });
}

} // namespace skr::container