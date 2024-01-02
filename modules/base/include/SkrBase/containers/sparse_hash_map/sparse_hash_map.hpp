#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_def.hpp"

// SparseHashMap def
namespace skr::container
{
template <typename Memory>
struct SparseHashMap : protected SparseHashSet<Memory> {
    using Super = SparseHashSet<Memory>;

    // sparse array configure
    using typename Memory::SizeType;
    using typename Memory::DataType;
    using typename Memory::StorageType;
    using typename Memory::BitBlockType;
    using typename Memory::AllocatorCtorParam;

    // sparse hash set configure
    using typename Memory::HashType;
    using typename Memory::KeyType;
    using typename Memory::KeyMapperType;
    using typename Memory::HasherType;
    using typename Memory::SetDataType;
    using typename Memory::SetStorageType;
    using Memory::allow_multi_key;

    // sparse hash map configure
    using typename Memory::MapKeyType;
    using typename Memory::MapValueType;
    using typename Memory::MapDataType;

    // from super
    using DataArr = typename Super::DataArr;

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
    SparseHashMap(const SparseHashMap& other);
    SparseHashMap(SparseHashMap&& other);

    // assign & move assign
    SparseHashMap& operator=(const SparseHashMap& rhs);
    SparseHashMap& operator=(SparseHashMap&& rhs);

    // getter
    SizeType       size() const;
    SizeType       capacity() const;
    SizeType       slack() const;
    SizeType       sparse_size() const;
    SizeType       hole_size() const;
    SizeType       bit_array_size() const;
    SizeType       free_list_head() const;
    bool           is_compact() const;
    bool           empty() const;
    DataArr&       data_arr();
    const DataArr& data_arr() const;
    Super&         data_set();
    const Super&   data_set() const;
    Memory&        memory();
    const Memory&  memory() const;

    // validate
    bool has_data(SizeType idx) const;
    bool is_hole(SizeType idx) const;
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const void* p) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    bool compact();
    bool compact_stable();
    bool compact_top();

    // rehash
    void rehash() const;
    bool rehash_if_need() const;

    // add, move behavior may not happened here, just for easy to use
    DataRef add(const MapKeyType& key, const MapValueType& value);
    DataRef add(const MapKeyType& key, MapValueType&& value);
    DataRef add(MapKeyType&& key, const MapValueType& value);
    DataRef add(MapKeyType&& key, MapValueType&& value);
    DataRef add(const MapKeyType& key);
    DataRef add(MapKeyType&& key);
    DataRef add_unsafe(const MapKeyType& key);
    DataRef add_unsafe(MapKeyType&& key);
    DataRef add_default(const MapKeyType& key);
    DataRef add_default(MapKeyType&& key);
    DataRef add_zeroed(const MapKeyType& key);
    DataRef add_zeroed(MapKeyType&& key);
    template <typename Pred, typename ConstructFunc, typename AssignFunc>
    DataRef add_ex(HashType hash, Pred&& comparer, ConstructFunc&& construct, AssignFunc&& assign);
    template <typename Pred>
    DataRef add_ex_unsafe(HashType hash, Pred&& comparer);

    // add or assign, try to use this api, instead of operator[]
    // move behavior or key may not happened here, just for easy to use
    DataRef add_or_assign(const MapKeyType& key, const MapValueType& value);
    DataRef add_or_assign(const MapKeyType& key, MapValueType&& value);
    DataRef add_or_assign(MapKeyType&& key, const MapValueType& value);
    DataRef add_or_assign(MapKeyType&& key, MapValueType&& value);

    // emplace
    template <typename... Args>
    DataRef try_emplace(const MapKeyType& key, Args&&... args);
    template <typename... Args>
    DataRef try_emplace(MapKeyType&& key, Args&&... args);
    template <typename... Args>
    DataRef emplace(const MapKeyType& key, Args&&... args);
    template <typename... Args>
    DataRef emplace(MapKeyType&& key, Args&&... args);

    // append
    void append(const SparseHashMap& set);
    void append(std::initializer_list<MapDataType> init_list);
    void append(const MapDataType* p, SizeType n);

    // remove
    DataRef  remove(const MapKeyType& key);
    SizeType remove_all(const MapKeyType& key); // [multi map extend]
    template <typename Pred>
    DataRef remove_ex(HashType hash, Pred&& comparer);
    template <typename Pred>
    SizeType remove_all_ex(HashType hash, Pred&& comparer); // [multi map extend]

    // erase, needn't update iterator, erase directly is safe
    It  erase(const It& it);
    CIt erase(const CIt& it);

    // find
    DataRef  find(const MapKeyType& key);
    CDataRef find(const MapKeyType& key) const;
    template <typename Pred>
    DataRef find_ex(HashType hash, Pred&& comparer);
    template <typename Pred>
    CDataRef find_ex(HashType hash, Pred&& comparer) const;

