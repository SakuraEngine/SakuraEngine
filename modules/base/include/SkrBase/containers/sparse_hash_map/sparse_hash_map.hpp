#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"

// SparseHashMap def
// TODO. remove/find/contain/count value
namespace skr::container
{
template <typename Memory>
struct SparseHashMap : protected SparseHashSet<Memory> {
    using Super = SparseHashSet<Memory>;

    // configure
    using typename Memory::KeyType;
    using typename Memory::ValueType;
    using SizeType = typename Memory::SizeType;
    using typename Memory::AllocatorCtorParam;
    using SparseHashMapStorageType = KVPair<KeyType, ValueType>;
    using DataArr                  = typename Super::DataArr;

    // from base
    using HashType     = typename Super::HashType;
    using HasherType   = typename Super::HasherType;
    using ComparerType = typename Super::ComparerType;

    // data ref & iterator
    using DataRef  = typename Super::DataRef;
    using CDataRef = typename Super::CDataRef;
    using It       = typename Super::It;
    using CIt      = typename Super::CIt;

    // ctor & dtor
    SparseHashMap(AllocatorCtorParam param = {});
    SparseHashMap(SizeType reserve_size, AllocatorCtorParam param = {});
    SparseHashMap(const SparseHashMapStorageType* p, SizeType n, AllocatorCtorParam param = {});
    SparseHashMap(std::initializer_list<SparseHashMapStorageType> init_list, AllocatorCtorParam param = {});
    ~SparseHashMap();

    // copy & move
    SparseHashMap(const SparseHashMap& other, AllocatorCtorParam param = {});
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
    SizeType       bucket_size() const;
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
    bool need_rehash() const;
    void rehash() const;
    bool rehash_if_need() const;

    // add, move behavior may not happened here, just for easy to use
    DataRef add(const KeyType& key, const ValueType& value);
    DataRef add(const KeyType& key, ValueType&& value);
    DataRef add(KeyType&& key, const ValueType& value);
    DataRef add(KeyType&& key, ValueType&& value);
    DataRef add_unsafe(const KeyType& key);
    DataRef add_unsafe(KeyType&& key);
    DataRef add_default(const KeyType& key);
    DataRef add_default(KeyType&& key);
    DataRef add_zeroed(const KeyType& key);
    DataRef add_zeroed(KeyType&& key);
    template <typename Comparer, typename Constructor>
    DataRef add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor);
    template <typename Comparer>
    DataRef add_ex_unsafe(HashType hash, Comparer&& comparer);

    // add or assign, try to use this api, instead of operator[]
    // move behavior or key may not happened here, just for easy to use
    DataRef add_or_assign(const KeyType& key, const ValueType& value);
    DataRef add_or_assign(const KeyType& key, ValueType&& value);
    DataRef add_or_assign(KeyType&& key, const ValueType& value);
    DataRef add_or_assign(KeyType&& key, ValueType&& value);

    // emplace
    template <typename... Args>
    DataRef emplace(const KeyType& key, Args&&... args);
    template <typename... Args>
    DataRef emplace(KeyType&& key, Args&&... args);

    // append
    void append(const SparseHashMap& set);
    void append(std::initializer_list<SparseHashMapStorageType> init_list);
    void append(const SparseHashMapStorageType* p, SizeType n);

    // remove
    DataRef  remove(const KeyType& key);
    SizeType remove_all(const KeyType& key); // [multi map extend]
    template <typename Comparer>
    DataRef remove_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    SizeType remove_all_ex(HashType hash, Comparer&& comparer); // [multi map extend]

    // erase, needn't update iterator, erase directly is safe
    void erase(const It& it);
    void erase(const CIt& it);

    // find
    DataRef  find(const KeyType& key);
    CDataRef find(const KeyType& key) const;
    template <typename Comparer>
    DataRef find_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    CDataRef find_ex(HashType hash, Comparer&& comparer) const;

    // contain
    bool     contain(const KeyType& key) const;
    SizeType count(const KeyType& key) const; // [multi map extend]
    template <typename Comparer>
    bool contain_ex(HashType hash, Comparer&& comparer) const;
    template <typename Comparer>
    SizeType count_ex(HashType hash, Comparer&& comparer) const; // [multi map extend]

