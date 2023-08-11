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
// TODO. 完善 bucket 系列操作
// TODO. 完善 hashed 与 as 系列的校验
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
    DataArr&        data();
    const DataArr&  data() const;
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

    // add/remove node to bucket
    bool    is_in_bucket(SizeType index) const;
    DataRef add_to_bucket_or_assign(SizeType index);
    void    add_to_bucket(SizeType index);
    void    remove_from_bucket(SizeType index);

    // add
    // check existence then add
    DataRef add(const T& v);
    DataRef add(T&& v);
    DataRef add_hashed(const T& v, HashType hash);
    DataRef add_hashed(T&& v, HashType hash);
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    DataRef add_as(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer());

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename... Args>
    DataRef emplace_hashed(HashType hash, Args&&... args);

    // append
    void append(const SparseHashSet& set);
    void append(std::initializer_list<T> init_list);
    void append(T* p, SizeType n);

    // remove
    DataRef  remove(const KeyType& key);
    DataRef  remove_hashed(const KeyType& key, HashType hash);
    SizeType remove_all(const KeyType& key);                       // [multi set extend]
    SizeType remove_all_hashed(const KeyType& key, HashType hash); // [multi set extend]

    // remove as
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    SizeType remove_as(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer());
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    SizeType remove_all_as(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()); // [multi set extend]

    // find
    DataRef  find(const KeyType& key);
    CDataRef find(const KeyType& key) const;
    DataRef  find_hashed(const KeyType& key, HashType hash);
    CDataRef find_hashed(const KeyType& key, HashType hash) const;

    // find as
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    DataRef find_as(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer());
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    CDataRef find_as(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()) const;

    // contain
    bool     contain(const KeyType& key) const;
    bool     contain_hashed(const KeyType& key, HashType hash) const;
    SizeType count(const KeyType& key) const;                       // [multi set extend]
    SizeType count_hashed(const KeyType& key, HashType hash) const; // [multi set extend]

    // contain as
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    bool contain_as(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()) const;
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    SizeType count_as(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()) const; // [multi set extend]

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
    SizeType  _calc_bucket_size(SizeType data_size) const;
    void      _clean_bucket() const;
    bool      _resize_bucket() const;
    SizeType  _bucket_index(SizeType hash) const;
    SizeType& _bucket_data(SizeType hash) const;

