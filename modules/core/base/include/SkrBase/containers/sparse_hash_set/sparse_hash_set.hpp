#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"
#include "SkrBase/containers/misc/container_traits.hpp"

// SparseHashSet def
namespace skr::container
{
template <typename Memory>
struct SparseHashSet : protected SparseHashBase<Memory> {
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
    using Cursor   = SparseHashSetCursor<SparseHashSet, false>;
    using CCursor  = SparseHashSetCursor<SparseHashSet, true>;
    using Iter     = SparseHashSetIter<SparseHashSet, false>;
    using CIter    = SparseHashSetIter<SparseHashSet, true>;
    using IterInv  = SparseHashSetIterInv<SparseHashSet, false>;
    using CIterInv = SparseHashSetIterInv<SparseHashSet, true>;

    // stl-style iterator
    using StlIt  = CursorIterStl<Cursor, false>;
    using CStlIt = CursorIterStl<CCursor, false>;

    // ctor & dtor
    SparseHashSet(AllocatorCtorParam param = {});
    SparseHashSet(SizeType reserve_size, AllocatorCtorParam param = {});
    SparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param = {});
    SparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param = {});
    ~SparseHashSet();

    // copy & move
    SparseHashSet(const SparseHashSet& rhs);
    SparseHashSet(SparseHashSet&& rhs);

    // assign & move assign
    SparseHashSet& operator=(const SparseHashSet& rhs);
    SparseHashSet& operator=(SparseHashSet&& rhs);

    // compare
    bool operator==(const SparseHashSet& rhs) const;
    bool operator!=(const SparseHashSet& rhs) const;

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
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    DataRef add(U&& v, DataRef hint);
    template <typename Pred, typename ConstructFunc, typename AssignFunc>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    DataRef add_ex(HashType hash, Pred&& pred, ConstructFunc&& construct, AssignFunc&& assign);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
    DataRef add_ex_unsafe(HashType hash, Pred&& pred);

    // emplace
    template <typename... Args>
    requires(std::is_constructible_v<typename Memory::SetDataType, Args...>)
    DataRef emplace(Args&&... args);
    template <typename... Args>
    requires(std::is_constructible_v<typename Memory::SetDataType, Args...>)
    DataRef emplace(DataRef hint, Args&&... args);

    // append
    void append(const SparseHashSet& set);
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

    // contains if
    using Super::contains_if;
    using Super::count_if;

    // visitor & modifier
    using Super::at;
    using Super::at_last;

    // sort
    using Super::sort;
    using Super::sort_stable;

    // set ops
    SparseHashSet operator&(const SparseHashSet& rhs) const;     // intersect
    SparseHashSet operator|(const SparseHashSet& rhs) const;     // union
    SparseHashSet operator^(const SparseHashSet& rhs) const;     // difference
    SparseHashSet operator-(const SparseHashSet& rhs) const;     // sub
    bool          is_sub_set_of(const SparseHashSet& rhs) const; // sub set

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
    const SparseHashSet& readonly() const;
};
} // namespace skr::container

