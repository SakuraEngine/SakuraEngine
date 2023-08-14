#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include "SkrBase/tools/hash.hpp"
#include "SkrBase/containers/sparse_hash_map/kvpair.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map_def.hpp"

// SparseHashSet config
namespace skr
{
template <typename K, typename V>
struct MapKey {
    SKR_INLINE constexpr K&       operator()(KVPair<K, V>& pair) const { return pair.key; }
    SKR_INLINE constexpr const K& operator()(const KVPair<K, V>& pair) const { return std::move(pair.value); }
};

template <typename K, typename V, bool MultiKey>
struct SparseHashMapConfigDefault {
    using KeyType       = K;
    using KeyMapperType = MapKey<K, V>;
    using HashType      = size_t;
    using HasherType    = Hash<HashType>;
    using ComparerType  = Equal<KeyType>;

    static constexpr bool multi_key = MultiKey;
};
} // namespace skr

// SparseHashMap def
// TODO. remove/find/contain/count value
namespace skr
{
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
struct SparseHashMap : private SparseHashSet<KVPair<K, V>, TBitBlock, Config, Alloc> {
    using Base = SparseHashSet<KVPair<K, V>, TBitBlock, Config, Alloc>;

    // basic
    using SizeType     = typename Alloc::SizeType;
    using DataType     = KVPair<K, V>;
    using BaseDataType = typename Base::DataType;
    using DataArr      = typename Base::DataArr;

    // from base
    using HashType      = typename Base::HashType;
    using KeyType       = typename Base::KeyType;
    using keyMapperType = typename Base::KeyMapperType;
    using HasherType    = typename Base::HasherType;
    using ComparerType  = typename Base::ComparerType;

    // data ref & iterator
    using DataRef  = SparseHashMapDataRef<K, V, SizeType>;
    using CDataRef = SparseHashMapDataRef<const K, const V, SizeType>;
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
    DataRef add_default_unsafe(const K& key);
    template <typename Comparer, typename Constructor>
    DataRef add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor);
    template <typename Comparer>
    DataRef add_ex_unsafe(HashType hash, Comparer&& comparer);

    // emplace
    template <typename... Args>
    DataRef emplace(const K& key, Args&&... args);
    template <typename... Args>
    DataRef emplace(K&& key, Args&&... args);
    template <typename Comparer, typename... Args>
    DataRef emplace_ex(HashType hash, Comparer&& comparer, Args&&... args);

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
    bool     contains(const K& key) const;
    SizeType count(const K& key) const; // [multi map extend]
    template <typename Comparer>
    bool contains_ex(HashType hash, Comparer&& comparer) const;
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
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>::SparseHashMap(Alloc alloc)
    : Base(std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>::SparseHashMap(SizeType reserve_size, Alloc alloc)
    : Base(reserve_size, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>::SparseHashMap(const DataType* p, SizeType n, Alloc alloc)
    : Base(p, n, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>::SparseHashMap(std::initializer_list<DataType> init_list, Alloc alloc)
    : Base(init_list, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>::~SparseHashMap()
{
}

// copy & move
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>::SparseHashMap(const SparseHashMap& other, Alloc alloc)
    : Base(other, std::move(alloc))
{
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>::SparseHashMap(SparseHashMap&& other)
    : Base(std::move(other))
{
}

// assign & move assign
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>& SparseHashMap<K, V, TBitBlock, Config, Alloc>::operator=(const SparseHashMap& rhs)
{
    Base::operator=(rhs);
    return *this;
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashMap<K, V, TBitBlock, Config, Alloc>& SparseHashMap<K, V, TBitBlock, Config, Alloc>::operator=(SparseHashMap&& rhs)
{
    Base::operator=(std::move(rhs));
    return *this;
}

// getter
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::size() const
{
    return Base::size();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::capacity() const
{
    return Base::capacity();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::slack() const
{
    return Base::slack();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::sparse_size() const
{
    return Base::sparse_size();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::hole_size() const
{
    return Base::hole_size();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::bit_array_size() const
{
    return Base::bit_array_size();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::free_list_head() const
{
    return Base::free_list_head();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::SizeType SparseHashMap<K, V, TBitBlock, Config, Alloc>::bucket_size() const
{
    return Base::bucket_size();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, Config, Alloc>::is_compact() const
{
    return Base::is_compact();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, Config, Alloc>::empty() const
{
    return Base::empty();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::DataArr& SparseHashMap<K, V, TBitBlock, Config, Alloc>::data_arr()
{
    return Base::data_arr();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE const typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::DataArr& SparseHashMap<K, V, TBitBlock, Config, Alloc>::data_arr() const
{
    return Base::data_arr();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::Base& SparseHashMap<K, V, TBitBlock, Config, Alloc>::data_set()
{
    return (*this);
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE const typename SparseHashMap<K, V, TBitBlock, Config, Alloc>::Base& SparseHashMap<K, V, TBitBlock, Config, Alloc>::data_set() const
{
    return (*this);
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE Alloc& SparseHashMap<K, V, TBitBlock, Config, Alloc>::allocator()
{
    return Base::allocator();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE const Alloc& SparseHashMap<K, V, TBitBlock, Config, Alloc>::allocator() const
{
    return Base::allocator();
}

// memory op
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, Config, Alloc>::clear()
{
    Base::clear();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, Config, Alloc>::release(SizeType capacity)
{
    Base::release(capacity);
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, Config, Alloc>::reserve(SizeType capacity)
{
    Base::reserve(capacity);
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, Config, Alloc>::shrink()
{
    Base::shrink();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, Config, Alloc>::compact()
{
    return Base::compact();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, Config, Alloc>::compact_stable()
{
    return Base::compact_stable();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, Config, Alloc>::compact_top()
{
    return Base::compact_top();
}

// rehash
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, Config, Alloc>::need_rehash() const
{
    return Base::need_rehash();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashMap<K, V, TBitBlock, Config, Alloc>::rehash() const
{
    Base::rehash();
}
template <typename K, typename V, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashMap<K, V, TBitBlock, Config, Alloc>::rehash_if_need() const
{
    return Base::rehash_if_need();
}

} // namespace skr