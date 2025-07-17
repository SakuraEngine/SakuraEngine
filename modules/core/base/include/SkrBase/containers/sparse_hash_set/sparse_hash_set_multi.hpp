#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"
#include "SkrBase/containers/misc/container_traits.hpp"

// MultiSparseHashSet def
namespace skr::container
{
template <typename Memory>
struct MultiSparseHashSet : protected SparseHashBase<Memory> {
    using Super = SparseHashBase<Memory>;

    // sparse vector configure
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
    using DataVector                      = SparseVector<Memory>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref
    using DataRef  = SparseHashSetDataRef<SetDataType, SizeType, HashType, false>;
    using CDataRef = SparseHashSetDataRef<SetDataType, SizeType, HashType, true>;

    // cursor & iterator
    using Cursor   = SparseHashSetCursor<MultiSparseHashSet, false>;
    using CCursor  = SparseHashSetCursor<MultiSparseHashSet, true>;
    using Iter     = SparseHashSetIter<MultiSparseHashSet, false>;
    using CIter    = SparseHashSetIter<MultiSparseHashSet, true>;
    using IterInv  = SparseHashSetIterInv<MultiSparseHashSet, false>;
    using CIterInv = SparseHashSetIterInv<MultiSparseHashSet, true>;

    // stl-style iterator
    using StlIt  = CursorIterStl<Cursor, false>;
    using CStlIt = CursorIterStl<CCursor, false>;

    // ctor & dtor
    MultiSparseHashSet(AllocatorCtorParam param = {});
    MultiSparseHashSet(SizeType reserve_size, AllocatorCtorParam param = {});
    MultiSparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param = {});
    MultiSparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param = {});
    ~MultiSparseHashSet();

    // copy & move
    MultiSparseHashSet(const MultiSparseHashSet& rhs);
    MultiSparseHashSet(MultiSparseHashSet&& rhs);

    // assign & move assign
    MultiSparseHashSet& operator=(const MultiSparseHashSet& rhs);
    MultiSparseHashSet& operator=(MultiSparseHashSet&& rhs);

    // getter
    using Super::size;
    using Super::capacity;
    using Super::slack;
    using Super::sparse_size;
    using Super::hole_size;
    using Super::bit_size;
    using Super::freelist_head;
    using Super::is_compact;
    using Super::is_empty;
    using Super::data_vector;
    using Super::bucket;
    using Super::memory;

    // validator
    using Super::has_data;
    using Super::is_hole;
    using Super::is_valid_index;

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
    requires(std::is_constructible_v<typename Memory::SetDataType, Args...>)
    DataRef emplace(Args&&... args);

    // append
    void append(const MultiSparseHashSet& set);
    void append(std::initializer_list<SetDataType> init_list);
    void append(const SetDataType* p, SizeType n);
    template <EachAbleContainer U>
    void append(U&& container);

    // remove
    using Super::remove_at;
    using Super::remove_at_unsafe;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool remove(const U& v);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    bool remove_ex(HashType hash, Pred&& pred);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    SizeType remove_all(const U& v);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
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
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    DataRef find_ex(HashType hash, Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    CDataRef find_ex(HashType hash, Pred&& pred) const;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    DataRef find_next(DataRef ref, const U& v);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    CDataRef find_next(CDataRef ref, const U& v) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    DataRef find_next_ex(CDataRef ref, Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    CDataRef find_next_ex(CDataRef ref, Pred&& pred) const;

    // find if
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    DataRef find_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    DataRef find_last_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    CDataRef find_if(Pred&& pred) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    CDataRef find_last_if(Pred&& pred) const;

    // contains
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool contains(const U& v) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    bool contains_ex(HashType hash, Pred&& pred) const;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    SizeType count(const U& v) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    SizeType count_ex(HashType hash, Pred&& pred) const;

    // contains if
    using Super::contains_if;
    using Super::count_if;

    // visitor & modifier
    using Super::at;
    using Super::at_last;

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
    const MultiSparseHashSet& readonly() const;
};
} // namespace skr::container

// MultiSparseHashSet impl
namespace skr::container
{
// ctor & dtor
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>::MultiSparseHashSet(AllocatorCtorParam param)
    : Super(std::move(param))
{
}
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>::MultiSparseHashSet(SizeType reserve_size, AllocatorCtorParam param)
    : Super(std::move(param))
{
    reserve(reserve_size);
}
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>::MultiSparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(p, n);
}
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>::MultiSparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(init_list);
}
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>::~MultiSparseHashSet()
{
    // handled by SparseHashBase
}

