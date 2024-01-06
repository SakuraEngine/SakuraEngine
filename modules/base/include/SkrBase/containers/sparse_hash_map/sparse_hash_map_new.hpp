#pragma once
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_base.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_def.hpp"

namespace skr::container
{
template <typename Memory>
struct SparseHashMap : protected SparseHashBase<Memory> {
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
    using It       = typename Super::It;
    using CIt      = typename Super::CIt;

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
    template <typename UK = MapKeyType, typename UV = MapValueType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
             std::convertible_to<UV, typename Memory::MapValueType>)
    DataRef add(UK&& key, UV&& value, DataRef hint);
    template <typename Pred, typename ConstructFunc, typename AssignFunc>
    DataRef add_ex(HashType hash, Pred&& pred, ConstructFunc&& construct, AssignFunc&& assign);
    template <typename Pred>
    DataRef add_ex_unsafe(HashType hash, Pred&& pred);

    // try add
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_unsafe(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_default(UK&& key);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_add_zeroed(UK&& key);

    // emplace
    template <typename UK = MapKeyType, typename... Args>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef emplace(UK&& key, Args&&... args);

    // try emplace
    template <typename UK = MapKeyType, typename... Args>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    DataRef try_emplace(UK&& key, Args&&... args);

    // append
    void append(const SparseHashMap& rhs);
    void append(std::initializer_list<MapDataType> init_list);
    void append(const MapDataType* p, SizeType n);

    // remove
    void remove_at(SizeType index);
    void remove_at_unsafe(SizeType index);
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    bool remove(const UK& key);
    template <typename Pred>
    bool remove_ex(HashType hash, Pred&& pred);

    // remove value
    template <typename UV = MapValueType>
    bool remove_value(const UV& value);
    template <typename UV = MapValueType>
    bool remove_all_value(const UV& value);

    // remove if

    // erase
    It  erase(const It& it);
    CIt erase(const CIt& it);

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

    // find value
    template <typename UV = MapValueType>
    DataRef find_value(const UV& value);
    template <typename UV = MapValueType>
    CDataRef find_value(const UV& value) const;

    // find if

    // contains
    template <typename UK = MapKeyType>
    requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
    bool contains(const UK& key) const;
    template <typename Pred>
    bool contains_ex(HashType hash, Pred&& pred) const;
    template <typename UV = MapValueType>
    bool contains_value(const UV& value) const;

    // contains if

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
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
         std::convertible_to<UV, typename Memory::MapValueType>)
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
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType> &&
         std::convertible_to<UV, typename Memory::MapValueType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(UK&& key, UV&& value, DataRef hint)
{
    if (hint.is_valid())
    { // assign case
        SKR_ASSERT(HashType()(key) == hint.hash());
        SKR_ASSERT(find(key) == hint);
        hint.key()   = std::forward<UK>(key);
        hint.value() = std::forward<UV>(value);
        return hint;
    }
    else
    { // construct case
        SKR_ASSERT(HashType()(key) == hint.hash());
        SKR_ASSERT(!contains(key));
        DataRef ref = Super::_add_unsafe(hint.hash());
        new (&ref.key()) MapKeyType(std::forward<UK>(key));
        new (&ref.value()) MapValueType(std::forward<UV>(value));
        return ref;
    }
}
template <typename Memory>
template <typename Pred, typename ConstructFunc, typename AssignFunc>
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
    return ref;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_ex_unsafe(HashType hash, Pred&& pred)
{
    if (DataRef ref = Super::_find(hash, [&pred](const MapDataType& data) { return pred(data.key); }))
    {
        return { ref.ptr(), ref.index(), hash, true };
    }
    else
    {
        return Super::_add_unsafe(hash);
    }
}

// try add
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (!ref.already_exist())
    {
        ref.key() = std::forward<UK>(key);
        memory::construct_stl_ub(&ref.value());
    }
    return ref;
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_add_unsafe(UK&& key)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (!ref.already_exist())
    {
        ref.key() = std::forward<UK>(key);
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
        ref.key() = std::forward<UK>(key);
        memory::construct(&ref.value());
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
        ref.key() = std::forward<UK>(key);
        memset(&ref.value(), 0, sizeof(MapValueType));
    }
    return ref;
}

