#pragma once
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_def.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_iterator.hpp"
#include "SkrBase/containers/misc/container_traits.hpp"

// SparseHashSet def
namespace skr::container
{
template <typename Memory>
struct SparseHashMap : protected SparseHashBase<Memory> {
    using Super = SparseHashBase<Memory>;

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

    // sparse hash map configure
    using typename Memory::MapKeyType;
    using typename Memory::MapValueType;
    using typename Memory::MapDataType;

    // helper
    using DataVector                      = SparseVector<Memory>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref
    using DataRef  = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, false>;
    using CDataRef = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, true>;

    // cursor & iterator
    using Cursor   = SparseHashMapCursor<SparseHashMap, false>;
    using CCursor  = SparseHashMapCursor<SparseHashMap, true>;
    using Iter     = SparseHashMapIter<SparseHashMap, false>;
    using CIter    = SparseHashMapIter<SparseHashMap, true>;
    using IterInv  = SparseHashMapIterInv<SparseHashMap, false>;
    using CIterInv = SparseHashMapIterInv<SparseHashMap, true>;

    // stl-style iterator
    using StlIt  = CursorIterStl<Cursor, false>;
    using CStlIt = CursorIterStl<CCursor, false>;

    // ctor & dtor
    SparseHashMap(AllocatorCtorParam param = {});
    SparseHashMap(SizeType reserve_size, AllocatorCtorParam param = {});
    SparseHashMap(const MapDataType* p, SizeType n, AllocatorCtorParam param = {});
    SparseHashMap(std::initializer_list<MapDataType> init_list, AllocatorCtorParam param = {});
    ~SparseHashMap();

    // copy & move
    SparseHashMap(const SparseHashMap& rhs);
    SparseHashMap(SparseHashMap&& rhs);

    // assign & move assign
    SparseHashMap& operator=(const SparseHashMap& rhs);
    SparseHashMap& operator=(SparseHashMap&& rhs);

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
    template <typename UK = MapKeyType, typename UV = MapValueType>
    requires(
        TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
        std::convertible_to<UV, typename Memory::MapValueType> &&
        std::is_constructible_v<typename Memory::MapValueType, UV>
    )
    DataRef add(UK&& key, UV&& value);
    template <typename UK = MapKeyType, typename UV = MapValueType>
    requires(
        TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
        std::convertible_to<UV, typename Memory::MapValueType> &&
        std::is_constructible_v<typename Memory::MapValueType, UV>
    )
    DataRef add(UK&& key, UV&& value, DataRef hint);
    template <typename Pred, typename ConstructFunc, typename AssignFunc>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
    DataRef add_ex(HashType hash, Pred&& pred, ConstructFunc&& construct, AssignFunc&& assign);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
    DataRef add_ex_unsafe(HashType hash, Pred&& pred);

