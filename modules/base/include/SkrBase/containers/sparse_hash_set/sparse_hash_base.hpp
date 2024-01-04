#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "sparse_hash_set_iterator.hpp"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"
#include "SkrBase/containers/concepts.h"

namespace skr::container
{
template <typename Memory>
struct SparseHashBase : protected SparseArray<Memory> {
    using Super = SparseArray<Memory>;

    // sparse array configure
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
    using DataArr                         = SparseArray<Memory>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref & iterator
    using DataRef  = SparseHashSetDataRef<SetDataType, SizeType, HashType, false>;
    using CDataRef = SparseHashSetDataRef<SetDataType, SizeType, HashType, true>;
    using It       = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, false>;
    using CIt      = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, true>;

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
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    SizeType        sparse_size() const;
    SizeType        hole_size() const;
    SizeType        bit_array_size() const;
    SizeType        free_list_head() const;
    bool            is_compact() const;
    bool            empty() const;
    DataArr&        data_arr();
    const DataArr&  data_arr() const;
    SizeType*       bucket();
    const SizeType* bucket() const;
    Memory&         memory();
    const Memory&   memory() const;

    // validator
    bool has_data(SizeType idx) const;
    bool is_hole(SizeType idx) const;
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const SetDataType* p) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    bool compact();
    bool compact_stable();
    bool compact_top();

    // rehash
    bool sync_hash();
    bool sync_hash_at(SizeType index);
    void rehash();
    bool rehash_if_need();

    // visitor
    const SetDataType& at(SizeType index) const;
    const SetDataType& last(SizeType index = 0) const;

protected:
    // basic add/find/remove
    DataRef _add_unsafe(HashType hash);
    template <typename Pred>
    CDataRef _find(HashType hash, Pred&& pred) const;
    template <typename Pred>
    CDataRef _find_next(DataRef ref, Pred&& pred) const;
    template <typename Pred>
    bool _remove(HashType hash, Pred&& pred);
    template <typename Pred>
    bool _remove_next(DataRef ref, Pred&& pred);
    template <typename Pred>
    SizeType _remove_all(HashType hash, Pred&& pred);
    void     _remove_at(SizeType index);

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
        auto& next_data = Super::at(*p_next_idx);

        if (*p_next_idx == index)
        {
            // update link
            *p_next_idx = next_data._sparse_hash_set_next;
            break;
        }
        else
        {
            // move to next node
            p_next_idx = &next_data._sparse_hash_set_next;
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
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::bit_array_size() const
{
    return Super::bit_array_size();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::free_list_head() const
{
    return Super::free_list_head();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::is_compact() const
{
    return Super::is_compact();
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::empty() const
{
    return Super::empty();
}
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::DataArr& SparseHashBase<Memory>::data_arr()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const typename SparseHashBase<Memory>::DataArr& SparseHashBase<Memory>::data_arr() const
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
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::is_valid_pointer(const SetDataType* p) const
{
    return Super::is_valid_pointer(p);
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
SKR_INLINE bool SparseHashBase<Memory>::sync_hash()
{
    bool hash_changed = false;
    for (auto it = Super::begin(); it != Super::end(); ++it)
    {
        HashType new_hash = HasherType()(it->_sparse_hash_set_data);
        if (new_hash != it->_sparse_hash_set_hash)
        {
            it->_sparse_hash_set_hash = new_hash;
            hash_changed              = true;
        }
    }

    if (hash_changed)
    {
        _clean_bucket();
        _build_bucket();
    }

    return hash_changed;
}
template <typename Memory>
SKR_INLINE bool SparseHashBase<Memory>::sync_hash_at(SizeType index)
{
    auto&    data     = Super::at(index);
    HashType new_hash = HasherType()(data._sparse_hash_set_data);
    if (new_hash != data._sparse_hash_set_hash)
    {
        data._sparse_hash_set_hash = new_hash;
        _remove_from_bucket(index);
        _add_to_bucket(data, index);
        return true;
    }
    return false;
}
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
SKR_INLINE const typename SparseHashBase<Memory>::SetDataType& SparseHashBase<Memory>::last(SizeType index) const
{
    return Super::last(index)._sparse_hash_set_data;
}

// basic add/find/remove
template <typename Memory>
SKR_INLINE typename SparseHashBase<Memory>::DataRef SparseHashBase<Memory>::_add_unsafe(HashType hash)
{
    auto data_arr_ref                        = Super::add_unsafe();
    data_arr_ref.ref()._sparse_hash_set_hash = hash;
    _add_to_bucket(data_arr_ref.ref(), data_arr_ref.index());
    return { &data_arr_ref.ref()._sparse_hash_set_data, data_arr_ref.index(), hash, false };
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashBase<Memory>::CDataRef SparseHashBase<Memory>::_find(HashType hash, Pred&& pred) const
{
    if (!bucket()) return {};

    SizeType search_index = bucket()[_bucket_index(hash)];
    while (search_index != npos)
    {
        auto& node = Super::at(search_index);
        if (node._sparse_hash_set_hash == hash && pred(node._sparse_hash_set_data))
        {
            return { &node._sparse_hash_set_data, search_index, hash, false };
        }
        search_index = node._sparse_hash_set_next;
    }
    return {};
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashBase<Memory>::CDataRef SparseHashBase<Memory>::_find_next(DataRef ref, Pred&& pred) const
{
    if (!bucket() || !ref.is_valid()) return {};

    SizeType search_index = Super::at(ref.index())._sparse_hash_set_next;
    while (search_index != npos)
    {
        auto& node = Super::at(search_index);
        if (node._sparse_hash_set_hash == ref.hash() && pred(node._sparse_hash_set_data))
        {
            return { &node._sparse_hash_set_data, search_index, ref.hash(), false };
        }
        search_index = node._sparse_hash_set_next;
    }
    return {};
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseHashBase<Memory>::_remove(HashType hash, Pred&& pred)
{
    if (DataRef ref = _find(hash, std::forward(pred)))
    {
        _remove_at(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseHashBase<Memory>::_remove_next(DataRef ref, Pred&& pred)
{
    if (DataRef next_ref = _find_next(ref, std::forward(pred)))
    {
        _remove_at(next_ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashBase<Memory>::SizeType SparseHashBase<Memory>::_remove_all(HashType hash, Pred&& pred)
{
    SizeType count = 0;

    SizeType* p_next_idx = bucket() + _bucket_index(hash);

    while (*p_next_idx != npos)
    {
        auto& next_data = Super::at(*p_next_idx);

        if (next_data._sparse_hash_set_hash == hash && pred(next_data._sparse_hash_set_data))
        {
            // update link
            SizeType index = *p_next_idx;
            *p_next_idx    = next_data._sparse_hash_set_next;

            // remove item
            _remove_at(index);
            ++count;
        }
        else
        {
            // move to next node
            p_next_idx = &next_data._sparse_hash_set_next;
        }
    }

    return count;
}
template <typename Memory>
SKR_INLINE void SparseHashBase<Memory>::_remove_at(SizeType index)
{
    _remove_from_bucket(index);
    data_arr().remove_at(index);
}

} // namespace skr::container