// emplace
template <typename Memory>
template <typename UK, typename... Args>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
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
        ref.key() = std::forward<UK>(key);
        memory::construct(&ref.value(), std::forward<Args>(args)...);
    }
    return ref;
}

// try emplace
template <typename Memory>
template <typename UK, typename... Args>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_emplace(UK&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    DataRef  ref  = add_ex_unsafe(hash, [&key](const MapKeyType& k) { return k == key; });
    if (!ref.already_exist())
    {
        ref.key() = std::forward<UK>(key);
        memory::construct(&ref.value(), std::forward<Args>(args)...);
    }
    return ref;
}

// append
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(const SparseHashMap& rhs)
{
    // fill slack
    SizeType count = 0;
    auto     it    = rhs.begin();
    while (slack() > 0 && it != rhs.end())
    {
        add(it->key, it->value);
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
            add(it->key, it->value);
            ++it;
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
        auto new_capacity = data_arr().capacity() + (init_list.size() - read_idx);
        data_arr().reserve(new_capacity);

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
        auto new_capacity = data_arr().capacity() + (n - read_idx);
        data_arr().reserve(new_capacity);

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
SKR_INLINE void SparseHashMap<Memory>::remove_at(SizeType index)
{
    Super::_remove_from_bucket(index);
    data_arr().remove_at(index);
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::remove_at_unsafe(SizeType index)
{
    Super::_remove_from_bucket(index);
    data_arr().remove_at_unsafe(index);
}
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
SKR_INLINE bool SparseHashMap<Memory>::remove_ex(HashType hash, Pred&& pred)
{
    return Super::_remove(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}

// remove value
template <typename Memory>
template <typename UV>
SKR_INLINE bool SparseHashMap<Memory>::remove_value(const UV& value)
{
    return data_arr().remove_if([&value](const MapDataType& data) { return data.value == value; });
}
template <typename Memory>
template <typename UV>
SKR_INLINE bool SparseHashMap<Memory>::remove_all_value(const UV& value)
{
    return data_arr().remove_all_if([&value](const MapDataType& data) { return data.value == value; });
}

// erase
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::It SparseHashMap<Memory>::erase(const It& it)
{
    remove_at(it.index());
    It new_it{ it };
    ++new_it;
    return new_it;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CIt SparseHashMap<Memory>::erase(const CIt& it)
{
    remove_at(it.index());
    CIt new_it{ it };
    ++new_it;
    return new_it;
}

// find
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find(const UK& key)
{
    HashType hash = HasherType()(key);
    return Super::_find(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename UK>
requires(TransparentToOrSameAs<UK, typename Memory::MapKeyType, typename Memory::HasherType>)
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find(const UK& key) const
{
    HashType hash = HasherType()(key);
    return Super::_find(hash, [&key](const MapDataType& data) { return data.key == key; });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred)
{
    return Super::_find(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_ex(HashType hash, Pred&& pred) const
{
    return Super::_find(hash, [&pred](const MapDataType& data) { return pred(data.key); });
}

// find value
template <typename Memory>
template <typename UV>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_value(const UV& value)
{
    return data_arr().find_if([&value](const MapDataType& data) { return data.value == value; });
}
template <typename Memory>
template <typename UV>
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_value(const UV& value) const
{
    return data_arr().find_if([&value](const MapDataType& data) { return data.value == value; });
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
SKR_INLINE bool SparseHashMap<Memory>::contains_ex(HashType hash, Pred&& pred) const
{
    return (bool)find_ex(hash, std::forward<Pred>(pred));
}
template <typename Memory>
template <typename UV>
SKR_INLINE bool SparseHashMap<Memory>::contains_value(const UV& value) const
{
    return (bool)find_value(value);
}

// support foreach
template <typename Memory>
typename SparseHashMap<Memory>::It SparseHashMap<Memory>::begin()
{
    return It(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
typename SparseHashMap<Memory>::It SparseHashMap<Memory>::end()
{
    return It(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
template <typename Memory>
typename SparseHashMap<Memory>::CIt SparseHashMap<Memory>::begin() const
{
    return CIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
typename SparseHashMap<Memory>::CIt SparseHashMap<Memory>::end() const
{
    return CIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}

} // namespace skr::container
