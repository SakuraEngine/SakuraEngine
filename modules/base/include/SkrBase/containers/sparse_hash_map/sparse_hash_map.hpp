#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"

// SparseHashSet config
namespace skr
{

} // namespace skr

// SparseHashMap def
// TODO. remove/find/contain/count value
namespace skr
{
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
struct SparseHashMap : private SparseHashSet<KVPair<K, V>, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> {
    using Base = SparseHashSet<KVPair<K, V>, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>;

    // basic
    using SizeType     = typename Alloc::SizeType;
    using DataType     = KVPair<K, V>;
    using BaseDataType = typename Base::DataType;
    using DataArr      = typename Base::DataArr;

    // from base
    using HashType      = typename Base::HashType;
    using KeyType       = typename Base::KeyType;
    using keyMapperType = typename Base::keyMapperType;
    using HasherType    = typename Base::HasherType;
    using ComparerType  = typename Base::ComparerType;

    // data ref & iterator
    using DataRef  = typename Base::DataRef;
    using CDataRef = typename Base::CDataRef;
    using It       = typename Base::It;
    using CIt      = typename Base::CIt;

    // ctor & dtor
    SparseHashMap(Alloc alloc = {});
    SparseHashMap(SizeType reserve_size, Alloc alloc = {});
    SparseHashMap(const DataType* p, SizeType n, Alloc alloc = {});
    SparseHashMap(std::initializer_list<DataType> init_list, Alloc alloc = {});
    ~SparseHashMap();

    // copy & move
    SparseHashMap(const SparseHashMap& other, Alloc alloc = {});
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
    Base&          data_set();
    const Base&    data_set() const;
    Alloc&         allocator();
    const Alloc&   allocator() const;

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

    // add
    DataRef add(const K& key, const V& value);
    DataRef add(const K& key, V&& value);
    DataRef add(K&& key, const V& value);
    DataRef add(K&& key, V&& value);
    DataRef add_unsafe(const K& key);
    DataRef add_unsafe(K&& key);
    DataRef add_default(const K& key);
    DataRef add_default(K&& key);
    DataRef add_zeroed(const K& key);
    DataRef add_zeroed(K&& key);
    template <typename Comparer, typename Constructor>
    DataRef add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor);
    template <typename Comparer>
    DataRef add_ex_unsafe(HashType hash, Comparer&& comparer);

    // emplace
    template <typename... Args>
    DataRef emplace(const K& key, Args&&... args);
    template <typename... Args>
    DataRef emplace(K&& key, Args&&... args);

    // append
    void append(const SparseHashMap& set);
    void append(std::initializer_list<DataType> init_list);
    void append(const DataType* p, SizeType n);

    // remove
    DataRef  remove(const K& key);
    SizeType remove_all(const K& key); // [multi map extend]
    template <typename Comparer>
    DataRef remove_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    SizeType remove_all_ex(HashType hash, Comparer&& comparer); // [multi map extend]

    // find
    DataRef  find(const K& key);
    CDataRef find(const K& key) const;
    template <typename Comparer>
    DataRef find_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    CDataRef find_ex(HashType hash, Comparer&& comparer) const;

    // contain
    bool     contain(const K& key) const;
    SizeType count(const K& key) const; // [multi map extend]
    template <typename Comparer>
    bool contain_ex(HashType hash, Comparer&& comparer) const;
    template <typename Comparer>
    SizeType count_ex(HashType hash, Comparer&& comparer) const; // [multi map extend]

    // sort
    template <typename TP = Less<K>>
    void sort(TP&& p = {});
    template <typename TP = Less<K>>
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
} // namespace skr

