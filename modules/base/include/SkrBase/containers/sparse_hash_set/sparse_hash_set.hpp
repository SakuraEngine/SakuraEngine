#pragma once
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"
#include "sparse_hash_set_iterator.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/tools/hash.hpp"

// SparseHashSet config
namespace skr
{
template <typename T, bool MultiKey>
struct SparseHashSetConfigDefault {
    using KeyType       = T;
    using KeyMapperType = MapFwd<T>;
    using HashType      = size_t;
    using HasherType    = Hash<KeyType>;
    using ComparerType  = Equal<KeyType>;

    static constexpr bool multi_key = MultiKey;
};
} // namespace skr

// SparseHashSet def
// TODO. bucket 与碰撞统计，以及更好的 bucket 分配策略
// TODO. find/contain/add/emplace/remove as 的操作重新统一
//      主要问题在于 emplace_as 无法给出一个明确的 as 值，as 操作本质上是一种糖，find 本质上只需要提供一个 hash 与 compare 即可
//      那么是否可以将 API 重新细分为 -/as/ex 的组织，as 作为 ex 的形式存在，而考虑到使用频次，hashed 可以通过 ex 的形式代替，这样所有 API 最后都会归纳到 xxx_ex
namespace skr
{
template <typename T, typename TBitBlock, typename Config, typename Alloc>
struct SparseHashSet {
    // from config
    using HashType      = typename Config::HashType;
    using KeyType       = typename Config::KeyType;
    using keyMapperType = typename Config::KeyMapperType;
    using HasherType    = typename Config::HasherType;
    using ComparerType  = typename Config::ComparerType;

    // basic
    using SizeType                        = typename Alloc::SizeType;
    using DataType                        = SparseHashSetData<T, SizeType, HashType>;
    using DataArr                         = SparseArray<DataType, TBitBlock, Alloc>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref & iterator
    using DataRef  = SparseHashSetDataRef<T, SizeType>;
    using CDataRef = SparseHashSetDataRef<const T, SizeType>;
    using It       = SparseHashSetIt<T, TBitBlock, SizeType, HashType, false>;
    using CIt      = SparseHashSetIt<T, TBitBlock, SizeType, HashType, true>;

public:
    // ctor & dtor
    SparseHashSet(Alloc alloc = Alloc());
    SparseHashSet(SizeType reserve_size, Alloc alloc = Alloc());
    SparseHashSet(const T* p, SizeType n, Alloc alloc = Alloc());
    SparseHashSet(std::initializer_list<T> init_list, Alloc alloc = Alloc());
    ~SparseHashSet();

    // copy & move
    SparseHashSet(const SparseHashSet& other, Alloc alloc = Alloc());
    SparseHashSet(SparseHashSet&& other);

    // assign & move assign
    SparseHashSet& operator=(const SparseHashSet& rhs);
    SparseHashSet& operator=(SparseHashSet&& rhs);

    // compare
    bool operator==(const SparseHashSet& rhs) const;
    bool operator!=(const SparseHashSet& rhs) const;

    // getter
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    SizeType        sparse_size() const;
    SizeType        hole_size() const;
    SizeType        bit_array_size() const;
    SizeType        free_list_head() const;
    SizeType        bucket_size() const;
    SizeType        bucket_mask() const;
    bool            is_compact() const;
    bool            empty() const;
    DataArr&        data_arr();
    const DataArr&  data_arr() const;
    SizeType*       bucket();
    const SizeType* bucket() const;
    Alloc&          allocator();
    const Alloc&    allocator() const;

    // validate
    bool has_data(SizeType idx) const;
    bool is_hole(SizeType idx) const;
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const T* p) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    bool compact();
    bool compact_stable();
    bool compact_top();

    // data op
    KeyType&       key_of(T& v) const;
    const KeyType& key_of(const T& v) const;
    bool           key_equal(const T& a, const T& b) const;
    HashType       hash_of(const T& v) const;

    // rehash
    bool need_rehash() const;
    void rehash() const;
    bool rehash_if_need() const;