// copy & move
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>::MultiSparseHashSet(const MultiSparseHashSet& rhs)
    : Super(rhs)
{
    // handled by SparseHashBase
}
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>::MultiSparseHashSet(MultiSparseHashSet&& rhs)
    : Super(std::move(rhs))
{
    // handled by SparseHashBase
}

// assign & move assign
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>& MultiSparseHashSet<Memory>::operator=(const MultiSparseHashSet& rhs)
{
    Super::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE MultiSparseHashSet<Memory>& MultiSparseHashSet<Memory>::operator=(MultiSparseHashSet&& rhs)
{
    Super::operator=(std::move(rhs));
    return *this;
}

// add
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::add(U&& v)
{
    HashType hash = HasherType()(v);
    DataRef  ref  = add_ex_unsafe(hash);
    new (ref.ptr()) SetDataType(std::forward<U>(v));
    return ref;
}
template <typename Memory>
template <typename ConstructFunc>
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::add_ex(HashType hash, ConstructFunc&& construct)
{
    DataRef ref = add_ex_unsafe(hash);
    construct(ref.ptr());
    SKR_ASSERT(HasherType()(ref.ref()) == hash);
    return ref;
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::add_ex_unsafe(HashType hash)
{
    return Super::template _add_unsafe<DataRef>(hash);
}

// emplace
template <typename Memory>
template <typename... Args>
requires(std::is_constructible_v<typename Memory::SetDataType, Args...>)
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::emplace(Args&&... args)
{
    auto data_arr_ref = data_vector().add_unsafe();
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
SKR_INLINE void MultiSparseHashSet<Memory>::append(const MultiSparseHashSet& set)
{
    reserve(size() + set.size());

    for (auto& v : set)
    {
        add(v);
    }
}
template <typename Memory>
SKR_INLINE void MultiSparseHashSet<Memory>::append(std::initializer_list<SetDataType> init_list)
{
    reserve(size() + init_list.size());

    for (auto& v : init_list)
    {
        add(v);
    }
}
template <typename Memory>
SKR_INLINE void MultiSparseHashSet<Memory>::append(const SetDataType* p, SizeType n)
{
    reserve(size() + n);

    for (SizeType i = 0; i < n; ++i)
    {
        add(p[i]);
    }
}
template <typename Memory>
template <EachAbleContainer U>
SKR_INLINE void MultiSparseHashSet<Memory>::append(U&& container)
{
    using Traits = ContainerTraits<std::decay_t<U>>;
    if constexpr (Traits::is_linear_memory)
    {
        auto n = Traits::size(std::forward<U>(container));
        auto p = Traits::data(std::forward<U>(container));
        append(p, n);
    }
    else if constexpr (Traits::is_iterable && Traits::has_size)
    {
        auto n     = Traits::size(std::forward<U>(container));
        auto begin = Traits::begin(std::forward<U>(container));
        auto end   = Traits::end(std::forward<U>(container));
        reserve(size() + n);
        for (; begin != end; ++begin)
        {
            add(*begin);
        }
    }
    else if constexpr (Traits::is_iterable)
    {
        auto begin = Traits::begin(std::forward<U>(container));
        auto end   = Traits::end(std::forward<U>(container));
        for (; begin != end; ++begin)
        {
            add(*begin);
        }
    }
}

// remove
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE bool MultiSparseHashSet<Memory>::remove(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::_remove(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE bool MultiSparseHashSet<Memory>::remove_ex(HashType hash, Pred&& pred)
{
    return Super::_remove(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultiSparseHashSet<Memory>::SizeType MultiSparseHashSet<Memory>::remove_all(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::_remove_all(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::SizeType MultiSparseHashSet<Memory>::remove_all_ex(HashType hash, Pred&& pred)
{
    return Super::_remove_all(hash, std::forward<Pred>(pred));
}

// find
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::find(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::template _find<DataRef>(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultiSparseHashSet<Memory>::CDataRef MultiSparseHashSet<Memory>::find(const U& v) const
{
    HashType hash = HasherType()(v);
    return Super::template _find<CDataRef>(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::template _find<DataRef>(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::CDataRef MultiSparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::template _find<CDataRef>(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::find_next(DataRef ref, const U& v)
{
    return Super::template _find_next<DataRef>(ref, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename MultiSparseHashSet<Memory>::CDataRef MultiSparseHashSet<Memory>::find_next(CDataRef ref, const U& v) const
{
    return Super::template _find_next<CDataRef>(ref, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::find_next_ex(CDataRef ref, Pred&& pred)
{
    return Super::template _find_next<DataRef>(ref, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::CDataRef MultiSparseHashSet<Memory>::find_next_ex(CDataRef ref, Pred&& pred) const
{
    return Super::template _find_next<CDataRef>(ref, std::forward<Pred>(pred));
}

// find if
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::find_if(Pred&& pred)
{
    return Super::template _find_if<DataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::DataRef MultiSparseHashSet<Memory>::find_last_if(Pred&& pred)
{
    return Super::template _find_last_if<DataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::CDataRef MultiSparseHashSet<Memory>::find_if(Pred&& pred) const
{
    return Super::template _find_if<CDataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename MultiSparseHashSet<Memory>::CDataRef MultiSparseHashSet<Memory>::find_last_if(Pred&& pred) const
{
    return Super::template _find_last_if<CDataRef>(std::forward<Pred>(pred));
}

// contains
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
bool MultiSparseHashSet<Memory>::contains(const U& v) const
{
    return (bool)find(v);
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
bool MultiSparseHashSet<Memory>::contains_ex(HashType hash, Pred&& pred) const
{
    return (bool)find_ex(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
typename MultiSparseHashSet<Memory>::SizeType MultiSparseHashSet<Memory>::count(const U& v) const
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
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
typename MultiSparseHashSet<Memory>::SizeType MultiSparseHashSet<Memory>::count_ex(HashType hash, Pred&& pred) const
{
    SizeType count = 0;

    auto ref = Super::template _find<CDataRef>(hash, std::forward<Pred>(pred));
    while (ref.is_valid())
    {
        ++count;
        ref = Super::template _find_next<CDataRef>(ref, std::forward<Pred>(pred));
    };

    return count;
}

// cursor & iterator
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::Cursor MultiSparseHashSet<Memory>::cursor_begin()
{
    return Cursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::CCursor MultiSparseHashSet<Memory>::cursor_begin() const
{
    return CCursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::Cursor MultiSparseHashSet<Memory>::cursor_end()
{
    return Cursor::End(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::CCursor MultiSparseHashSet<Memory>::cursor_end() const
{
    return CCursor::End(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::Iter MultiSparseHashSet<Memory>::iter()
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::CIter MultiSparseHashSet<Memory>::iter() const
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::IterInv MultiSparseHashSet<Memory>::iter_inv()
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::CIterInv MultiSparseHashSet<Memory>::iter_inv() const
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashSet<Memory>::range()
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashSet<Memory>::range() const
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashSet<Memory>::range_inv()
{
    return cursor_end().as_range_inv();
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashSet<Memory>::range_inv() const
{
    return cursor_end().as_range_inv();
}

// stl-style iterator
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::StlIt MultiSparseHashSet<Memory>::begin()
{
    return { Cursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::CStlIt MultiSparseHashSet<Memory>::begin() const
{
    return { CCursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::StlIt MultiSparseHashSet<Memory>::end()
{
    return { Cursor::EndOverflow(this) };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashSet<Memory>::CStlIt MultiSparseHashSet<Memory>::end() const
{
    return { CCursor::EndOverflow(this) };
}

// syntax
template <typename Memory>
SKR_INLINE const MultiSparseHashSet<Memory>& MultiSparseHashSet<Memory>::readonly() const
{
    return *this;
}
} // namespace skr::container

// container traits
namespace skr::container
{
template <typename Memory>
struct ContainerTraits<MultiSparseHashSet<Memory>> {
    constexpr static bool is_linear_memory = false; // data(), size()
    constexpr static bool has_size         = true;  // size()
    constexpr static bool is_iterable      = true;  // begin(), end()
    constexpr static bool is_reservable    = true;  // reserve()

    using ElementType = typename Memory::DataType;
    using SizeType    = typename Memory::SizeType;

    inline static SizeType size(const MultiSparseHashSet<Memory>& container) { return container.size(); }

    inline static typename MultiSparseHashSet<Memory>::StlIt  begin(MultiSparseHashSet<Memory>& set) { return set.begin(); }
    inline static typename MultiSparseHashSet<Memory>::StlIt  end(MultiSparseHashSet<Memory>& set) { return set.end(); }
    inline static typename MultiSparseHashSet<Memory>::CStlIt begin(const MultiSparseHashSet<Memory>& set) { return set.begin(); }
    inline static typename MultiSparseHashSet<Memory>::CStlIt end(const MultiSparseHashSet<Memory>& set) { return set.end(); }

    inline static void reserve(MultiSparseHashSet<Memory>& container, SizeType n) { container.reserve(n); }
};
} // namespace skr::container