    // contains
    bool     contains(const MapKeyType& key) const;
    SizeType count(const MapKeyType& key) const; // [multi map extend]
    template <typename Pred>
    bool contains_ex(HashType hash, Pred&& comparer) const;
    template <typename Pred>
    SizeType count_ex(HashType hash, Pred&& comparer) const; // [multi map extend]

    // sort
    template <typename TP = Less<MapKeyType>>
    void sort(TP&& p = {});
    template <typename TP = Less<MapKeyType>>
    void sort_stable(TP&& p = {});

    // set ops
    SparseHashMap operator&(const SparseHashMap& rhs) const;     // intersect
    SparseHashMap operator|(const SparseHashMap& rhs) const;     // union
    SparseHashMap operator^(const SparseHashMap& rhs) const;     // difference
    SparseHashMap operator-(const SparseHashMap& rhs) const;     // sub
    bool          is_sub_set_of(const SparseHashMap& rhs) const; // sub set

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;
};
} // namespace skr::container

// SparseHashMap impl
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
    : Super(reserve_size, std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(const MapDataType* p, SizeType n, AllocatorCtorParam param)
    : Super(p, n, std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(std::initializer_list<MapDataType> init_list, AllocatorCtorParam param)
    : Super(init_list, std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::~SparseHashMap()
{
}

// copy & move
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(const SparseHashMap& other)
    : Super(other)
{
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(SparseHashMap&& other)
    : Super(std::move(other))
{
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

// getter
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::size() const
{
    return Super::size();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::capacity() const
{
    return Super::capacity();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::slack() const
{
    return Super::slack();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::sparse_size() const
{
    return Super::sparse_size();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::hole_size() const
{
    return Super::hole_size();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::bit_array_size() const
{
    return Super::bit_array_size();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::free_list_head() const
{
    return Super::free_list_head();
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::is_compact() const
{
    return Super::is_compact();
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::empty() const
{
    return Super::empty();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataArr& SparseHashMap<Memory>::data_arr()
{
    return Super::data_arr();
}
template <typename Memory>
SKR_INLINE const typename SparseHashMap<Memory>::DataArr& SparseHashMap<Memory>::data_arr() const
{
    return Super::data_arr();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::Super& SparseHashMap<Memory>::data_set()
{
    return (*this);
}
template <typename Memory>
SKR_INLINE const typename SparseHashMap<Memory>::Super& SparseHashMap<Memory>::data_set() const
{
    return (*this);
}
template <typename Memory>
SKR_INLINE Memory& SparseHashMap<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& SparseHashMap<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::has_data(SizeType idx) const
{
    return Super::has_data(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::is_hole(SizeType idx) const
{
    return Super::is_hole(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::is_valid_index(SizeType idx) const
{
    return Super::is_valid_index(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::is_valid_pointer(const void* p) const
{
    return Super::is_valid_pointer(p);
}

// memory op
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::clear()
{
    Super::clear();
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::release(SizeType capacity)
{
    Super::release(capacity);
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::reserve(SizeType capacity)
{
    Super::reserve(capacity);
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::shrink()
{
    Super::shrink();
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::compact()
{
    return Super::compact();
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::compact_stable()
{
    return Super::compact_stable();
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::compact_top()
{
    return Super::compact_top();
}

// rehash
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::rehash() const
{
    Super::rehash();
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::rehash_if_need() const
{
    return Super::rehash_if_need();
}

// add
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(const MapKeyType& key, const MapValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        memory::copy(&ref.ref().value, &value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(const MapKeyType& key, MapValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        memory::move(&ref.ref().value, &value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(MapKeyType&& key, const MapValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        memory::copy(&ref.ref().value, &value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(MapKeyType&& key, MapValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        memory::move(&ref.ref().value, &value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(const MapKeyType& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        memory::construct_stl_ub(&ref.ref().value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(MapKeyType&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        memory::construct_stl_ub(&ref.ref().value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_unsafe(const MapKeyType& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_unsafe(MapKeyType&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_default(const MapKeyType& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        memory::construct(&ref.ref().value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_default(MapKeyType&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        memory::construct(&ref.ref().value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_zeroed(const MapKeyType& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        memset(&ref.ref().value, 0, sizeof(MapValueType));
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_zeroed(MapKeyType&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        memset(&ref.ref().value, 0, sizeof(MapValueType));
    }

    return ref;
}
template <typename Memory>
template <typename Pred, typename ConstructFunc, typename AssignFunc>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_ex(HashType hash, Pred&& comparer, ConstructFunc&& construct, AssignFunc&& assign)
{
    auto ref = Super::add_ex_unsafe(hash, std::forward<Pred>(comparer));

    if (ref.already_exist())
    {
        assign(ref.ptr());
    }
    else
    {
        construct(ref.ptr());
    }

    return ref;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_ex_unsafe(HashType hash, Pred&& comparer)
{
    auto ref = Super::add_ex_unsafe(hash, std::forward<Pred>(comparer));
    return ref;
}

// add or assign, instead of operator[]
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(const MapKeyType& key, const MapValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        memory::copy(&ref.ref().value, &value);
    }
    else
    {
        memory::assign(&ref.ref().value, &value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(const MapKeyType& key, MapValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        memory::move(&ref.ref().value, &value);
    }
    else
    {
        memory::move_assign(&ref.ref().value, &value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(MapKeyType&& key, const MapValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        memory::copy(&ref.ref().value, &value);
    }
    else
    {
        memory::assign(&ref.ref().value, &value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(MapKeyType&& key, MapValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        memory::move(&ref.ref().value, &value);
    }
    else
    {
        memory::move_assign(&ref.ref().value, &value);
    }

    return ref;
}

// emplace

template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_emplace(const MapKeyType& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        new (&ref.ref().value) MapValueType(std::forward<Args>(args)...);
    }

    return ref;
}
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::try_emplace(MapKeyType&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        new (&ref.ref().value) MapValueType(std::forward<Args>(args)...);
    }

    return ref;
}
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::emplace(const MapKeyType& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::copy(&ref.ref().key, &key);
        new (&ref.ref().value) MapValueType(std::forward<Args>(args)...);
    }
    else
    {
        memory::assign(&ref.ref().key, &key);
        MapValueType v(std::forward<Args>(args)...);
        memory::move_assign(&ref.ref().value, &v);
    }

    return ref;
}
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::emplace(MapKeyType&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const MapKeyType& k) { return k == key; });

    if (!ref.already_exist())
    {
        memory::move(&ref.ref().key, &key);
        new (&ref.ref().value) MapValueType(std::forward<Args>(args)...);
    }
    else
    {
        memory::move_assign(&ref.ref().key, &key);
        MapValueType v(std::forward<Args>(args)...);
        memory::move_assign(&ref.ref().value, &v);
    }

    return ref;
}

// append
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(const SparseHashMap& set)
{
    Super::append(set);
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(std::initializer_list<MapDataType> init_list)
{
    Super::append(init_list);
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(const MapDataType* p, SizeType n)
{
    Super::append(p, n);
}

// remove
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::remove(const MapKeyType& key)
{
    return Super::remove(key);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::remove_all(const MapKeyType& key)
{
    return Super::remove_all(key);
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::remove_ex(HashType hash, Pred&& comparer)
{
    return Super::remove_ex(hash, std::forward<Pred>(comparer));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::remove_all_ex(HashType hash, Pred&& comparer)
{
    return Super::remove_all_ex(hash, std::forward<Pred>(comparer));
}

// erase, needn't update iterator, erase directly is safe
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::It SparseHashMap<Memory>::erase(const It& it)
{
    return Super::erase(it);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CIt SparseHashMap<Memory>::erase(const CIt& it)
{
    return Super::erase(it);
}

// find
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find(const MapKeyType& key)
{
    return Super::find(key);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find(const MapKeyType& key) const
{
    return Super::find(key);
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_ex(HashType hash, Pred&& comparer)
{
    return Super::find_ex(hash, std::forward<Pred>(comparer));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_ex(HashType hash, Pred&& comparer) const
{
    return Super::find_ex(hash, std::forward<Pred>(comparer));
}

// contains
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::contains(const MapKeyType& key) const
{
    return Super::contains(key);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::count(const MapKeyType& key) const
{
    return Super::count(key);
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseHashMap<Memory>::contains_ex(HashType hash, Pred&& comparer) const
{
    return Super::contains_ex(hash, std::forward<Pred>(comparer));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::count_ex(HashType hash, Pred&& comparer) const
{
    return Super::count_ex(hash, std::forward<Pred>(comparer));
}

// sort
template <typename Memory>
template <typename TP>
SKR_INLINE void SparseHashMap<Memory>::sort(TP&& p)
{
    Super::sort(std::forward<TP>(p));
}
template <typename Memory>
template <typename TP>
SKR_INLINE void SparseHashMap<Memory>::sort_stable(TP&& p)
{
    Super::sort_stable(std::forward<TP>(p));
}

// set ops
template <typename Memory>
SKR_INLINE SparseHashMap<Memory> SparseHashMap<Memory>::operator&(const SparseHashMap& rhs) const
{
    return Super::operator&(rhs);
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory> SparseHashMap<Memory>::operator|(const SparseHashMap& rhs) const
{
    return Super::operator|(rhs);
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory> SparseHashMap<Memory>::operator^(const SparseHashMap& rhs) const
{
    return Super::operator^(rhs);
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory> SparseHashMap<Memory>::operator-(const SparseHashMap& rhs) const
{
    return Super::operator-(rhs);
}
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::is_sub_set_of(const SparseHashMap& rhs) const
{
    return Super::is_sub_set_of(rhs);
}

// support foreach
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::It SparseHashMap<Memory>::begin()
{
    return Super::begin();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::It SparseHashMap<Memory>::end()
{
    return Super::end();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CIt SparseHashMap<Memory>::begin() const
{
    return Super::begin();
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CIt SparseHashMap<Memory>::end() const
{
    return Super::end();
}
} // namespace skr::container