    // add
    // check existence then add
    DataRef add(const T& v);
    DataRef add(T&& v);
    template <typename Comparer, typename Constructor>
    DataRef add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor);
    template <typename Comparer>
    DataRef add_unsafe(HashType hash, Comparer&& comparer);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename Comparer, typename... Args>
    DataRef emplace_ex(HashType hash, Comparer&& comparer, Args&&... args);

    // append
    void append(const SparseHashSet& set);
    void append(std::initializer_list<T> init_list);
    void append(const T* p, SizeType n);

    // remove
    DataRef  remove(const KeyType& key);
    SizeType remove_all(const KeyType& key); // [multi set extend]
    template <typename Comparer>
    DataRef remove_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    SizeType remove_all_ex(HashType hash, Comparer&& comparer); // [multi set extend]

    // find
    DataRef  find(const KeyType& key);
    CDataRef find(const KeyType& key) const;
    template <typename Comparer>
    DataRef find_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    CDataRef find_ex(HashType hash, Comparer&& comparer) const;

    // contain
    bool     contain(const KeyType& key) const;
    SizeType count(const KeyType& key) const; // [multi set extend]
    template <typename Comparer>
    bool contain_ex(HashType hash, Comparer&& comparer) const;
    template <typename Comparer>
    SizeType count_ex(HashType hash, Comparer&& comparer) const; // [multi set extend]

    // sort
    template <typename TP = Less<T>>
    void sort(TP&& p = TP());
    template <typename TP = Less<T>>
    void sort_stable(TP&& p = TP());

    // set ops
    SparseHashSet operator&(const SparseHashSet& rhs) const;     // intersect
    SparseHashSet operator|(const SparseHashSet& rhs) const;     // union
    SparseHashSet operator^(const SparseHashSet& rhs) const;     // difference
    SparseHashSet operator-(const SparseHashSet& rhs) const;     // sub
    bool          is_sub_set_of(const SparseHashSet& rhs) const; // sub set

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;

private:
    // helpers
    SizeType _calc_bucket_size(SizeType data_size) const;
    SizeType _bucket_index(SizeType hash) const; // get bucket data index by hash
    void     _clean_bucket() const;              // remove all elements from bucket
    bool     _resize_bucket() const;             // resize hash bucket
    bool     _is_in_bucket(SizeType index) const;
    void     _add_to_bucket(SizeType index);
    void     _remove_from_bucket(SizeType index);

private:
    mutable SizeType* _bucket      = nullptr;
    mutable SizeType  _bucket_size = 0;
    mutable SizeType  _bucket_mask = 0;
    DataArr           _data;
};
} // namespace skr

