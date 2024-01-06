#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"

namespace skr::container
{
template <typename Memory>
struct MultSparseHashSet : protected SparseHashBase<Memory> {
    using Super = SparseHashBase<Memory>;

    // sparse array configure
    using typename Super::SizeType;
    using typename Super::DataType;
    using typename Super::StorageType;
    using typename Super::BitBlockType;
    using typename Super::AllocatorCtorParam;

    // sparse hash set configure
    using typename Super::HashType;
    using typename Super::HasherType;
    using typename Super::SetDataType;
    using typename Super::SetStorageType;

    // helper
    using DataArr                         = SparseArray<Memory>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref & iterator
    using DataRef  = SparseHashSetDataRef<SetDataType, SizeType, HashType, false>;
    using CDataRef = SparseHashSetDataRef<SetDataType, SizeType, HashType, true>;
    using It       = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, false>;
    using CIt      = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, true>;

    // ctor & dtor
    MultSparseHashSet(AllocatorCtorParam param = {});
    MultSparseHashSet(SizeType reserve_size, AllocatorCtorParam param = {});
    MultSparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param = {});
    MultSparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param = {});
    ~MultSparseHashSet();

    // copy & move
    MultSparseHashSet(const MultSparseHashSet& rhs);
    MultSparseHashSet(MultSparseHashSet&& rhs);

    // assign & move assign
    MultSparseHashSet& operator=(const MultSparseHashSet& rhs);
    MultSparseHashSet& operator=(MultSparseHashSet&& rhs);

    // getter
    using Super::size;
    using Super::capacity;
    using Super::slack;
    using Super::sparse_size;
    using Super::hole_size;
    using Super::bit_array_size;
    using Super::free_list_head;
    using Super::is_compact;
    using Super::empty;
    using Super::data_arr;
    using Super::bucket;
    using Super::memory;

    // validator
    using Super::has_data;
    using Super::is_hole;
    using Super::is_valid_index;
    using Super::is_valid_pointer;

    // memory op
    using Super::clear;
    using Super::release;
    using Super::reserve;
    using Super::shrink;
    using Super::compact;
    using Super::compact_stable;
    using Super::compact_top;

    // rehash
    using Super::sync_hash;
    using Super::sync_hash_at;
    using Super::rehash;
    using Super::rehash_if_need;

    // add
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    DataRef add(U&& v);
    template <typename ConstructFunc>
    DataRef add_ex(HashType hash, ConstructFunc&& construct);
    DataRef add_ex_unsafe(HashType hash);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);

    // append
    void append(const MultSparseHashSet& set);
    void append(std::initializer_list<SetDataType> init_list);
    void append(const SetDataType* p, SizeType n);

    // remove
    void remove_at(SizeType index);
    void remove_at_unsafe(SizeType index);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool remove(const U& v);
    template <typename Pred>
    bool remove_ex(HashType hash, Pred&& pred);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    SizeType remove_all(const U& v);
    template <typename Pred>
    SizeType remove_all_ex(HashType hash, Pred&& pred);

    // erase
    It  erase(const It& it);
    CIt erase(const CIt& it);

    // find
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    DataRef find(const U& v);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    CDataRef find(const U& v) const;
    template <typename Pred>
    DataRef find_ex(HashType hash, Pred&& pred);
    template <typename Pred>
    CDataRef find_ex(HashType hash, Pred&& pred) const;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    DataRef find_next(DataRef ref, const U& v);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    CDataRef find_next(CDataRef ref, const U& v) const;
    template <typename Pred>
    DataRef find_next_ex(CDataRef ref, Pred&& pred);
    template <typename Pred>
    CDataRef find_next_ex(CDataRef ref, Pred&& pred) const;

    // contains
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool contains(const U& v) const;
    template <typename Pred>
    bool contains_ex(HashType hash, Pred&& pred) const;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    SizeType count(const U& v) const;
    template <typename Pred>
    SizeType count_ex(HashType hash, Pred&& pred) const;

    // visitor & modifier
    using Super::at;
    using Super::last;
    using Super::modify_at;
    using Super::modify_last;
    using Super::modify;

    // sort
    using Super::sort;
    using Super::sort_stable;

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;
};
} // namespace skr::container

namespace skr::container
{
// ctor & dtor
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>::MultSparseHashSet(AllocatorCtorParam param)
    : Super(std::move(param))
{
}
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>::MultSparseHashSet(SizeType reserve_size, AllocatorCtorParam param)
    : Super(std::move(param))
{
    reserve(reserve_size);
}
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>::MultSparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(p, n);
}
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>::MultSparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(init_list);
}
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>::~MultSparseHashSet()
{
    // handled by SparseHashBase
}

// copy & move
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>::MultSparseHashSet(const MultSparseHashSet& rhs)
    : Super(rhs)
{
    // handled by SparseHashBase
}
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>::MultSparseHashSet(MultSparseHashSet&& rhs)
    : Super(std::move(rhs))
{
    // handled by SparseHashBase
}

