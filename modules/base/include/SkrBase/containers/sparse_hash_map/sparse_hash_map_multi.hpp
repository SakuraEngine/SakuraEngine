#pragma once
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_def.hpp"

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

    // data ref & iterator
    using DataRef  = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, false>;
    using CDataRef = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, true>;
    using StlIt    = typename Super::StlIt;
    using CStlIt   = typename Super::CStlIt;
    // data ref & iterator
    using DataRef  = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, false>;
    using CDataRef = SparseHashMapDataRef<MapKeyType, MapValueType, SizeType, HashType, true>;
    using StlIt    = typename Super::StlIt;
    using CStlIt   = typename Super::CStlIt;

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
    DataRef add(UK&& key);
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

    // erase
    StlIt  erase(const StlIt& it);
    CStlIt erase(const CStlIt& it);

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
    using Super::find_if;
    using Super::find_last_if;

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
    using Super::modify_at;
    using Super::modify_last;
    using Super::modify;

    // sort
    using Super::sort;
    using Super::sort_stable;

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
    return ref;
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add_ex_unsafe(HashType hash)
{
    return Super::_add_unsafe(hash);
}

// key only add
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::add(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash);
    new (&ref.key()) MapKeyType{ std::move(key) };
    memory::construct_stl_ub(&ref.value());
    return ref;
}
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

// erase
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::StlIt MultiSparseHashMap<Memory>::erase(const StlIt& it)
{
    remove_at(it.index());
    StlIt new_it{ it };
    ++new_it;
    return new_it;
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CStlIt MultiSparseHashMap<Memory>::erase(const CStlIt& it)
{
    remove_at(it.index());
    CStlIt new_it{ it };
    ++new_it;
    return new_it;
}

// find
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find(const UK& key)
{
    HashType hash = HasherType()(key);
    return Super::_find(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find(const UK& key) const
{
    HashType hash = HasherType()(key);
    return Super::_find(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::_find(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::_find(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_next(DataRef ref, const UK& key)
{
    return Super::_find_next(ref, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_next(CDataRef ref, const UK& key) const
{
    return Super::_find_next(ref, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::DataRef MultiSparseHashMap<Memory>::find_next_ex(DataRef ref, Pred&& pred)
{
    return Super::_find_next(ref, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::CDataRef MultiSparseHashMap<Memory>::find_next_ex(CDataRef ref, Pred&& pred) const
{
    return Super::_find_next(ref, [&pred](const MapDataType& data) { return pred(data.key); });
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
    auto ref  = Super::_find(HasherType()(key), pred);
    while (ref.is_valid())
    {
        ++count;
        ref = Super::_find_next(ref, pred);
    };

    return count;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename MultiSparseHashMap<Memory>::SizeType MultiSparseHashMap<Memory>::count_ex(HashType hash, Pred&& pred) const
{
    SizeType count = 0;

    auto ref = Super::_find(hash, [&pred](const MapDataType& data) { return pred(data.key); });
    while (ref.is_valid())
    {
        ++count;
        ref = Super::_find_next(ref, [&pred](const MapDataType& data) { return pred(data.key); });
    };

    return count;
}
template <typename Memory>
template <typename UV>
SKR_INLINE typename MultiSparseHashMap<Memory>::SizeType MultiSparseHashMap<Memory>::count_value(const UV& value) const
{
    return count_if([&value](const MapDataType& data) { return data.value == value; });
}

// support foreach
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::StlIt MultiSparseHashMap<Memory>::begin()
{
    return StlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::StlIt MultiSparseHashMap<Memory>::end()
{
    return StlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CStlIt MultiSparseHashMap<Memory>::begin() const
{
    return CStlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename MultiSparseHashMap<Memory>::CStlIt MultiSparseHashMap<Memory>::end() const
{
    return CStlIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
} // namespace skr::container