// SparseHashSet impl
namespace skr
{
// helpers
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::_calc_bucket_size(SizeType data_size) const
{
    static constexpr SizeType min_size_to_hash    = 4;
    static constexpr SizeType basic_bucket_size   = 8;
    static constexpr SizeType avg_bucket_capacity = 2;

    if (data_size >= min_size_to_hash)
    {
        return bit_ceil(SizeType(data_size / avg_bucket_capacity) + basic_bucket_size);
    }
    else if (data_size)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::_bucket_index(SizeType hash) const
{
    return hash & _bucket_mask;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::_clean_bucket() const
{
    if (_bucket)
    {
        SizeType* begin = _bucket;
        SizeType* end   = _bucket + _bucket_size;
        for (; begin != end; ++begin)
        {
            *begin = npos;
        }
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::_resize_bucket() const // resize bucket (nocopy)
{
    SizeType new_bucket_size = _calc_bucket_size(_data.capacity());

    if (new_bucket_size != _bucket_size)
    {
        if (new_bucket_size) // has new size
        {
            _bucket      = _data.allocator().resize_container(_bucket, _bucket_size, _bucket_size, new_bucket_size);
            _bucket_size = new_bucket_size;
            _bucket_mask = new_bucket_size - 1;
        }
        else // new size is 0
        {
            if (_bucket)
            {
                _data.allocator().free(_bucket);
                _bucket      = nullptr;
                _bucket_size = 0;
                _bucket_mask = 0;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::_is_in_bucket(SizeType index) const
{
    if (has_data(index))
    {
        const auto& node         = _data[index];
        auto        search_index = _bucket[_bucket_index(node.hash)];

        while (search_index != npos)
        {
            if (search_index == index)
            {
                return true;
            }
            search_index = _data[search_index].next;
        }
        return false;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::_add_to_bucket(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(!_bucket || !_is_in_bucket(index));

    if (!rehash_if_need())
    {
        DataType& data      = _data[index];
        SizeType& index_ref = _bucket[_bucket_index(data.hash)];
        data.next           = index_ref;
        index_ref           = index;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::_remove_from_bucket(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(_is_in_bucket(index));

    DataType& data = _data[index];
    for (SizeType& index_ref = _bucket[_bucket_index(data.hash)]; index_ref != npos; index_ref = _data[index_ref].next)
    {
        if (index_ref == index)
        {
            index_ref = data.next;
            break;
        }
    }
}

// ctor & dtor
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(Alloc alloc)
    : _data(std::move(alloc))
{
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(SizeType reserve_size, Alloc alloc)
    : _data(std::move(alloc))
{
    reserve(reserve_size);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(const T* p, SizeType n, Alloc alloc)
    : _data(std::move(alloc))
{
    append(p, n);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(std::initializer_list<T> init_list, Alloc alloc)
    : _data(std::move(alloc))
{
    append(init_list);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::~SparseHashSet() { release(); }

// copy & move
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(const SparseHashSet& other, Alloc alloc)
    : _data(std::move(alloc))
{
    *this = other;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(SparseHashSet&& other)
    : _bucket(other._bucket)
    , _bucket_size(other._bucket_size)
    , _bucket_mask(other._bucket_mask)
    , _data(std::move(other._data))
{
    other._bucket      = nullptr;
    other._bucket_size = 0;
    other._bucket_mask = 0;
}

// assign & move assign
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>& SparseHashSet<T, TBitBlock, Config, Alloc>::operator=(const SparseHashSet& rhs)
{
    if (this != &rhs)
    {
        // clear
        clear();

        if (!rhs.empty())
        {
            // realloc bucket
            if (_bucket_size != rhs._bucket_size)
            {
                _bucket_size = rhs._bucket_size;
                _bucket_mask = rhs._bucket_mask;
                _bucket      = _data.allocator().template alloc<SizeType>(_bucket_size);
            }

            // copy bucket
            std::memcpy(_bucket, rhs._bucket, _bucket_size * sizeof(SizeType));

            // copy data
            _data = rhs._data;
        }
    }
    return *this;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>& SparseHashSet<T, TBitBlock, Config, Alloc>::operator=(SparseHashSet&& rhs)
{
    if (this != &rhs)
    {
        // clear
        clear();

        // move data
        _bucket      = rhs._bucket;
        _bucket_size = rhs._bucket_size;
        _bucket_mask = rhs._bucket_mask;
        _data        = std::move(rhs._data);

        // clean up rhs
        rhs._bucket      = nullptr;
        rhs._bucket_size = 0;
        rhs._bucket_mask = 0;
    }
    return *this;
}

// compare
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::operator==(const SparseHashSet& rhs) const
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
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::operator!=(const SparseHashSet& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::size() const
{
    return _data.size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::capacity() const
{
    return _data.capacity();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::slack() const
{
    return _data.slack();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::sparse_size() const
{
    return _data.sparse_size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::hole_size() const
{
    return _data.hole_size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::bit_array_size() const
{
    return _data.bit_array_size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::free_list_head() const
{
    return _data.free_list_head();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::bucket_size() const
{
    return _bucket_size;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::bucket_mask() const
{
    return _bucket_mask;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_compact() const
{
    return _data.is_compact();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
bool SparseHashSet<T, TBitBlock, Config, Alloc>::empty() const
{
    return _data.empty();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataArr& SparseHashSet<T, TBitBlock, Config, Alloc>::data_arr()
{
    return _data;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataArr& SparseHashSet<T, TBitBlock, Config, Alloc>::data_arr() const
{
    return _data;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType* SparseHashSet<T, TBitBlock, Config, Alloc>::bucket()
{
    return _bucket;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType* SparseHashSet<T, TBitBlock, Config, Alloc>::bucket() const
{
    return _bucket;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
Alloc& SparseHashSet<T, TBitBlock, Config, Alloc>::allocator()
{
    return _data.allocator();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const Alloc& SparseHashSet<T, TBitBlock, Config, Alloc>::allocator() const
{
    return _data.allocator();
}

// validate
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::has_data(SizeType idx) const
{
    return _data.has_data(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_hole(SizeType idx) const
{
    return _data.is_hole(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_valid_index(SizeType idx) const
{
    return _data.is_valid_index(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_valid_pointer(const T* p) const
{
    return _data.is_valid_pointer(p);
}

// memory op
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::clear()
{
    _data.clear();
    _clean_bucket();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::release(SizeType capacity)
{
    _data.release(capacity);
    _resize_bucket();
    _clean_bucket();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::reserve(SizeType capacity)
{
    _data.reserve(capacity);
    rehash_if_need();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::shrink()
{
    _data.shrink();
    if (_resize_bucket())
    {
        rehash();
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::compact()
{
    if (_data.compact())
    {
        rehash();
        return true;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::compact_stable()
{
    if (_data.compact_stable())
    {
        rehash();
        return true;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::compact_top()
{
    return _data.compact_top();
}

// data op
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::KeyType& SparseHashSet<T, TBitBlock, Config, Alloc>::key_of(T& v) const
{
    return keyMapperType()(v);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE const typename SparseHashSet<T, TBitBlock, Config, Alloc>::KeyType& SparseHashSet<T, TBitBlock, Config, Alloc>::key_of(const T& v) const
{
    return keyMapperType()(v);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::key_equal(const T& a, const T& b) const
{
    return ComparerType()(key_of(a), key_of(b));
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::HashType SparseHashSet<T, TBitBlock, Config, Alloc>::hash_of(const T& v) const
{
    return HasherType()(key_of(v));
}

// rehash
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::need_rehash() const
{
    SizeType new_bucket_size = _calc_bucket_size(_data.capacity());
    return _data.size() > 0 && (new_bucket_size != _bucket_size);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::rehash() const
{
    // resize bucket
    _resize_bucket();

    // rehash
    if (_bucket)
    {
        _clean_bucket();
        for (auto it = _data.begin(); it; ++it)
        {
            // link to head
            SizeType& index_ref = _bucket[_bucket_index(it->hash)];
            it->next            = index_ref;
            index_ref           = it.index();
        }
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::rehash_if_need() const
{
    if (need_rehash())
    {
        rehash();
        return true;
    }
    else
    {
        return false;
    }
}

// try to add (first check existence, then add, never assign)
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add(const T& v)
{
    HashType hash = hash_of(v);
    return add_ex(
    hash,
    [&v, this](const KeyType& k) { return ComparerType()(k, key_of(v)); },
    [&v](void* p) { new (p) T(v); });
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add(T&& v)
{
    HashType hash = hash_of(v);
    return add_ex(
    hash,
    [&v, this](const KeyType& k) { return ComparerType()(k, key_of(v)); },
    [&v](void* p) { new (p) T(std::move(v)); });
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer, typename Constructor>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor)
{
    DataRef add_result = add_unsafe(hash, std::forward<Comparer>(comparer));

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        constructor(add_result.data);
        SKR_ASSERT(_data[add_result.index].hash == hash_of(*add_result.data));
        _add_to_bucket(add_result.index);
    }

    return add_result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add_unsafe(HashType hash, Comparer&& comparer)
{
    if constexpr (!Config::multi_key)
    {
        DataRef ref = find_ex(hash, std::forward<Comparer>(comparer));
        if (ref)
        {
            ref.already_exist = true;
            return ref;
        }
        else
        {
            auto data_arr_ref  = _data.add_unsafe();
            data_arr_ref->hash = hash;
            return { &data_arr_ref->data, data_arr_ref.index, false };
        }
    }
    else
    {
        auto data_arr_ref  = _data.add_unsafe();
        data_arr_ref->hash = hash;
        return { &data_arr_ref->data, data_arr_ref.index, false };
    }
}

// emplace
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::emplace(Args&&... args)
{
    // emplace to data array
    auto data_arr_ref = _data.add_unsafe();
    new (&data_arr_ref->data) T(std::forward<Args>(args)...);
    data_arr_ref->hash = hash_of(data_arr_ref->data);

    if constexpr (!Config::multi_key)
    {
        // check if data has been added to set
        if (DataRef found_info = find_ex(data_arr_ref->hash, [&data_arr_ref, this](const KeyType& k) { return ComparerType()(k, key_of(data_arr_ref->data)); }))
        {
            // remove new data
            _data.remove_at(data_arr_ref.index);

            // return old data
            found_info.already_exist = true;
            return found_info;
        }
    }

    _add_to_bucket(data_arr_ref.index);

    return { &data_arr_ref->data, data_arr_ref.index, false };
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer, typename... Args>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::emplace_ex(HashType hash, Comparer&& comparer, Args&&... args)
{
    DataRef add_result = add_unsafe(hash, comparer);

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        new (add_result.data) T(std::forward<Args>(args)...);
        SKR_ASSERT(_data[add_result.index].hash == hash_of(*add_result.data));
        _add_to_bucket(add_result.index);
    }

    return add_result;
}

// append
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::append(const SparseHashSet& set)
{
    for (const auto& v : set)
    {
        add(v);
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::append(std::initializer_list<T> init_list)
{
    for (const auto& v : init_list)
    {
        add(v);
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::append(const T* p, SizeType n)
{
    for (SizeType i = 0; i < n; ++i)
    {
        add(p[n]);
    }
}

// remove
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::remove(const KeyType& key)
{
    HashType hash = HasherType()(key);
    return remove_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::remove_all(const KeyType& key)
{
    HashType hash = HasherType()(key);
    remove_all_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::remove_ex(HashType hash, Comparer&& comparer)
{
    if (DataRef ref = find_ex(hash, std::forward<Comparer>(comparer)))
    {
        _remove_from_bucket(ref.index);
        _data.remove_at(ref.index);
        return ref;
    }
    return {};
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::remove_all_ex(HashType hash, Comparer&& comparer)
{
    SizeType search_index = _bucket_data(hash);
    SizeType count        = 0;
    while (search_index != npos)
    {
        DataType& data = _data[search_index];
        SizeType  next = data.next;
        if (data.hash == hash && comparer(key_of(data.data)))
        {
            _remove_from_bucket(search_index);
            _data.remove_at(search_index);
            ++count;
        }
        search_index = next;
    }
    return count;
}

// find
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find(const KeyType& key)
{
    HashType hash = HasherType()(key);
    return find_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CDataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find(const KeyType& key) const
{
    HashType hash = HasherType()(key);
    return find_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find_ex(HashType hash, Comparer&& comparer)
{
    if (!_bucket) return {};

    SizeType search_index = _bucket[_bucket_index(hash)];
    while (search_index != npos)
    {
        auto& node = _data[search_index];
        if (node.hash == hash && comparer(key_of(node.data)))
        {
            return { &node.data, search_index };
        }
        search_index = node.next;
    }
    return {};
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CDataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find_ex(HashType hash, Comparer&& comparer) const
{
    DataRef info = const_cast<SparseHashSet*>(this)->find_ex(hash, std::forward<Comparer>(comparer));
    return { info.data, info.index };
}

// contain
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::contain(const KeyType& key) const
{
    return (bool)find(key);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::count(const KeyType& key) const
{
    HashType hash = HasherType()(key);
    return count_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::contain_ex(HashType hash, Comparer&& comparer) const
{
    return (bool)find_ex(hash, std::forward<Comparer>(comparer));
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::count_ex(HashType hash, Comparer&& comparer) const
{
    SizeType& search_index = _bucket[_bucket_index(hash)];
    SizeType  count        = 0;

    while (search_index != npos)
    {
        DataType& data = _data[search_index];
        if (data.hash == hash && comparer(key_of(data.data)))
        {
            ++count;
        }
        search_index = data.next;
    }
    return count;
}

// sort
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::sort(TP&& p)
{
    _data.template sort([&](const DataType& a, const DataType& b) { return p(key_of(a), key_of(b)); });
    rehash();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::sort_stable(TP&& p)
{
    _data.template sort_stable([&](const DataType& a, const DataType& b) { return p(key_of(a), key_of(b)); });
    rehash();
}

// set ops
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc> SparseHashSet<T, TBitBlock, Config, Alloc>::operator&(const SparseHashSet& rhs) const
{
    bool                 rhs_smaller = size() > rhs.size();
    const SparseHashSet& a           = rhs_smaller ? rhs : *this;
    const SparseHashSet& b           = rhs_smaller ? *this : rhs;

    SparseHashSet result;
    result.reserve(a.size());

    for (const auto& v : a)
    {
        if (b.contain(key_of(v)))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc> SparseHashSet<T, TBitBlock, Config, Alloc>::operator|(const SparseHashSet& rhs) const
{
    SparseHashSet result(*this);
    for (const auto& v : rhs)
    {
        result.add(v);
    }
    return result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc> SparseHashSet<T, TBitBlock, Config, Alloc>::operator^(const SparseHashSet& rhs) const
{
    SparseHashSet result(size());

    for (const auto& v : *this)
    {
        if (!rhs.contain(key_of(v)))
        {
            result.add(v);
        }
    }

    for (const auto& v : rhs)
    {
        if (!contain(v))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc> SparseHashSet<T, TBitBlock, Config, Alloc>::operator-(const SparseHashSet& rhs) const
{
    SparseHashSet result(size());

    for (const auto& v : *this)
    {
        if (!rhs.contain(key_of(v)))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_sub_set_of(const SparseHashSet& rhs) const
{
    if (rhs.size() >= size())
    {
        for (const auto& v : *this)
        {
            if (!rhs.contain(key_of(v)))
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
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::It SparseHashSet<T, TBitBlock, Config, Alloc>::begin()
{
    return It(_data.data(), _data.sparse_size(), _data.bit_array());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::It SparseHashSet<T, TBitBlock, Config, Alloc>::end()
{
    return It(_data.data(), _data.sparse_size(), _data.bit_array(), _data.sparse_size());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CIt SparseHashSet<T, TBitBlock, Config, Alloc>::begin() const
{
    return CIt(_data.data(), _data.sparse_size(), _data.bit_array());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CIt SparseHashSet<T, TBitBlock, Config, Alloc>::end() const
{
    return CIt(_data.data(), _data.sparse_size(), _data.bit_array(), _data.sparse_size());
}
} // namespace skr