// SparseHashSet impl
namespace skr::container
{
// ctor & dtor
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(AllocatorCtorParam param)
    : Super(std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(SizeType reserve_size, AllocatorCtorParam param)
    : Super(std::move(param))
{
    reserve(reserve_size);
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(p, n);
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(init_list);
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::~SparseHashSet()
{
    // handled by SparseHashBase
}

// copy & move
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(const SparseHashSet& rhs)
    : Super(rhs)
{
    // handled by SparseHashBase
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(SparseHashSet&& rhs)
    : Super(std::move(rhs))
{
    // handled by SparseHashBase
}

// assign & move assign
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>& SparseHashSet<Memory>::operator=(const SparseHashSet& rhs)
{
    Super::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>& SparseHashSet<Memory>::operator=(SparseHashSet&& rhs)
{
    Super::operator=(std::move(rhs));
    return *this;
}

// compare
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::operator==(const SparseHashSet& rhs) const
{
    if (size() == rhs.size())
    {
        return is_sub_set_of(rhs);
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::operator!=(const SparseHashSet& rhs) const
{
    return !(*this == rhs);
}

// add
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::add(U&& v)
{
    HashType hash = HasherType()(v);
    DataRef  ref  = add_ex_unsafe(hash, [&v](const SetDataType& k) { return k == v; });
    if (ref.already_exist())
    { // assign case
        ref.ref() = std::forward<U>(v);
    }
    else
    { // construct case
        new (ref.ptr()) SetDataType(std::forward<U>(v));
    }
    return ref;
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::add(U&& v, DataRef hint)
{
    if (hint.is_valid())
    { // assign case
        SKR_ASSERT(HasherType()(v) == hint.hash());
        SKR_ASSERT(find(v) == hint);
        hint.ref() = std::forward<U>(v);
        return { hint.ptr(), hint.index(), hint.hash(), true };
    }
    else
    { // add case
        SKR_ASSERT(HasherType()(v) == hint.hash());
        SKR_ASSERT(!contains(v));
        DataRef ref = Super::template _add_unsafe<DataRef>(hint.hash());
        new (ref.ptr()) SetDataType(std::forward<U>(v));
        return ref;
    }
}
template <typename Memory>
template <typename Pred, typename ConstructFunc, typename AssignFunc>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::add_ex(HashType hash, Pred&& pred, ConstructFunc&& construct, AssignFunc&& assign)
{
    DataRef ref = add_ex_unsafe(hash, std::forward<Pred>(pred));
    if (ref.already_exist())
    { // assign case
        assign(ref.ptr());
    }
    else
    { // construct case
        construct(ref.ptr());
    }
    SKR_ASSERT(HasherType()(ref.ref()) == hash);
    return ref;
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::add_ex_unsafe(HashType hash, Pred&& pred)
{
    if (DataRef ref = Super::template _find<DataRef>(hash, std::forward<Pred>(pred)))
    {
        return { ref.ptr(), ref.index(), hash, true };
    }
    else
    {
        return Super::template _add_unsafe<DataRef>(hash);
    }
}

// emplace
template <typename Memory>
template <typename... Args>
requires(std::is_constructible_v<typename Memory::SetDataType, Args...>)
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::emplace(Args&&... args)
{
    // emplace to data vector
    auto data_arr_ref = data_vector().add_unsafe();
    new (&data_arr_ref.ref()._sparse_hash_set_data) SetDataType(std::forward<Args>(args)...);
    data_arr_ref.ref()._sparse_hash_set_hash = HasherType()(data_arr_ref.ref()._sparse_hash_set_data);

    if (DataRef ref = find_ex(
            data_arr_ref.ref()._sparse_hash_set_hash,
            [&data_arr_ref](const SetDataType& k) {
                return k == data_arr_ref.ref()._sparse_hash_set_data;
            }
        ))
    {
        // move data
        ::skr::memory::move(ref.ptr(), &data_arr_ref.ref()._sparse_hash_set_data);

        // remove placeholder
        data_vector().remove_at_unsafe(data_arr_ref.index());

        // return old data
        return { ref.ptr(), ref.index(), ref.hash(), true };
    }
    else
    {
        Super::_add_to_bucket(data_arr_ref.ref(), data_arr_ref.index());
        return {
            &data_arr_ref.ref()._sparse_hash_set_data,
            data_arr_ref.index(),
            data_arr_ref.ref()._sparse_hash_set_hash,
            false
        };
    }
}
template <typename Memory>
template <typename... Args>
requires(std::is_constructible_v<typename Memory::SetDataType, Args...>)
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::emplace(DataRef hint, Args&&... args)
{
    if (hint.is_valid())
    { // assign case
        SetDataType data{ std::forward<Args>(args)... };
        SKR_ASSERT(HasherType()(data) == hint.hash());
        SKR_ASSERT(find(data) == hint);
        hint.ref() = std::move(data);
        return {
            hint.ptr(),
            hint.index(),
            hint.hash(),
            true
        };
    }
    else
    { // add case
        auto data_arr_ref = data_vector().add_unsafe();
        new (&data_arr_ref.ref()._sparse_hash_set_data) SetDataType(std::forward<Args>(args)...);
        data_arr_ref.ref()._sparse_hash_set_hash = HasherType()(data_arr_ref.ref()._sparse_hash_set_data);
        SKR_ASSERT(data_arr_ref.ref()._sparse_hash_set_hash = hint.hash());
        SKR_ASSERT(!contains(data_arr_ref.ref()._sparse_hash_set_data));
        Super::_add_to_bucket(data_arr_ref.ref(), data_arr_ref.index());
        return {
            &data_arr_ref.ref()._sparse_hash_set_data,
            data_arr_ref.index(),
            data_arr_ref.ref()._sparse_hash_set_hash,
            false
        };
    }
}

// append
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::append(const SparseHashSet& rhs)
{
    // fill slack
    SizeType count  = 0;
    auto     cursor = rhs.cursor_begin();
    while (slack() > 0 && !cursor.reach_end())
    {
        add(cursor.ref());
        cursor.move_next();
        ++count;
    }

    // reserve and add
    if (!cursor.reach_end())
    {
        auto new_capacity = data_vector().capacity() + (rhs.size() - count);
        data_vector().reserve(new_capacity);

        while (!cursor.reach_end())
        {
            add(cursor.ref());
            cursor.move_next();
        }
    }
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::append(std::initializer_list<SetDataType> init_list)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < init_list.size())
    {
        const auto& v = init_list.begin()[read_idx];
        add(v);
        ++read_idx;
    }

    // reserve and add
    if (read_idx < init_list.size())
    {
        auto new_capacity = data_vector().capacity() + (init_list.size() - read_idx);
        data_vector().reserve(new_capacity);

        while (read_idx < init_list.size())
        {
            const auto& v = init_list.begin()[read_idx];
            add(v);
            ++read_idx;
        }
    }
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::append(const SetDataType* p, SizeType n)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < n)
    {
        const auto& v = p[read_idx];
        add(v);
        ++read_idx;
    }

    // reserve and add
    if (read_idx < n)
    {
        auto new_capacity = data_vector().capacity() + (n - read_idx);
        data_vector().reserve(new_capacity);

        while (read_idx < n)
        {
            const auto& v = p[read_idx];
            add(v);
            ++read_idx;
        }
    }
}
template <typename Memory>
template <EachAbleContainer U>
SKR_INLINE void SparseHashSet<Memory>::append(U&& container)
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

        // fill slack
        SizeType count = 0;
        while (slack() > 0 && begin != end)
        {
            add(*begin);
            ++begin;
            ++count;
        }

        // reserve and add
        if (begin != end)
        {
            auto new_capacity = data_vector().capacity() + (n - count);
            data_vector().reserve(new_capacity);

            while (begin != end)
            {
                add(*begin);
                ++begin;
            }
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
SKR_INLINE bool SparseHashSet<Memory>::remove(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::_remove(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE bool SparseHashSet<Memory>::remove_ex(HashType hash, Pred&& pred)
{
    return Super::_remove(hash, std::forward<Pred>(pred));
}

// find
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::template _find<DataRef>(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find(const U& v) const
{
    HashType hash = HasherType()(v);
    return Super::template _find<CDataRef>(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::template _find<DataRef>(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::template _find<CDataRef>(hash, std::forward<Pred>(pred));
}

// find if
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_if(Pred&& pred)
{
    return Super::template _find_if<DataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_last_if(Pred&& pred)
{
    return Super::template _find_last_if<DataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find_if(Pred&& pred) const
{
    return Super::template _find_if<CDataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find_last_if(Pred&& pred) const
{
    return Super::template _find_last_if<CDataRef>(std::forward<Pred>(pred));
}

// contains
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE bool SparseHashSet<Memory>::contains(const U& v) const
{
    return (bool)find(v);
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::SetDataType&>)
SKR_INLINE bool SparseHashSet<Memory>::contains_ex(HashType hash, Pred&& pred) const
{
    return (bool)find_ex(hash, std::forward<Pred>(pred));
}

// set ops
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator&(const SparseHashSet& rhs) const
{ // intersect
    bool                 rhs_smaller = size() > rhs.size();
    const SparseHashSet& a           = rhs_smaller ? rhs : *this;
    const SparseHashSet& b           = rhs_smaller ? *this : rhs;

    SparseHashSet result;
    result.reserve(a.size());

    for (const auto& v : a)
    {
        if (b.contains(v))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator|(const SparseHashSet& rhs) const
{ // union
    SparseHashSet result(*this);
    for (const auto& v : rhs)
    {
        result.add(v);
    }
    return result;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator^(const SparseHashSet& rhs) const
{ // difference
    SparseHashSet result(size());

    for (const auto& v : range())
    {
        if (!rhs.contains(v))
        {
            result.add(v);
        }
    }

    for (const auto& v : rhs)
    {
        if (!contains(v))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator-(const SparseHashSet& rhs) const
{ // sub
    SparseHashSet result(size());

    for (const auto& v : range())
    {
        if (!rhs.contains(v))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::is_sub_set_of(const SparseHashSet& rhs) const
{
    if (rhs.size() >= size())
    {
        for (const auto& v : range())
        {
            if (!rhs.contains(v))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

// cursor & iterator
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::Cursor SparseHashSet<Memory>::cursor_begin()
{
    return Cursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CCursor SparseHashSet<Memory>::cursor_begin() const
{
    return CCursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::Cursor SparseHashSet<Memory>::cursor_end()
{
    return Cursor::End(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CCursor SparseHashSet<Memory>::cursor_end() const
{
    return CCursor::End(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::Iter SparseHashSet<Memory>::iter()
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CIter SparseHashSet<Memory>::iter() const
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::IterInv SparseHashSet<Memory>::iter_inv()
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CIterInv SparseHashSet<Memory>::iter_inv() const
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE auto SparseHashSet<Memory>::range()
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto SparseHashSet<Memory>::range() const
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto SparseHashSet<Memory>::range_inv()
{
    return cursor_end().as_range_inv();
}
template <typename Memory>
SKR_INLINE auto SparseHashSet<Memory>::range_inv() const
{
    return cursor_end().as_range_inv();
}

// stl-style iterator
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::StlIt SparseHashSet<Memory>::begin()
{
    return { Cursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CStlIt SparseHashSet<Memory>::begin() const
{
    return { CCursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::StlIt SparseHashSet<Memory>::end()
{
    return { Cursor::EndOverflow(this) };
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CStlIt SparseHashSet<Memory>::end() const
{
    return { CCursor::EndOverflow(this) };
}

// syntax
template <typename Memory>
SKR_INLINE const SparseHashSet<Memory>& SparseHashSet<Memory>::readonly() const
{
    return *this;
}
} // namespace skr::container

// container traits
namespace skr::container
{
template <typename Memory>
struct ContainerTraits<SparseHashSet<Memory>> {
    constexpr static bool is_linear_memory = false; // data(), size()
    constexpr static bool has_size         = true;  // size()
    constexpr static bool is_iterable      = true;  // begin(), end()
    constexpr static bool is_reservable    = true;  // reserve()

    using ElementType = typename Memory::DataType;
    using SizeType    = typename Memory::SizeType;

    inline static SizeType size(const SparseHashSet<Memory>& container) { return container.size(); }

    inline static typename SparseHashSet<Memory>::StlIt  begin(SparseHashSet<Memory>& container) { return container.begin(); }
    inline static typename SparseHashSet<Memory>::StlIt  end(SparseHashSet<Memory>& container) { return container.end(); }
    inline static typename SparseHashSet<Memory>::CStlIt begin(const SparseHashSet<Memory>& container) { return container.begin(); }
    inline static typename SparseHashSet<Memory>::CStlIt end(const SparseHashSet<Memory>& container) { return container.end(); }

    inline static void reserve(SparseHashSet<Memory>& container, SizeType n) { container.reserve(n); }
};
} // namespace skr::container