private:
    mutable SizeType* m_bucket      = nullptr;
    mutable SizeType  m_bucket_size = 0;
    mutable SizeType  m_bucket_mask = 0;
    DataArr           m_data;
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
        return bit_ceil_log2(SizeType(data_size / avg_bucket_capacity) + basic_bucket_size);
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
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::_clean_bucket() const
{
    if (m_bucket)
    {
        SizeType* begin = m_bucket;
        SizeType* end   = m_bucket + m_bucket_size;
        for (; begin != end; ++begin)
        {
            *begin = npos;
        }
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::_resize_bucket() const
{
    SizeType new_bucket_size = _calc_bucket_size(m_data.capacity());

    if (new_bucket_size != m_bucket_size)
    {
        if (new_bucket_size)
        {
            m_bucket      = m_data.resize_container(m_bucket, m_bucket_size, m_bucket_size, new_bucket_size);
            m_bucket_size = new_bucket_size;
            m_bucket_mask = new_bucket_size - 1;
        }
        else
        {
            if (m_bucket)
            {
                m_data.allocator().free(m_bucket);
                m_bucket      = nullptr;
                m_bucket_size = 0;
                m_bucket_mask = 0;
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
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::_bucket_index(SizeType hash) const
{
    return hash & m_bucket_mask;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType& SparseHashSet<T, TBitBlock, Config, Alloc>::_bucket_data(SizeType hash) const
{
    return m_bucket[_bucket_index(hash)];
}

// ctor & dtor
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(Alloc alloc)
    : m_data(std::move(alloc))
{
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(SizeType reserve_size, Alloc alloc)
    : m_data(std::move(alloc))
{
    reserve(reserve_size);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(const T* p, SizeType n, Alloc alloc)
    : m_data(std::move(alloc))
{
    append(p, n);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(std::initializer_list<T> init_list, Alloc alloc)
    : m_data(std::move(alloc))
{
    append(init_list);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::~SparseHashSet() { release(); }

// copy & move
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(const SparseHashSet& other, Alloc alloc)
    : m_data(std::move(alloc))
{
    *this = other;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>::SparseHashSet(SparseHashSet&& other)
    : m_bucket(other.m_bucket)
    , m_bucket_size(other.m_bucket_size)
    , m_bucket_mask(other.m_bucket_mask)
    , m_data(std::move(other.m_data))
{
    other.m_bucket      = nullptr;
    other.m_bucket_size = 0;
    other.m_bucket_mask = 0;
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
            if (m_bucket_size != rhs.m_bucket_size)
            {
                m_bucket_size = rhs.m_bucket_size;
                m_bucket_mask = rhs.m_bucket_mask;
                m_bucket      = m_data.allocator().template alloc<SizeType>(m_bucket_size);
            }

            // copy bucket
            std::memcpy(m_bucket, rhs.m_bucket, m_bucket_size * sizeof(SizeType));

            // copy data
            m_data = rhs.m_data;
        }
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, Config, Alloc>& SparseHashSet<T, TBitBlock, Config, Alloc>::operator=(SparseHashSet&& rhs)
{
    if (this != &rhs)
    {
        // clear
        clear();

        // move data
        m_bucket      = rhs.m_bucket;
        m_bucket_size = rhs.m_bucket_size;
        m_bucket_mask = rhs.m_bucket_mask;
        m_data        = std::move(rhs.m_data);

        // clean up rhs
        rhs.m_bucket      = nullptr;
        rhs.m_bucket_size = 0;
        rhs.m_bucket_mask = 0;
    }
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
    return m_data.size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::capacity() const
{
    return m_data.capacity();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::slack() const
{
    return m_data.slack();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::sparse_size() const
{
    return m_data.sparse_size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::hole_size() const
{
    return m_data.hole_size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::bit_array_size() const
{
    return m_data.bit_array_size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::free_list_head() const
{
    return m_data.free_list_head();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::bucket_size() const
{
    return m_bucket_size;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::bucket_mask() const
{
    return m_bucket_mask;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_compact() const
{
    return m_data.is_compact();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
bool SparseHashSet<T, TBitBlock, Config, Alloc>::empty() const
{
    return m_data.empty();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataArr& SparseHashSet<T, TBitBlock, Config, Alloc>::data()
{
    return m_data;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataArr& SparseHashSet<T, TBitBlock, Config, Alloc>::data() const
{
    return m_data;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType* SparseHashSet<T, TBitBlock, Config, Alloc>::bucket()
{
    return m_bucket;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType* SparseHashSet<T, TBitBlock, Config, Alloc>::bucket() const
{
    return m_bucket;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
Alloc& SparseHashSet<T, TBitBlock, Config, Alloc>::allocator()
{
    return m_data.allocator();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const Alloc& SparseHashSet<T, TBitBlock, Config, Alloc>::allocator() const
{
    return m_data.allocator();
}

// validate
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::has_data(SizeType idx) const
{
    return m_data.has_data(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_hole(SizeType idx) const
{
    return m_data.is_hole(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_valid_index(SizeType idx) const
{
    return m_data.is_valid_index(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_valid_pointer(const T* p) const
{
    return m_data.is_valid_pointer(p);
}

// memory op
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::clear()
{
    m_data.clear();
    _clean_bucket();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::release(SizeType capacity)
{
    m_data.release(capacity);
    _resize_bucket();
    _clean_bucket();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::reserve(SizeType capacity)
{
    m_data.reserve(capacity);
    rehash_if_need();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::shrink()
{
    m_data.shrink();
    if (_resize_bucket())
    {
        rehash();
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::compact()
{
    if (m_data.compact())
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
    if (m_data.compact_stable())
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
    return m_data.compact_top();
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
    SizeType new_bucket_size = _calc_bucket_size(m_data.capacity());
    return m_data.size() > 0 && (new_bucket_size != m_bucket_size);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::rehash() const
{
    // try resize bucket
    _resize_bucket();

    // rehash
    if (m_bucket)
    {
        _clean_bucket();
        for (auto it = m_data.begin(); it; ++it)
        {
            SizeType& id_ref = _bucket_data(it->hash);
            it->next         = id_ref;
            id_ref           = it.index();
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

// per element hash op
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::is_in_bucket(SizeType index) const
{
    if (has_data(index))
    {
        DataType& data = m_data[index];
        SizeType  id   = _bucket_data(data.hash);

        while (id != npos)
        {
            if (id == index)
            {
                return true;
            }
            id = m_data[id].next;
        }
        return false;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add_to_bucket_or_assign(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(!is_in_bucket(index));

    DataType& data = m_data[index];

    DataRef info(&data.data, index);
    if constexpr (!Config::multi_key)
    {
        // check if data has been added to set
        if (DataRef& found_info = find_hashed(key_of(data.data), data.hash))
        {
            // cover the old data
            memory::move(&found_info.data, &data.data, 1);

            // remove old data
            m_data.remove_at(index);

            // modify data
            info               = found_info;
            info.already_exist = true;
        }
        else
        {
            // link to bucket
            if (!rehash_if_need())
            {
                SizeType& id_ref = _bucket_data(data.hash);
                data.next        = id_ref;
                id_ref           = index;
            }
        }
    }
    else
    {
        // link to bucket
        if (!rehash_if_need())
        {
            SizeType& id_ref = _bucket_data(data.hash);
            data.next        = id_ref;
            id_ref           = index;
        }
    }
    return info;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::add_to_bucket(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(!is_in_bucket(index));

    DataType& data   = m_data[index];
    SizeType& id_ref = _bucket_data(data.hash);
    data->next       = id_ref;
    id_ref           = index;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::remove_from_bucket(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(is_in_bucket(index));

    DataType& data = m_data[index];
    for (SizeType& id_ref = _bucket_data(data.hash); id_ref != npos; id_ref = m_data[id_ref].next)
    {
        if (id_ref == index)
        {
            id_ref = data.next;
            break;
        }
    }
}

// try to add (first check existence, then add, never assign)
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add(const T& v)
{
    HashType hash = hash_of(v);

    if (DataRef ref = find_hashed(v, hash))
    {
        ref.already_exist = true;
        return ref;
    }
    else
    {
        auto info = m_data.addUnsafe();
        new (&info->data) T(std::forward(v));
        info->hash = hash;
        SKR_ASSERT(info->hash == hash_of(*info->data));

        add_to_bucket(info.index());

        return { info->data, info.index, false };
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add(T&& v)
{
    HashType hash = hash_of(v);

    if (DataRef ref = find_hashed(v, hash))
    {
        ref.already_exist = true;
        return ref;
    }
    else
    {
        auto info = m_data.addUnsafe();
        new (&info->data) T(std::forward(v));
        info->hash = hash;
        SKR_ASSERT(info->hash == hash_of(*info->data));

        add_to_bucket(info.index());

        return { info->data, info.index, false };
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add_hashed(const T& v, HashType hash)
{
    SKR_ASSERT(hash == hash_of(v));

    if (DataRef ref = find_hashed(v, hash))
    {
        ref.already_exist = true;
        return ref;
    }
    else
    {
        auto info = m_data.addUnsafe();
        new (&info->data) T(std::forward(v));
        info->hash = hash;
        SKR_ASSERT(info->hash == hash_of(*info->data));

        add_to_bucket(info.index());

        return { info->data, info.index, false };
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add_hashed(T&& v, HashType hash)
{
    SKR_ASSERT(hash == hash_of(v));

    if (DataRef ref = find_hashed(v, hash))
    {
        ref.already_exist = true;
        return ref;
    }
    else
    {
        auto info = m_data.addUnsafe();
        new (&info->data) T(std::forward(v));
        info->hash = hash;
        SKR_ASSERT(info->hash == hash_of(*info->data));

        add_to_bucket(info.index());

        return { info->data, info.index, false };
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::add_as(AsType&& v, AsHasher&& hasher,
                                                                                                                           AsComparer&& comparer)
{
    HashType hash            = hasher(v);
    auto     constant_hasher = [hash](auto& a) { return hash; };

    if (DataRef ref = find_as(std::forward<AsType>(v), constant_hasher, std::forward<AsComparer>(comparer)))
    {
        ref.already_exist = true;
        return ref;
    }
    else
    {
        auto info = m_data.addUnsafe();
        new (&info->data) T(std::forward(v));
        info->hash = hash;
        SKR_ASSERT(info->hash == hash_of(*info->data));

        add_to_bucket(info.index());

        return { info->data, info.index, false };
    }
}

// emplace
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::emplace(Args&&... args)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::forward<Args>(args)...);
    info->hash = hash_of(info->data);

    // link or assign
    add_to_bucket_or_assign(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::emplace_hashed(HashType hash, Args&&... args)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::forward<Args>(args)...);
    info->hash = hash;
    SKR_ASSERT(info->hash == hash_of(*info->data));

    // link or assign
    add_to_bucket_or_assign(info.index);
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
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::append(T* p, SizeType n)
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
    if (DataRef ref = find(key))
    {
        memory::destruct(ref.data, 1);
        remove_from_bucket(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::remove_hashed(const KeyType& key, HashType hash)
{
    if (DataRef ref = find_hashed(key, hash))
    {
        memory::destruct(ref.data, 1);
        remove_from_bucket(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::remove_all(const KeyType& key)
{
    HashType  hash   = HasherType()(key);
    SizeType& id_ref = _bucket_data(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(key_of(data.data), key))
        {
            memory::destruct(&data.data, 1);
            remove_from_bucket(id_ref);
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::remove_all_hashed(const KeyType& key, HashType hash)
{
    SKR_ASSERT(HasherType()(key) == hash);

    SizeType& id_ref = _bucket_data(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(key_of(data.data), key))
        {
            memory::destruct(&data.data, 1);
            remove_from_bucket(id_ref);
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}

// remove as
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::remove_as(AsType&& v, AsHasher&& hasher,
                                                                                                                               AsComparer&& comparer)
{
    if (DataRef info = find_as(std::forward<AsType>(v), std::forward<AsHasher>(hasher), std::forward<AsComparer>(comparer)))
    {
        memory::destruct(info.data, 1);
        remove_from_bucket(info.index);
        return info.index;
    }
    return npos;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::remove_all_as(AsType&& v, AsHasher&& hasher,
                                                                                                                                   AsComparer&& comparer)
{
    HashType  hash   = hasher(v);
    SizeType& id_ref = _bucket_data(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (comparer(data.data, v))
        {
            memory::destruct(&data.data, 1);
            remove_from_bucket(id_ref);
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}

// find
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find(const KeyType& key)
{
    HashType hash   = HasherType()(key);
    SizeType id_ref = _bucket_data(hash);
    DataRef  info;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(key_of(data.data), key))
        {
            info.data  = &data.data;
            info.index = id_ref;
            break;
        }
        id_ref = data.next;
    }
    return info;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CDataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find(const KeyType& key) const
{
    DataRef info = const_cast<SparseHashSet*>(this)->find(key);
    return CDataRef(info.data, info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find_hashed(const KeyType& key, HashType hash)
{
    SKR_ASSERT(HasherType()(key) == hash);

    SizeType id_ref = _bucket_data(hash);
    DataRef  info;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(key_of(data.data), key))
        {
            info.data  = &data.data;
            info.index = id_ref;
            break;
        }
        id_ref = data.next;
    }
    return info;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CDataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find_hashed(const KeyType& key, HashType hash) const
{
    DataRef info = const_cast<SparseHashSet*>(this)->find_hashed(key, hash);
    return CDataRef(info.data, info.index);
}

// find as
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::DataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find_as(AsType&& v, AsHasher&& hasher,
                                                                                                                            AsComparer&& comparer)
{
    HashType hash   = hasher(v);
    SizeType id_ref = _bucket_data(hash);
    DataRef  info;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (comparer(key_of(data.data), v))
        {
            info.data  = &data.data;
            info.index = id_ref;
            break;
        }
        id_ref = data.next;
    }
    return info;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CDataRef SparseHashSet<T, TBitBlock, Config, Alloc>::find_as(AsType&& v, AsHasher&& hasher,
                                                                                                                             AsComparer&& comparer) const
{
    DataRef info = const_cast<SparseHashSet*>(this)->find_as(std::forward<AsType>(v), std::forward<AsHasher>(hasher), std::forward<AsComparer>(comparer));
    return CDataRef(info.data, info.index);
}

// contain
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::contain(const KeyType& key) const
{
    return (bool)find(key);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::contain_hashed(const KeyType& key, HashType hash) const
{
    return (bool)find_hashed(key, hash);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::count(const KeyType& key) const
{
    HashType  hash   = HasherType()(key);
    SizeType& id_ref = _bucket_data(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(key_of(data.data), key))
        {
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::count_hashed(const KeyType& key,
                                                                                                                                  HashType       hash) const
{
    SKR_ASSERT(HasherType()(key) == hash);

    SizeType& id_ref = _bucket_data(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(key_of(data.data), key))
        {
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}

// contain as
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE bool SparseHashSet<T, TBitBlock, Config, Alloc>::contain_as(AsType&& v, AsHasher&& hasher, AsComparer&& comparer) const
{
    return (bool)find_as(std::forward<AsType>(v), std::forward<AsHasher>(hasher), std::forward<AsComparer>(comparer));
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::SizeType SparseHashSet<T, TBitBlock, Config, Alloc>::count_as(AsType&& v, AsHasher&& hasher,
                                                                                                                              AsComparer&& comparer) const
{
    HashType  hash   = hasher(v);
    SizeType& id_ref = _bucket_data(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (comparer(key_of(data.data), v))
        {
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}

// sort
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::sort(TP&& p)
{
    m_data.template sort([&](const DataType& a, const DataType& b) { return p(key_of(a), key_of(b)); });
    rehash();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashSet<T, TBitBlock, Config, Alloc>::sort_stable(TP&& p)
{
    m_data.template sort_stable([&](const DataType& a, const DataType& b) { return p(key_of(a), key_of(b)); });
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
    if (rhs.size() <= size())
    {
        for (const auto& v : rhs)
        {
            if (!contain(key_of(v)))
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
    return It(m_data.data(), m_data.sparse_size(), m_data.bitArray());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::It SparseHashSet<T, TBitBlock, Config, Alloc>::end()
{
    return It(m_data.data(), m_data.sparse_size(), m_data.bitArray(), m_data.sparse_size());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CIt SparseHashSet<T, TBitBlock, Config, Alloc>::begin() const
{
    return CIt(m_data.data(), m_data.sparse_size(), m_data.bitArray());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, Config, Alloc>::CIt SparseHashSet<T, TBitBlock, Config, Alloc>::end() const
{
    return CIt(m_data.data(), m_data.sparse_size(), m_data.bitArray(), m_data.sparse_size());
}
} // namespace skr