// SparseHashMap impl
namespace skr
{
// ctor & dtor
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashMap(Alloc alloc)
    : Base(std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashMap(SizeType reserve_size, Alloc alloc)
    : Base(reserve_size, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashMap(const DataType* p, SizeType n, Alloc alloc)
    : Base(p, n, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashMap(std::initializer_list<DataType> init_list, Alloc alloc)
    : Base(init_list, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::~SparseHashMap()
{
}

// copy & move
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashMap(const SparseHashMap& other, Alloc alloc)
    : Base(other, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashMap(SparseHashMap&& other)
    : Base(std::move(other))
{
}

// assign & move assign
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator=(const SparseHashMap& rhs)
{
    Base::operator=(rhs);
    return *this;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator=(SparseHashMap&& rhs)
{
    Base::operator=(std::move(rhs));
    return *this;
}

// getter
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::size() const
{
    return Base::size();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::capacity() const
{
    return Base::capacity();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::slack() const
{
    return Base::slack();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::sparse_size() const
{
    return Base::sparse_size();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::hole_size() const
{
    return Base::hole_size();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::bit_array_size() const
{
    return Base::bit_array_size();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::free_list_head() const
{
    return Base::free_list_head();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::bucket_size() const
{
    return Base::bucket_size();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_compact() const
{
    return Base::is_compact();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::empty() const
{
    return Base::empty();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataArr& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::data_arr()
{
    return Base::data_arr();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE const typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataArr& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::data_arr() const
{
    return Base::data_arr();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::Base& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::data_set()
{
    return (*this);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE const typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::Base& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::data_set() const
{
    return (*this);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE Alloc& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::allocator()
{
    return Base::allocator();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE const Alloc& SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::allocator() const
{
    return Base::allocator();
}

// validate
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::has_data(SizeType idx) const
{
    return Base::has_data(idx);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_hole(SizeType idx) const
{
    return Base::is_hole(idx);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_valid_index(SizeType idx) const
{
    return Base::is_valid_index(idx);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_valid_pointer(const void* p) const
{
    return Base::is_valid_pointer(p);
}

// memory op
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::clear()
{
    Base::clear();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::release(SizeType capacity)
{
    Base::release(capacity);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::reserve(SizeType capacity)
{
    Base::reserve(capacity);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::shrink()
{
    Base::shrink();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::compact()
{
    return Base::compact();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::compact_stable()
{
    return Base::compact_stable();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::compact_top()
{
    return Base::compact_top();
}

// rehash
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::need_rehash() const
{
    return Base::need_rehash();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::rehash() const
{
    Base::rehash();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::rehash_if_need() const
{
    return Base::rehash_if_need();
}

// add
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add(const K& key, const V& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(key);
        new (&ref->value) V(value);
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add(const K& key, V&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(key);
        new (&ref->value) V(std::move(value));
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add(K&& key, const V& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(std::move(key));
        new (&ref->value) V(value);
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add(K&& key, V&& value)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(std::move(key));
        new (&ref->value) V(std::move(value));
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_unsafe(const K& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(std::move(key));
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_unsafe(K&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(std::move(key));
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_default(const K& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(key);
        new (&ref->value) V();
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_default(K&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(std::move(key));
        new (&ref->value) V();
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_zeroed(const K& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(key);
        memset(&ref->value, 0, sizeof(V));
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_zeroed(K&& key)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(std::move(key));
        memset(&ref->value, 0, sizeof(V));
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer, typename Constructor>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor)
{
    auto ref = Base::add_ex_unsafe(hash, std::forward<Comparer>(comparer));

    if (!ref.already_exist)
    {
        constructor(ref.data);
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_ex_unsafe(HashType hash, Comparer&& comparer)
{
    auto ref = Base::add_ex_unsafe(hash, std::forward<Comparer>(comparer));
    return ref;
}

// emplace
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::emplace(const K& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(key);
        new (&ref->value) V(std::forward<Args>(args)...);
    }

    return ref;
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::emplace(K&& key, Args&&... args)
{
    HashType hash = HasherType()(key);
    auto     ref  = Base::add_ex_unsafe(
    hash,
    [&key](const K& k) { return ComparerType()(k, key); });

    if (!ref.already_exist)
    {
        new (&ref->key) K(std::move(key));
        new (&ref->value) V(std::forward<Args>(args)...);
    }

    return ref;
}

// append
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::append(const SparseHashMap& set)
{
    Base::append(set);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::append(std::initializer_list<DataType> init_list)
{
    Base::append(init_list);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::append(const DataType* p, SizeType n)
{
    Base::append(p, n);
}

// remove
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove(const K& key)
{
    return Base::remove(key);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove_all(const K& key)
{
    return Base::remove_all(key);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove_ex(HashType hash, Comparer&& comparer)
{
    return Base::remove_ex(hash, std::forward<Comparer>(comparer));
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove_all_ex(HashType hash, Comparer&& comparer)
{
    return Base::remove_all_ex(hash, std::forward<Comparer>(comparer));
}

// find
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find(const K& key)
{
    return Base::find(key);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CDataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find(const K& key) const
{
    return Base::find(key);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find_ex(HashType hash, Comparer&& comparer)
{
    return Base::find_ex(hash, std::forward<Comparer>(comparer));
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CDataRef SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find_ex(HashType hash, Comparer&& comparer) const
{
    return Base::find_ex(hash, std::forward<Comparer>(comparer));
}

// contain
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::contain(const K& key) const
{
    return Base::contain(key);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::count(const K& key) const
{
    return Base::count(key);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::contain_ex(HashType hash, Comparer&& comparer) const
{
    return Base::contain_ex(hash, std::forward<Comparer>(comparer));
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::count_ex(HashType hash, Comparer&& comparer) const
{
    return Base::count_ex(hash, std::forward<Comparer>(comparer));
}

// sort
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::sort(TP&& p)
{
    Base::sort(std::forward<TP>(p));
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::sort_stable(TP&& p)
{
    Base::sort_stable(std::forward<TP>(p));
}

// set ops
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator&(const SparseHashMap& rhs) const
{
    return Base::operator&(rhs);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator|(const SparseHashMap& rhs) const
{
    return Base::operator|(rhs);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator^(const SparseHashMap& rhs) const
{
    return Base::operator^(rhs);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator-(const SparseHashMap& rhs) const
{
    return Base::operator-(rhs);
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_sub_set_of(const SparseHashMap& rhs) const
{
    return Base::is_sub_set_of(rhs);
}

// support foreach
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::It SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::begin()
{
    return Base::begin();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::It SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::end()
{
    return Base::end();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CIt SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::begin() const
{
    return Base::begin();
}
template <typename K, typename V, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CIt SparseHashMap<K, V, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::end() const
{
    return Base::end();
}
} // namespace skr