    // try add (key only add)
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_unsafe(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_default(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_zeroed(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_unsafe(UK&& key, DataRef hint);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_default(UK&& key, DataRef hint);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_zeroed(UK&& key, DataRef hint);

    // emplace
    template <typename UK = MapKeyType, typename... Args>
    requires(
        TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
        std::is_constructible_v<typename Memory::MapValueType, Args...>
    )
    DataRef emplace(UK&& key, Args&&... args);

    // try emplace
    template <typename UK = MapKeyType, typename... Args>
    requires(
        TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
        std::is_constructible_v<typename Memory::MapValueType, Args...>
    )
    DataRef try_emplace(UK&& key, Args&&... args);

    // append
    void append(const SparseHashMap& rhs);
    void append(std::initializer_list<MapDataType> init_list);
    void append(const MapDataType* p, SizeType n);

    // remove
    using Super::remove_at;
    using Super::remove_at_unsafe;
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    bool remove(const UK& key);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
    bool remove_ex(HashType hash, Pred&& pred);

    // remove value
    template <typename UV = MapValueType>
    requires(concepts::HasEq<typename Memory::MapValueType, UV>)
    bool remove_value(const UV& value);
    template <typename UV = MapValueType>
    requires(concepts::HasEq<typename Memory::MapValueType, UV>)
    SizeType remove_all_value(const UV& value);

    // remove if
    using Super::remove_if;
    using Super::remove_last_if;
    using Super::remove_all_if;

    // find
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef find(const UK& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    CDataRef find(const UK& key) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
    DataRef find_ex(HashType hash, Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
    CDataRef find_ex(HashType hash, Pred&& pred) const;

    // find value
    template <typename UV = MapValueType>
    requires(concepts::HasEq<typename Memory::MapValueType, UV>)
    DataRef find_value(const UV& value);
    template <typename UV = MapValueType>
    requires(concepts::HasEq<typename Memory::MapValueType, UV>)
    CDataRef find_value(const UV& value) const;

    // find if
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
    DataRef find_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
    DataRef find_last_if(Pred&& pred);
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
    CDataRef find_if(Pred&& pred) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
    CDataRef find_last_if(Pred&& pred) const;

    // contains
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    bool contains(const UK& key) const;
    template <typename Pred>
    requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
    bool contains_ex(HashType hash, Pred&& pred) const;

    // contains value
    template <typename UV = MapValueType>
    requires(concepts::HasEq<typename Memory::MapValueType, UV>)
    bool contains_value(const UV& value) const;

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
    const SparseHashMap& readonly() const;
};
} // namespace skr::container

// SparseHashSet impl
namespace skr::container
{
// ctor & dtor
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(AllocatorCtorParam param)
    : Super(std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(SizeType reserve_size, AllocatorCtorParam param)
    : Super(std::move(param))
{
    reserve(reserve_size);
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(const MapDataType* p, SizeType n, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(p, n);
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(std::initializer_list<MapDataType> init_list, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(init_list);
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::~SparseHashMap()
{
    // handled by SparseHashBase
}

// copy & move
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(const SparseHashMap& rhs)
    : Super(rhs)
{
    // handled by SparseHashBase
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(SparseHashMap&& rhs)
    : Super(std::move(rhs))
{
    // handled by SparseHashBase
}

// assign & move assign
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>& SparseHashMap<Memory>::operator=(const SparseHashMap& rhs)
{
    Super::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>& SparseHashMap<Memory>::operator=(SparseHashMap&& rhs)
{
    Super::operator=(std::move(rhs));
    return *this;
}

// add
template <typename Memory>
template <typename UK, typename UV>
requires(
    TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
    std::convertible_to<UV, typename Memory::MapValueType> &&
    std::is_constructible_v<typename Memory::MapValueType, UV>
)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(UK&& key, UV&& value)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (ref.already_exist())
    { // assign case
        ref.key()   = std::forward<UK>(key);
        ref.value() = std::forward<UV>(value);
    }
    else
    { // construct case
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        new (&ref.value()) MapValueType(std::forward<UV>(value));
    }
    return ref;
}
template <typename Memory>
template <typename UK, typename UV>
requires(
    TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
    std::convertible_to<UV, typename Memory::MapValueType> &&
    std::is_constructible_v<typename Memory::MapValueType, UV>
)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(UK&& key, UV&& value, DataRef hint)
{
    if (hint.is_valid())
    { // assign case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(find(key) == hint);
        hint.key()   = std::forward<UK>(key);
        hint.value() = std::forward<UV>(value);
        return {
            hint.ptr(),
            hint.index(),
            hint.hash(),
            true,
        };
    }
    else
    { // construct case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(!contains(key));
        DataRef ref = Super::template _add_unsafe<DataRef>(hint.hash());
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        new (&ref.value()) MapValueType(std::forward<UV>(value));
        return ref;
    }
}
template <typename Memory>
template <typename Pred, typename ConstructFunc, typename AssignFunc>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_ex(HashType hash, Pred&& pred, ConstructFunc&& construct, AssignFunc&& assign)
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
    SKR_ASSERT(HasherType()(ref.ref().key) == hash);
    return ref;
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_ex_unsafe(HashType hash, Pred&& pred)
{
    if (DataRef ref = Super::template _find<DataRef>(hash, [&pred](const MapDataType& data) { return pred(data.key); }))
    {
        return { ref.ptr(), ref.index(), hash, true };
    }
    else
    {
        return Super::template _add_unsafe<DataRef>(hash);
    }
}

// try add
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add_unsafe(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (!ref.already_exist())
    {
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
    }
    return ref;
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add_default(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (!ref.already_exist())
    {
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        ::skr::memory::construct(&ref.value());
    }
    return ref;
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add_zeroed(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (!ref.already_exist())
    {
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        memset(&ref.value(), 0, sizeof(MapValueType));
    }
    return ref;
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add_unsafe(UK&& key, DataRef hint)
{
    if (hint.is_valid())
    { // assign case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(find(key) == hint);
        return {
            hint.ptr(),
            hint.index(),
            hint.hash(),
            true,
        };
    }
    else
    { // construct case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(!contains(key));
        DataRef ref = Super::template _add_unsafe<DataRef>(hint.hash());
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        return ref;
    }
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add_default(UK&& key, DataRef hint)
{
    if (hint.is_valid())
    { // assign case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(find(key) == hint);
        return {
            hint.ptr(),
            hint.index(),
            hint.hash(),
            true,
        };
    }
    else
    { // construct case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(!contains(key));
        DataRef ref = Super::template _add_unsafe<DataRef>(hint.hash());
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        ::skr::memory::construct(&ref.value());
        return ref;
    }
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add_zeroed(UK&& key, DataRef hint)
{
    if (hint.is_valid())
    { // assign case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(find(key) == hint);
        return {
            hint.ptr(),
            hint.index(),
            hint.hash(),
            true,
        };
    }
    else
    { // construct case
        SKR_ASSERT(HasherType()(key) == hint.hash());
        SKR_ASSERT(!contains(key));
        DataRef ref = Super::template _add_unsafe<DataRef>(hint.hash());
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        memset(&ref.value(), 0, sizeof(MapValueType));
        return ref;
    }
}

// emplace
template <typename Memory>
template <typename UK, typename... Args>
requires(
    TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
    std::is_constructible_v<typename Memory::MapValueType, Args...>
)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::emplace(UK&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (ref.already_exist())
    {
        ref.key()   = std::forward<UK>(key);
        ref.value() = MapValueType{ std::forward<Args>(args)... };
    }
    else
    {
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        new (&ref.value()) MapValueType(std::forward<Args>(args)...);
    }
    return ref;
}

// try emplace
template <typename Memory>
template <typename UK, typename... Args>
requires(
    TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
    std::is_constructible_v<typename Memory::MapValueType, Args...>
)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_emplace(UK&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (!ref.already_exist())
    {
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        new (&ref.value()) MapValueType(std::forward<Args>(args)...);
    }
    return ref;
}

// append
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(const SparseHashMap& rhs)
{
    // fill slack
    SizeType count = 0;
    auto     iter  = rhs.iter();
    while (slack() > 0 && iter.has_next())
    {
        add(iter.ref().key, iter.ref().value);
        iter.move_next();
        ++count;
    }

    // reserve and add
    if (iter.has_next())
    {
        auto new_capacity = data_vector().capacity() + (rhs.size() - count);
        data_vector().reserve(new_capacity);

        while (iter.has_next())
        {
            add(iter.ref().key, iter.ref().value);
            iter.move_next();
        }
    }
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(std::initializer_list<MapDataType> init_list)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < init_list.size())
    {
        const auto& v = init_list.begin()[read_idx];
        add(v.key, v.value);
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
            add(v.key, v.value);
            ++read_idx;
        }
    }
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(const MapDataType* p, SizeType n)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < n)
    {
        const auto& v = p[read_idx];
        add(v.key, v.value);
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
            add(v.key, v.value);
            ++read_idx;
        }
    }
}

// remove
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE bool SparseHashMap<Memory>::remove(const UK& key)
{
    HashType hash = HasherType()(key);
    return Super::_remove(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
SKR_INLINE bool SparseHashMap<Memory>::remove_ex(HashType hash, Pred&& pred)
{
    return Super::_remove(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}

// remove value
template <typename Memory>
template <typename UV>
requires(concepts::HasEq<typename Memory::MapValueType, UV>)
SKR_INLINE bool SparseHashMap<Memory>::remove_value(const UV& value)
{
    return remove_if([&value](const MapDataType& data) { return data.value == value; });
}
template <typename Memory>
template <typename UV>
requires(concepts::HasEq<typename Memory::MapValueType, UV>)
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::remove_all_value(const UV& value)
{
    return remove_all_if([&value](const MapDataType& data) { return data.value == value; });
}

// find
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find(const UK& key)
{
    HashType hash = HasherType()(key);
    return Super::template _find<DataRef>(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find(const UK& key) const
{
    HashType hash = HasherType()(key);
    return Super::template _find<CDataRef>(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::template _find<DataRef>(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::template _find<CDataRef>(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}

// find value
template <typename Memory>
template <typename UV>
requires(concepts::HasEq<typename Memory::MapValueType, UV>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_value(const UV& value)
{
    return find_if([&value](const MapDataType& data) { return data.value == value; });
}
template <typename Memory>
template <typename UV>
requires(concepts::HasEq<typename Memory::MapValueType, UV>)
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_value(const UV& value) const
{
    return find_if([&value](const MapDataType& data) { return data.value == value; });
}

// find if
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_if(Pred&& pred)
{
    return Super::template _find_if<DataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_last_if(Pred&& pred)
{
    return Super::template _find_last_if<DataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_if(Pred&& pred) const
{
    return Super::template _find_if<CDataRef>(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapDataType&>)
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_last_if(Pred&& pred) const
{
    return Super::template _find_last_if<CDataRef>(std::forward<Pred>(pred));
}

// contains
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE bool SparseHashMap<Memory>::contains(const UK& key) const
{
    return (bool)find(key);
}
template <typename Memory>
template <typename Pred>
requires(std::is_invocable_r_v<bool, Pred, const typename Memory::MapKeyType&>)
SKR_INLINE bool SparseHashMap<Memory>::contains_ex(HashType hash, Pred&& pred) const
{
    return (bool)find_ex(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename UV>
requires(concepts::HasEq<typename Memory::MapValueType, UV>)
SKR_INLINE bool SparseHashMap<Memory>::contains_value(const UV& value) const
{
    return (bool)find_value(value);
}

// cursor & iterator
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::Cursor SparseHashMap<Memory>::cursor_begin()
{
    return Cursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CCursor SparseHashMap<Memory>::cursor_begin() const
{
    return CCursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::Cursor SparseHashMap<Memory>::cursor_end()
{
    return Cursor::End(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CCursor SparseHashMap<Memory>::cursor_end() const
{
    return CCursor::End(this);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::Iter SparseHashMap<Memory>::iter()
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CIter SparseHashMap<Memory>::iter() const
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::IterInv SparseHashMap<Memory>::iter_inv()
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CIterInv SparseHashMap<Memory>::iter_inv() const
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE auto SparseHashMap<Memory>::range()
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto SparseHashMap<Memory>::range() const
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto SparseHashMap<Memory>::range_inv()
{
    return cursor_end().as_range_inv();
}
template <typename Memory>
SKR_INLINE auto SparseHashMap<Memory>::range_inv() const
{
    return cursor_end().as_range_inv();
}

// stl-style iterator
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::StlIt SparseHashMap<Memory>::begin()
{
    return { Cursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CStlIt SparseHashMap<Memory>::begin() const
{
    return { CCursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::StlIt SparseHashMap<Memory>::end()
{
    return { Cursor::EndOverflow(this) };
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CStlIt SparseHashMap<Memory>::end() const
{
    return { CCursor::EndOverflow(this) };
}

// syntax
template <typename Memory>
SKR_INLINE const SparseHashMap<Memory>& SparseHashMap<Memory>::readonly() const
{
    return *this;
}
} // namespace skr::container

// container traits
namespace skr::container
{
template <typename Memory>
struct ContainerTraits<SparseHashMap<Memory>> {
    constexpr static bool is_linear_memory = false; // data(), size()
    constexpr static bool has_size         = true;  // size()
    constexpr static bool is_iterable      = true;  // begin(), end()
    constexpr static bool is_reservable    = true;  // reserve()

    using ElementType = typename Memory::DataType;
    using SizeType    = typename Memory::SizeType;

    inline static SizeType size(const SparseHashMap<Memory>& container) { return container.size(); }

    inline static typename SparseHashMap<Memory>::StlIt  begin(SparseHashMap<Memory>& set) { return set.begin(); }
    inline static typename SparseHashMap<Memory>::StlIt  end(SparseHashMap<Memory>& set) { return set.end(); }
    inline static typename SparseHashMap<Memory>::CStlIt begin(const SparseHashMap<Memory>& set) { return set.begin(); }
    inline static typename SparseHashMap<Memory>::CStlIt end(const SparseHashMap<Memory>& set) { return set.end(); }

    inline static void reserve(SparseHashMap<Memory>& container, SizeType new_capacity) { container.reserve(new_capacity); }
};
} // namespace skr::container