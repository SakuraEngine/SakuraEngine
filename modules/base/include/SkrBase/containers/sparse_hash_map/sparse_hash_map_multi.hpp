#pragma once
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_def.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_iterator.hpp"

namespace skr::container
{
template <typename Memory>
struct MultiSparseHashMap : protected SparseHashBase<Memory> {
    using Super = SparseHashBase<Memory>;

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

    // sparse hash map configure
    using typename Memory::MapKeyType;
    using typename Memory::MapValueType;
    using typename Memory::MapDataType;

    // helper
    using DataArr                         = SparseArray<Memory>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref
    using DataRef  = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, false>;
    using CDataRef = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, true>;

    // cursor & iterator
    using Cursor   = SparseHashMapCursor<MultiSparseHashMap, false>;
    using CCursor  = SparseHashMapCursor<MultiSparseHashMap, true>;
    using Iter     = SparseHashMapIter<MultiSparseHashMap, false>;
    using CIter    = SparseHashMapIter<MultiSparseHashMap, true>;
    using IterInv  = SparseHashMapIterInv<MultiSparseHashMap, false>;
    using CIterInv = SparseHashMapIterInv<MultiSparseHashMap, true>;

    // stl-style iterator
    using StlIt  = CursorIterStl<Cursor, false>;
    using CStlIt = CursorIterStl<CCursor, false>;

    // ctor & dtor
    MultiSparseHashMap(AllocatorCtorParam param = {});
    MultiSparseHashMap(SizeType reserve_size, AllocatorCtorParam param = {});
    MultiSparseHashMap(const MapDataType* p, SizeType n, AllocatorCtorParam param = {});
    MultiSparseHashMap(std::initializer_list<MapDataType> init_list, AllocatorCtorParam param = {});
    ~MultiSparseHashMap();

    // copy & move
    MultiSparseHashMap(const MultiSparseHashMap& rhs);
    MultiSparseHashMap(MultiSparseHashMap&& rhs);

    // assign & move assign
    MultiSparseHashMap& operator=(const MultiSparseHashMap& rhs);
    MultiSparseHashMap& operator=(MultiSparseHashMap&& rhs);

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
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
             std::convertible_to<UV, typename Memory::MapValueType>)
    DataRef add(UK&& key, UV&& value);
    template <typename ConstructFunc>
    DataRef add_ex(HashType hash, ConstructFunc&& construct);
    DataRef add_ex_unsafe(HashType hash);

