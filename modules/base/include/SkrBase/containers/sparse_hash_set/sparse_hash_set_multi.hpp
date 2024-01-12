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

    // data ref
    using DataRef  = SparseHashSetDataRef<SetDataType, SizeType, HashType, false>;
    using CDataRef = SparseHashSetDataRef<SetDataType, SizeType, HashType, true>;

    // cursor & iterator
    using Cursor   = SparseHashSetCursor<MultSparseHashSet, false>;
    using CCursor  = SparseHashSetCursor<MultSparseHashSet, true>;
    using Iter     = SparseHashSetIter<MultSparseHashSet, false>;
    using CIter    = SparseHashSetIter<MultSparseHashSet, true>;
    using IterInv  = SparseHashSetIterInv<MultSparseHashSet, false>;
    using CIterInv = SparseHashSetIterInv<MultSparseHashSet, true>;

    // stl-style iterator
    using StlIt  = CursorIterStl<Cursor, false>;
    using CStlIt = CursorIterStl<CCursor, false>;

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
    using Super::remove_at;
    using Super::remove_at_unsafe;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool remove(const U& v);
    template <typename Pred>
    bool remove_ex(HashType hash, Pred&& pred);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    SizeType remove_all(const U& v);
    template <typename Pred>
    SizeType remove_all_ex(HashType hash, Pred&& pred);

    // remove if
    using Super::remove_if;
    using Super::remove_last_if;
    using Super::remove_all_if;

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

    // find if
    template <typename Pred>
    DataRef find_if(Pred&& pred);
    template <typename Pred>
    DataRef find_last_if(Pred&& pred);
    template <typename Pred>
    CDataRef find_if(Pred&& pred) const;
    template <typename Pred>
    CDataRef find_last_if(Pred&& pred) const;

    // contains
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool contains(const U& v) const;
    template <typename Pred>
    bool contains_ex(HashType hash, Pred&& pred) const;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    SizeType count(const U& v) const;
    template <typename Pred>
    SizeType count_ex(HashType hash, Pred&& pred) const;

    // contains if
    using Super::contains_if;
    using Super::count_if;

    // visitor & modifier
    using Super::at;
    using Super::last;

    // sort
    using Super::sort;
    using Super::sort_stable;

    // cursor & iterator
    Cursor   cursor_begin();
    CCursor  cursor_begin() const;
    Cursor   cursor_end();
    CCursor  cursor_end() const;
    Iter     iter();
    CIter    iter() const;
    IterInv  iter_inv();
    CIterInv iter_inv() const;
    auto     range();
    auto     range() const;
    auto     range_inv();
    auto     range_inv() const;

    // stl-style iterator
    StlIt  begin();
    CStlIt begin() const;
    StlIt  end();
    CStlIt end() const;

    // syntax
    const MultSparseHashSet& readonly() const;
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
    SKR_ASSERT(HasherType()(ref.ref()) == hash);
    return ref;
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::add_ex_unsafe(HashType hash)
{
    return Super::template _add_unsafe<DataRef>(hash);
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

// find
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::template _find<DataRef>(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find(const U& v) const
{
    HashType hash = HasherType()(v);
    return Super::template _find<CDataRef>(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::template _find<DataRef>(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::template _find<CDataRef>(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_next(DataRef ref, const U& v)
{
    return Super::template _find_next<DataRef>(ref, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_next(CDataRef ref, const U& v) const
{
    return Super::template _find_next<CDataRef>(ref, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_next_ex(CDataRef ref, Pred&& pred)
{
    return Super::template _find_next<DataRef>(ref, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_next_ex(CDataRef ref, Pred&& pred) const
{
    return Super::template _find_next<CDataRef>(ref, std::forward<Pred>(pred));
}

// find if
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_if(Pred&& pred)
{
    return Super::template _find_if<DataRef>(std::forward(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::DataRef MultSparseHashSet<Memory>::find_last_if(Pred&& pred)
{
    return Super::template _find_last_if<DataRef>(std::forward(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_if(Pred&& pred) const
{
    return Super::template _find_if<CDataRef>(std::forward(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultSparseHashSet<Memory>::CDataRef MultSparseHashSet<Memory>::find_last_if(Pred&& pred) const
{
    return Super::template _find_last_if<CDataRef>(std::forward(pred));
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
    auto ref  = Super::template _find<CDataRef>(HasherType()(v), pred);
    while (ref.is_valid())
    {
        ++count;
        ref = Super::template _find_next<CDataRef>(ref, pred);
    };

    return count;
}
template <typename Memory>
template <typename Pred>
typename MultSparseHashSet<Memory>::SizeType MultSparseHashSet<Memory>::count_ex(HashType hash, Pred&& pred) const
{
    SizeType count = 0;

    auto ref = Super::template _find<CDataRef>(hash, std::forward<Pred>(pred));
    while (ref.is_valid())
    {
        ++count;
        ref = Super::template _find_next<CDataRef>(ref, std::forward<pred>());
    };

    return count;
}

// cursor & iterator
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::Cursor MultSparseHashSet<Memory>::cursor_begin()
{
    return Cursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CCursor MultSparseHashSet<Memory>::cursor_begin() const
{
    return CCursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::Cursor MultSparseHashSet<Memory>::cursor_end()
{
    return Cursor::End(this);
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CCursor MultSparseHashSet<Memory>::cursor_end() const
{
    return CCursor::End(this);
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::Iter MultSparseHashSet<Memory>::iter()
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CIter MultSparseHashSet<Memory>::iter() const
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::IterInv MultSparseHashSet<Memory>::iter_inv()
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CIterInv MultSparseHashSet<Memory>::iter_inv() const
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE auto MultSparseHashSet<Memory>::range()
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto MultSparseHashSet<Memory>::range() const
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto MultSparseHashSet<Memory>::range_inv()
{
    return cursor_end().as_range_inv();
}
template <typename Memory>
SKR_INLINE auto MultSparseHashSet<Memory>::range_inv() const
{
    return cursor_end().as_range_inv();
}

// stl-style iterator
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::StlIt MultSparseHashSet<Memory>::begin()
{
    return { Cursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CStlIt MultSparseHashSet<Memory>::begin() const
{
    return { CCursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::StlIt MultSparseHashSet<Memory>::end()
{
    return { Cursor::EndOverflow(this) };
}
template <typename Memory>
SKR_INLINE typename MultSparseHashSet<Memory>::CStlIt MultSparseHashSet<Memory>::end() const
{
    return { CCursor::EndOverflow(this) };
}

// syntax
template <typename Memory>
SKR_INLINE const MultSparseHashSet<Memory>& MultSparseHashSet<Memory>::readonly() const
{
    return *this;
}
} // namespace skr::container