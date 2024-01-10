#pragma once
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"

namespace skr::container
{
template <typename Memory>
struct SparseHashSet : protected SparseHashBase<Memory> {
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
    using StlIt    = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, false>;
    using CStlIt   = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, true>;

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
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    DataRef add(U&& v, DataRef hint);
    template <typename Pred, typename ConstructFunc, typename AssignFunc>
    DataRef add_ex(HashType hash, Pred&& pred, ConstructFunc&& construct, AssignFunc&& assign);
    template <typename Pred>
    DataRef add_ex_unsafe(HashType hash, Pred&& pred);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename... Args>
    DataRef emplace(DataRef hint, Args&&... args);

    // append
    void append(const SparseHashSet& set);
    void append(std::initializer_list<SetDataType> init_list);
    void append(const SetDataType* p, SizeType n);

    // remove
    using Super::remove_at;
    using Super::remove_at_unsafe;
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool remove(const U& v);
    template <typename Pred>
    bool remove_ex(HashType hash, Pred&& pred);

    // remove if
    using Super::remove_if;
    using Super::remove_last_if;
    using Super::remove_all_if;

    // erase
    StlIt  erase(const StlIt& it);
    CStlIt erase(const CStlIt& it);

    // find
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    DataRef find(const U& v);
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    CDataRef find(const U& v) const;
    template <typename Pred>
    DataRef find_ex(HashType hash, Pred&& pred);
    template <typename Pred>
    CDataRef find_ex(HashType hash, Pred&& pred) const;

    // find if
    using Super::find_if;
    using Super::find_last_if;

    // contains
    template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U = SetDataType>
    bool contains(const U& v) const;
    template <typename Pred>
    bool contains_ex(HashType hash, Pred&& pred) const;

    // contains if
    using Super::contains_if;
    using Super::count_if;

    // visitor & modifier
    using Super::at;
    using Super::last;
    using Super::modify_at;
    using Super::modify_last;
    using Super::modify;

    // sort
    using Super::sort;
    using Super::sort_stable;

    // set ops
    SparseHashSet operator&(const SparseHashSet& rhs) const;     // intersect
    SparseHashSet operator|(const SparseHashSet& rhs) const;     // union
    SparseHashSet operator^(const SparseHashSet& rhs) const;     // difference
    SparseHashSet operator-(const SparseHashSet& rhs) const;     // sub
    bool          is_sub_set_of(const SparseHashSet& rhs) const; // sub set

    // support foreach
    StlIt  begin();
    StlIt  end();
    CStlIt begin() const;
    CStlIt end() const;
};
} // namespace skr::container

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
        SKR_ASSERT(HashType()(v) == hint.hash());
        SKR_ASSERT(find(v) == hint);
        hint.ref() = std::forward<U>(v);
        return hint;
    }
    else
    { // add case
        SKR_ASSERT(HashType()(v) == hint.hash());
        SKR_ASSERT(!contains(v));
        DataRef ref = Super::_add_unsafe(hint.hash());
        new (ref.ptr()) SetDataType(std::forward<U>(v));
        return ref;
    }
}
template <typename Memory>
template <typename Pred, typename ConstructFunc, typename AssignFunc>
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
    return ref;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::add_ex_unsafe(HashType hash, Pred&& pred)
{
    if (DataRef ref = Super::_find(hash, std::forward<Pred>(pred)))
    {
        return { ref.ptr(), ref.index(), hash, true };
    }
    else
    {
        return Super::_add_unsafe(hash);
    }
}

// emplace
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::emplace(Args&&... args)
{
    // emplace to data array
    auto data_arr_ref = data_arr().add_unsafe();
    new (&data_arr_ref.ref()._sparse_hash_set_data) SetDataType(std::forward<Args>(args)...);
    data_arr_ref.ref()._sparse_hash_set_hash = HasherType()(data_arr_ref.ref()._sparse_hash_set_data);

    if (DataRef ref = find_ex(
        data_arr_ref.ref()._sparse_hash_set_hash,
        [&data_arr_ref](const SetDataType& k) {
            return k == data_arr_ref.ref()._sparse_hash_set_data;
        }))
    {
        // move data
        memory::move(ref.ptr(), &data_arr_ref.ref()._sparse_hash_set_data);

        // remove placeholder
        data_arr().remove_at_unsafe(data_arr_ref.index());

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
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::emplace(DataRef hint, Args&&... args)
{
    if (hint.is_valid())
    { // assign case
        SetDataType data{ std::forward<Args>(args)... };
        SKR_ASSERT(HasherType()(data) == hint.hash());
        SKR_ASSERT(find(data) == hint);
        hint.ref() = std::move(data);
        return hint;
    }
    else
    { // add case
        auto data_arr_ref = data_arr().add_unsafe();
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
    SizeType count = 0;
    auto     it    = rhs.begin();
    while (slack() > 0 && it != rhs.end())
    {
        add(*it);
        ++it;
        ++count;
    }

    // reserve and add
    if (it != rhs.end())
    {
        auto new_capacity = data_arr().capacity() + (rhs.size() - count);
        data_arr().reserve(new_capacity);

        while (it != rhs.end())
        {
            add(*it);
            ++it;
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
        auto new_capacity = data_arr().capacity() + (init_list.size() - read_idx);
        data_arr().reserve(new_capacity);

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
        auto new_capacity = data_arr().capacity() + (n - read_idx);
        data_arr().reserve(new_capacity);

        while (read_idx < n)
        {
            const auto& v = p[read_idx];
            add(v);
            ++read_idx;
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
SKR_INLINE bool SparseHashSet<Memory>::remove_ex(HashType hash, Pred&& pred)
{
    return Super::_remove(hash, std::forward<Pred>(pred));
}

// erase
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::StlIt SparseHashSet<Memory>::erase(const StlIt& it)
{
    remove_at(it.index());
    StlIt new_it(it);
    ++new_it;
    return new_it;
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CStlIt SparseHashSet<Memory>::erase(const CStlIt& it)
{
    remove_at(it.index());
    CStlIt new_it(it);
    ++new_it;
    return new_it;
}

// find
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find(const U& v)
{
    HashType hash = HasherType()(v);
    return Super::_find(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <TransparentToOrSameAs<typename Memory::SetDataType, typename Memory::HasherType> U>
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find(const U& v) const
{
    HashType hash = HasherType()(v);
    return Super::_find(hash, [&v](const SetDataType& k) { return k == v; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::_find(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::_find(hash, std::forward<Pred>(pred));
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

    for (const auto& v : *this)
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

    for (const auto& v : *this)
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
        for (const auto& v : *this)
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

// support foreach
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::StlIt SparseHashSet<Memory>::begin()
{
    return StlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::StlIt SparseHashSet<Memory>::end()
{
    return StlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CStlIt SparseHashSet<Memory>::begin() const
{
    return CStlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CStlIt SparseHashSet<Memory>::end() const
{
    return CStlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
} // namespace skr::container