// assign & move assign
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>& MultSparseHashSet<Memory>::operator=(const MultSparseHashSet& rhs)
{
    Super::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE MultSparseHashSet<Memory>& MultSparseHashSet<Memory>::operator=(MultSparseHashSet&& rhs)
{
    Super::operator=(std::move(rhs));
    return *this;
}

// add
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::add(U&& v)
{
    HashType hash = HasherType()(v);
    DataRef  ref  = add_ex_unsafe(hash);
    new (ref.ptr()) SetDataType(std::forward<U>(v));
    return ref;
}
template <typename Memory>
template <typename ConstructFunc>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::add_ex(HashType hash, ConstructFunc&& construct)
{
    DataRef ref = add_ex_unsafe(hash);
    construct(ref.ptr());
    return ref;
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::add_ex_unsafe(HashType hash)
{
    return Super::_add_unsafe(hash);
}

// emplace
template <typename Memory>
template <typename... Args>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::emplace(Args&&... args)
{
    auto data_arr_ref = data_arr().add_unsafe();
    new (&data_arr_ref.ref()._sparse_hash_set_data) SetDataType(std::forward<Args>(args)...);
    data_arr_ref.ref()._sparse_hash_set_hash = HasherType()(data_arr_ref.ref()._sparse_hash_set_data);
    Super::_add_to_bucket(data_arr_ref.ref(), data_arr_ref.index());
    return {
        &data_arr_ref.ref()._sparse_hash_set_data,
        data_arr_ref.index(),
        data_arr_ref.ref()._sparse_hash_set_hash,
        false
    };
}

// append
template <typename Memory>
SKR_INLINE void MultSparseHashSet<Memory>::append(const MultSparseHashSet& set)
{
    reserve(size() + set.size());

    for (auto& v : set)
    {
        add(v);
    }
}
template <typename Memory>
SKR_INLINE void MultSparseHashSet<Memory>::append(std::initializer_list<SetDataType> init_list)
{
    reserve(size() + init_list.size());

    for (auto& v : init_list)
    {
        add(v);
    }
}
template <typename Memory>
SKR_INLINE void MultSparseHashSet<Memory>::append(const SetDataType* p, SizeType n)
{
    reserve(size() + n);

    for (SizeType i = 0; i < n; ++i)
    {
        add(p[i]);
    }
}

// remove
template <typename Memory>
SKR_INLINE void MultSparseHashSet<Memory>::remove_at(SizeType index)
{
    Super::_remove_from_bucket(index);
    data_arr().remove_at(index);
}
template <typename Memory>
SKR_INLINE void MultSparseHashSet<Memory>::remove_at_unsafe(SizeType index)
{
    Super::_remove_from_bucket(index);
    data_arr().remove_at_unsafe(index);
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE bool MultSparseHashSet<Memory>::remove(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::_remove(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool MultSparseHashSet<Memory>::remove_ex(HashType hash, Pred&& pred)
{
    return Super::_remove(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::SizeType MultSparseHashSet<Memory>::remove_all(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::_remove_all(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::SizeType MultSparseHashSet<Memory>::remove_all_ex(HashType hash, Pred&& pred)
{
    return Super::_remove_all(hash, std::forward<Pred>(pred));
}

// erase
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::It MultSparseHashSet<Memory>::erase(const It& it)
{
    remove_at(it.index());
    It new_it(it);
    ++new_it;
    return new_it;
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CIt MultSparseHashSet<Memory>::erase(const CIt& it)
{
    remove_at(it.index());
    CIt new_it(it);
    ++new_it;
    return new_it;
}

// find
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::_find(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find(const U& v) const
{
    HashType hash = HasherType()(v);
    return Super::_find(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::_find(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::_find(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_next(DataRef ref, const U& v)
{
    return Super::_find_next(ref, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_next(CDataRef ref, const U& v) const
{
    return Super::_find_next(ref, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_next_ex(CDataRef ref, Pred&& pred)
{
    return Super::_find_next(ref, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_next_ex(CDataRef ref, Pred&& pred) const
{
    return Super::_find_next(ref, std::forward<Pred>(pred));
}

// contains
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
bool MultSparseHashSet<Memory>::contains(const U& v) const
{
    return (bool)find(v);
}
template <typename Memory>
template <typename Pred>
bool MultSparseHashSet<Memory>::contains_ex(HashType hash, Pred&& pred) const
{
    return (bool)find_ex(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
typename MultSparseHashSet<Memory>::SizeType MultSparseHashSet<Memory>::count(const U& v) const
{
    SizeType count = 0;

    auto pred = [&v](const SetDataType& k) { return k == v; };
    auto ref  = Super::_find(HasherType()(v), pred);
    while (ref.is_valid())
    {
        ++count;
        ref = Super::_find_next(ref, pred);
    };

    return count;
}
template <typename Memory>
template <typename Pred>
typename MultSparseHashSet<Memory>::SizeType MultSparseHashSet<Memory>::count_ex(HashType hash, Pred&& pred) const
{
    SizeType count = 0;

    auto ref = Super::_find(hash, std::forward<Pred>(pred));
    while (ref.is_valid())
    {
        ++count;
        ref = Super::_find_next(ref, std::forward<pred>());
    };

    return count;
}

// support foreach
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::It MultSparseHashSet<Memory>::begin()
{
    return It(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::It MultSparseHashSet<Memory>::end()
{
    return It(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CIt MultSparseHashSet<Memory>::begin() const
{
    return CIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CIt MultSparseHashSet<Memory>::end() const
{
    return CIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}

} // namespace skr::container