    // key only add
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef add_unsafe(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef add_default(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef add_zeroed(UK&& key);

    // emplace
    template <typename UK = MapKeyType, typename... Args>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef emplace(UK&& key, Args&&... args);

    // append
    void append(const MultiSparseHashMap& rhs);
    void append(std::initializer_list<MapDataType> init_list);
    void append(const MapDataType* p, SizeType n);

    // remove
    using Super::remove_at;
    using Super::remove_at_unsafe;
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    bool remove(const UK& key);
    template <typename Pred>
    bool remove_ex(HashType hash, Pred&& pred);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    SizeType remove_all(const UK& key);
    template <typename Pred>
    SizeType remove_all_ex(HashType hash, Pred&& pred);

    // remove value
    template <typename UV = MapValueType>
    bool remove_value(const UV& value);
    template <typename UV = MapValueType>
    bool remove_all_value(const UV& value);

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
    DataRef find_ex(HashType hash, Pred&& pred);
    template <typename Pred>
    CDataRef find_ex(HashType hash, Pred&& pred) const;
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef find_next(DataRef ref, const UK& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    CDataRef find_next(CDataRef ref, const UK& key) const;
    template <typename Pred>
    DataRef find_next_ex(DataRef ref, Pred&& pred);
    template <typename Pred>
    CDataRef find_next_ex(CDataRef ref, Pred&& pred) const;

    // find value
    template <typename UV = MapValueType>
    DataRef find_value(const UV& value);
    template <typename UV = MapValueType>
    CDataRef find_value(const UV& value) const;

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
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    bool contains(const UK& key) const;
    template <typename Pred>
    bool contains_ex(HashType hash, Pred&& pred) const;
    template <typename UV = MapValueType>
    bool contains_value(const UV& value) const;
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    SizeType count(const UK& key) const;
    template <typename Pred>
    SizeType count_ex(HashType hash, Pred&& pred) const;
    template <typename UV = MapValueType>
    SizeType count_value(const UV& value) const;

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
    const MultiSparseHashMap& readonly() const;
};
} // namespace skr::container

namespace skr::container
{
// ctor & dtor
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>::MultiSparseHashMap(AllocatorCtorParam param)
    : Super(std::move(param))
{
}
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>::MultiSparseHashMap(SizeType reserve_size, AllocatorCtorParam param)
    : Super(std::move(param))
{
    reserve(reserve_size);
}
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>::MultiSparseHashMap(const MapDataType* p, SizeType n, AllocatorCtorParam param)
{
    append(p, n);
}
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>::MultiSparseHashMap(std::initializer_list<MapDataType> init_list, AllocatorCtorParam param)
{
    append(init_list);
}
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>::~MultiSparseHashMap()
{
    // handled by SparseHashBase
}

// copy & move
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>::MultiSparseHashMap(const MultiSparseHashMap& rhs)
    : Super(rhs)
{
    // handled by SparseHashBase
}
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>::MultiSparseHashMap(MultiSparseHashMap&& rhs)
    : Super(rhs)
{
    // handled by SparseHashBase
}

// assign & move assign
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>& MultiSparseHashMap<Memory>::operator=(const MultiSparseHashMap& rhs)
{
    Super::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE MultiSparseHashMap<Memory>& MultiSparseHashMap<Memory>::operator=(MultiSparseHashMap&& rhs)
{
    Super::operator=(std::move(rhs));
    return *this;
}

// add
template <typename Memory>
template <typename UK, typename UV>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
         std::convertible_to<UV, typename Memory::MapValueType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add(UK&& key, UV&& value)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash);
    new (&ref.key()) MapKeyType{ std::move(key) };
    new (&ref.value()) MapValueType{ std::move(value) };
    return ref;
}
template <typename Memory>
template <typename ConstructFunc>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add_ex(HashType hash, ConstructFunc&& construct)
{
    DataRef ref = add_ex_unsafe(hash);
    construct(ref.ptr());
    SKR_ASSERT(HasherType()(ref.ref().key) == hash);
    return ref;
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add_ex_unsafe(HashType hash)
{
    return Super::template _add_unsafe<DataRef>(hash);
}

// key only add
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add_unsafe(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash);
    new (&ref.key()) MapKeyType{ std::move(key) };
    return ref;
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add_default(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash);
    new (&ref.key()) MapKeyType{ std::move(key) };
    memory::construct(&ref.value());
    return ref;
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add_zeroed(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash);
    new (&ref.key()) MapKeyType{ std::move(key) };
    memset(&ref.value(), 0, sizeof(MapValueType));
    return ref;
}

// emplace
template <typename Memory>
template <typename UK, typename... Args>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::emplace(UK&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash);
    new (&ref.key()) MapKeyType{ std::move(key) };
    new (&ref.value()) MapValueType{ std::forward<Args>(args)... };
    return ref;
}

// append
template <typename Memory>
SKR_INLINE void MultiSparseHashMap<Memory>::append(const MultiSparseHashMap& rhs)
{
    reserve(size() + rhs.size());

    for (auto& v : rhs)
    {
        add(v.key, v.value);
    }
}
template <typename Memory>
SKR_INLINE void MultiSparseHashMap<Memory>::append(std::initializer_list<MapDataType> init_list)
{
    reserve(size() + init_list.size());

    for (auto& v : init_list)
    {
        add(v.key, v.value);
    }
}
template <typename Memory>
SKR_INLINE void MultiSparseHashMap<Memory>::append(const MapDataType* p, SizeType n)
{
    reserve(size() + n);

    for (SizeType i = 0; i < n; ++i)
    {
        add(p[i].key, p[i].value);
    }
}

// remove
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE bool MultiSparseHashMap<Memory>::remove(const UK& key)
{
    HashType hash = HasherType()(key);
    return Super::_remove(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool MultiSparseHashMap<Memory>::remove_ex(HashType hash, Pred&& pred)
{
    return Super::_remove(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::SizeType MultiSparseHashMap<Memory>::remove_all(const UK& key)
{
    HashType hash = HasherType()(key);
    return Super::_remove_all(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::SizeType MultiSparseHashMap<Memory>::remove_all_ex(HashType hash, Pred&& pred)
{
    return Super::_remove_all(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}

// remove value
template <typename Memory>
template <typename UV>
SKR_INLINE bool MultiSparseHashMap<Memory>::remove_value(const UV& value)
{
    return remove_if([&value](const MapDataType& data) { return data.value == value; });
}
template <typename Memory>
template <typename UV>
SKR_INLINE bool MultiSparseHashMap<Memory>::remove_all_value(const UV& value)
{
    return remove_all_if([&value](const MapDataType& data) { return data.value == value; });
}

// find
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find(const UK& key)
{
    HashType hash = HasherType()(key);
    return Super::template _find<DataRef>(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find(const UK& key) const
{
    HashType hash = HasherType()(key);
    return Super::template _find<CDataRef>(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::template _find<DataRef>(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::template _find<CDataRef>(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_next(DataRef ref, const UK& key)
{
    return Super::template _find_next<DataRef>(ref, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_next(CDataRef ref, const UK& key) const
{
    return Super::template _find_next<CDataRef>(ref, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_next_ex(DataRef ref, Pred&& pred)
{
    return Super::template _find_next<DataRef>(ref, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_next_ex(CDataRef ref, Pred&& pred) const
{
    return Super::template _find_next<CDataRef>(ref, [&pred](const MapDataType& data) { return pred(data.key); });
}

// find value
template <typename Memory>
template <typename UV>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_value(const UV& value)
{
    return find_if([&value](const MapDataType& data) { return data.value == value; });
}
template <typename Memory>
template <typename UV>
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_value(const UV& value) const
{
    return find_if([&value](const MapDataType& data) { return data.value == value; });
}

// find if
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_if(Pred&& pred)
{
    return Super::template _find_if<DataRef>(std::forward(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_last_if(Pred&& pred)
{
    return Super::template _find_last_if<DataRef>(std::forward(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_if(Pred&& pred) const
{
    return Super::template _find_if<CDataRef>(std::forward(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_last_if(Pred&& pred) const
{
    return Super::template _find_last_if<CDataRef>(std::forward(pred));
}

// contains
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE bool MultiSparseHashMap<Memory>::contains(const UK& key) const
{
    return (bool)find(key);
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool MultiSparseHashMap<Memory>::contains_ex(HashType hash, Pred&& pred) const
{
    return (bool)find_ex(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename UV>
SKR_INLINE bool MultiSparseHashMap<Memory>::contains_value(const UV& value) const
{
    return (bool)find_value(value);
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::SizeType MultiSparseHashMap<Memory>::count(const UK& key) const
{
    SizeType count = 0;

    auto pred = [&key](const MapDataType& data) { return data.key == key; };
    auto ref  = Super::template _find<CDataRef>(HasherType()(key), pred);
    while (ref.is_valid())
    {
        ++count;
        ref = Super::template _find_next<CDataRef>(ref, pred);
    };

    return count;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::SizeType MultiSparseHashMap<Memory>::count_ex(HashType hash, Pred&& pred) const
{
    SizeType count = 0;

    auto ref = Super::template _find<CDataRef>(hash, [&pred](const MapDataType& data) { return pred(data.key); });
    while (ref.is_valid())
    {
        ++count;
        ref = Super::template _find_next<CDataRef>(ref, [&pred](const MapDataType& data) { return pred(data.key); });
    };

    return count;
}
template <typename Memory>
template <typename UV>
SKR_INLINE typename MultiSparseHashMap<Memory>::SizeType MultiSparseHashMap<Memory>::count_value(const UV& value) const
{
    return count_if([&value](const MapDataType& data) { return data.value == value; });
}

// cursor & iterator
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::Cursor MultiSparseHashMap<Memory>::cursor_begin()
{
    return Cursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CCursor MultiSparseHashMap<Memory>::cursor_begin() const
{
    return CCursor::Begin(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::Cursor MultiSparseHashMap<Memory>::cursor_end()
{
    return Cursor::End(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CCursor MultiSparseHashMap<Memory>::cursor_end() const
{
    return CCursor::End(this);
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::Iter MultiSparseHashMap<Memory>::iter()
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CIter MultiSparseHashMap<Memory>::iter() const
{
    return { cursor_begin() };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::IterInv MultiSparseHashMap<Memory>::iter_inv()
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CIterInv MultiSparseHashMap<Memory>::iter_inv() const
{
    return { cursor_end() };
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashMap<Memory>::range()
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashMap<Memory>::range() const
{
    return cursor_begin().as_range();
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashMap<Memory>::range_inv()
{
    return cursor_end().as_range_inv();
}
template <typename Memory>
SKR_INLINE auto MultiSparseHashMap<Memory>::range_inv() const
{
    return cursor_end().as_range_inv();
}

// stl-style iterator
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::StlIt MultiSparseHashMap<Memory>::begin()
{
    return { Cursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CStlIt MultiSparseHashMap<Memory>::begin() const
{
    return { CCursor::Begin(this) };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::StlIt MultiSparseHashMap<Memory>::end()
{
    return { Cursor::EndOverflow(this) };
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CStlIt MultiSparseHashMap<Memory>::end() const
{
    return { CCursor::EndOverflow(this) };
}

// syntax
template <typename Memory>
SKR_INLINE const MultiSparseHashMap<Memory>& MultiSparseHashMap<Memory>::readonly() const
{
}
} // namespace skr::container