    // sort
    template <typename TP = Less<KeyType>>
    void sort(TP&& p = {});
    template <typename TP = Less<KeyType>>
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
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(const SparseHashMapStorageType* p, SizeType n, AllocatorCtorParam param)
    : Super(p, n, std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(std::initializer_list<SparseHashMapStorageType> init_list, AllocatorCtorParam param)
    : Super(init_list, std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::~SparseHashMap()
{
}

// copy & move
template <typename Memory>
SKR_INLINE SparseHashMap<Memory>::SparseHashMap(const SparseHashMap& other, AllocatorCtorParam param)
    : Super(other, std::move(param))
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
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::bucket_size() const
{
    return Super::bucket_size();
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
SKR_INLINE bool SparseHashMap<Memory>::need_rehash() const
{
    return Super::need_rehash();
}
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
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(const KeyType& key, const ValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(key);
        new (&ref->value) ValueType(value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(const KeyType& key, ValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(key);
        new (&ref->value) ValueType(std::move(value));
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(KeyType&& key, const ValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
        new (&ref->value) ValueType(value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add(KeyType&& key, ValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
        new (&ref->value) ValueType(std::move(value));
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_unsafe(const KeyType& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_unsafe(KeyType&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_default(const KeyType& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(key);
        new (&ref->value) ValueType();
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_default(KeyType&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
        new (&ref->value) ValueType();
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_zeroed(const KeyType& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(key);
        memset(&ref->value, 0, sizeof(ValueType));
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_zeroed(KeyType&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
        memset(&ref->value, 0, sizeof(ValueType));
    }

    return ref;
}
template <typename Memory>
template <typename Comparer, typename Constructor>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor)
{
    auto ref = Super::add_ex_unsafe(hash, std::forward<Comparer>(comparer));

    if (!ref.already_exist)
    {
        constructor(ref.data);
    }

    return ref;
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_ex_unsafe(HashType hash, Comparer&& comparer)
{
    auto ref = Super::add_ex_unsafe(hash, std::forward<Comparer>(comparer));
    return ref;
}

// add or assign, instead of operator[]
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(const KeyType& key, const ValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(key);
        new (&ref->value) ValueType(value);
    }
    else
    {
        ref->value = value;
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(const KeyType& key, ValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(key);
        new (&ref->value) ValueType(std::move(value));
    }
    else
    {
        ref->value = std::move(value);
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(KeyType&& key, const ValueType& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
        new (&ref->value) ValueType(value);
    }
    else
    {
        ref->value = value;
    }

    return ref;
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::add_or_assign(KeyType&& key, ValueType&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
        new (&ref->value) ValueType(std::move(value));
    }
    else
    {
        ref->value = std::move(value);
    }

    return ref;
}

// emplace
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::emplace(const KeyType& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(key);
        new (&ref->value) ValueType(std::forward<Args>(args)...);
    }

    return ref;
}
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::emplace(KeyType&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Super::add_ex_unsafe(
    hash,
    [&key](const KeyType& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) KeyType(std::move(key));
        new (&ref->value) ValueType(std::forward<Args>(args)...);
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
SKR_INLINE void SparseHashMap<Memory>::append(std::initializer_list<SparseHashMapStorageType> init_list)
{
    Super::append(init_list);
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::append(const SparseHashMapStorageType* p, SizeType n)
{
    Super::append(p, n);
}

// remove
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::remove(const KeyType& key)
{
    return Super::remove(key);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::remove_all(const KeyType& key)
{
    return Super::remove_all(key);
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::remove_ex(HashType hash, Comparer&& comparer)
{
    return Super::remove_ex(hash, std::forward<Comparer>(comparer));
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::remove_all_ex(HashType hash, Comparer&& comparer)
{
    return Super::remove_all_ex(hash, std::forward<Comparer>(comparer));
}

// erase, needn't update iterator, erase directly is safe
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::erase(const It& it)
{
    Super::erase(it);
}
template <typename Memory>
SKR_INLINE void SparseHashMap<Memory>::erase(const CIt& it)
{
    Super::erase(it);
}

// find
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find(const KeyType& key)
{
    return Super::find(key);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find(const KeyType& key) const
{
    return Super::find(key);
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<Memory>::DataRef SparseHashMap<Memory>::find_ex(HashType hash, Comparer&& comparer)
{
    return Super::find_ex(hash, std::forward<Comparer>(comparer));
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<Memory>::CDataRef SparseHashMap<Memory>::find_ex(HashType hash, Comparer&& comparer) const
{
    return Super::find_ex(hash, std::forward<Comparer>(comparer));
}

// contain
template <typename Memory>
SKR_INLINE bool SparseHashMap<Memory>::contain(const KeyType& key) const
{
    return Super::contain(key);
}
template <typename Memory>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::count(const KeyType& key) const
{
    return Super::count(key);
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE bool SparseHashMap<Memory>::contain_ex(HashType hash, Comparer&& comparer) const
{
    return Super::contain_ex(hash, std::forward<Comparer>(comparer));
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<Memory>::SizeType SparseHashMap<Memory>::count_ex(HashType hash, Comparer&& comparer) const
{
    return Super::count_ex(hash, std::forward<Comparer>(comparer));
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