#pragma once
#include "SkrRT/stl/fwd_stl.hpp"
#include "SkrRT/base/config.hpp"
#include "SkrRT/algo/functor.hpp"
#include "SkrRT/stl/sparse_array/sparse_array.hpp"
#include "sparse_hash_set_iterator.hpp"
#include "SkrRT/stl/allocator/allocator.hpp"
#include "SkrRT/base/tools/hash.hpp"

// USet config
namespace skr
{
template <typename T, bool MultiKey>
struct USetConfigDefault {
    using KeyType       = T;
    using KeyMapperType = MapFwd<T>;
    using HashType      = size_t;
    using HasherType    = Hash<KeyType>;
    using ComparerType  = Equal<KeyType>;

    static constexpr bool multi_key = MultiKey;
};
} // namespace skr

// USet def
namespace skr
{
template <typename T, typename TBitBlock, typename Config, typename Alloc>
class USet
{
public:
    // from config
    using HashType      = typename Config::HashType;
    using KeyType       = typename Config::KeyType;
    using keyMapperType = typename Config::KeyMapperType;
    using HasherType    = typename Config::HasherType;
    using ComparerType  = typename Config::ComparerType;

    // basic
    using SizeType                        = typename Alloc::SizeType;
    using DataType                        = USetData<T, SizeType, HashType>;
    using DataArr                         = SparseArray<DataType, TBitBlock, Alloc>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref & iterator
    using DataRef  = USetDataRef<T, SizeType>;
    using CDataRef = USetDataRef<const T, SizeType>;
    using It       = USetIt<T, TBitBlock, SizeType, HashType, false>;
    using CIt      = USetIt<T, TBitBlock, SizeType, HashType, true>;

public:
    // ctor & dtor
    USet(Alloc alloc = Alloc());
    USet(SizeType reserve_size, Alloc alloc = Alloc());
    USet(const T* p, SizeType n, Alloc alloc = Alloc());
    USet(std::initializer_list<T> init_list, Alloc alloc = Alloc());
    ~USet();

    // copy & move
    USet(const USet& other, Alloc alloc = Alloc());
    USet(USet&& other);

    // assign & move assign
    USet& operator=(const USet& rhs);
    USet& operator=(USet&& rhs);

    // compare
    bool operator==(const USet& rhs) const;
    bool operator!=(const USet& rhs) const;

    // getter
    SizeType        size() const;
    SizeType        capacity() const;
    SizeType        slack() const;
    SizeType        sparseSize() const;
    SizeType        holeSize() const;
    SizeType        bitArraySize() const;
    SizeType        freelistHead() const;
    SizeType        bucketSize() const;
    SizeType        bucketMask() const;
    bool            isCompact() const;
    bool            empty() const;
    DataArr&        data();
    const DataArr&  data() const;
    SizeType*       bucket();
    const SizeType* bucket() const;
    Alloc&          allocator();
    const Alloc&    allocator() const;

    // validate
    bool hasData(SizeType idx) const;
    bool isHole(SizeType idx) const;
    bool isValidIndex(SizeType idx) const;
    bool isValidPointer(const T* p) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    bool compact();
    bool compactStable();
    bool compactTop();

    // data op
    KeyType&       keyOf(T& v) const;
    const KeyType& keyOf(const T& v) const;
    bool           keyEqual(const T& a, const T& b) const;
    HashType       hashOf(const T& v) const;

    // rehash
    bool needRehash() const;
    void rehash() const;
    bool rehashIfNeed() const;

    // per element hash op
    bool    isInBucket(SizeType index) const;
    DataRef addToBucketOrAssign(SizeType index);
    void    addToBucketAnyway(SizeType index);
    void    removeFromBucket(SizeType index);

    // add (add or assign)
    DataRef add(const T& v);
    DataRef add(T&& v);
    DataRef addHashed(const T& v, HashType hash);
    DataRef addHashed(T&& v, HashType hash);

    // add anyway (add but never check existence)
    DataRef addAnyway(const T& v);
    DataRef addAnyway(T&& v);
    DataRef addAnywayHashed(const T& v, HashType hash);
    DataRef addAnywayHashed(T&& v, HashType hash);

    // try to add (first check existence, then add, never assign)
    DataRef tryAdd(const T& v);
    DataRef tryAdd(T&& v);
    DataRef tryAddHashed(const T& v, HashType hash);
    DataRef tryAddHashed(T&& v, HashType hash);
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    DataRef tryAddAs(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer());

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename... Args>
    DataRef emplaceHashed(HashType hash, Args&&... args);
    template <typename... Args>
    DataRef emplaceAnyway(Args&&... args);
    template <typename... Args>
    DataRef emplaceAnywayHashed(HashType hash, Args&&... args);

    // append
    void append(const USet& set);
    void append(std::initializer_list<T> init_list);
    void append(T* p, SizeType n);

    // remove
    DataRef  remove(const KeyType& key);
    DataRef  removeHashed(const KeyType& key, HashType hash);
    SizeType removeAll(const KeyType& key);                      // [multi set extend]
    SizeType removeAllHashed(const KeyType& key, HashType hash); // [multi set extend]

    // remove as
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    SizeType removeAs(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer());
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    SizeType removeAllAs(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()); // [multi set extend]

    // modify
    T&       operator[](SizeType index);
    const T& operator[](SizeType index) const;

    // find
    DataRef  find(const KeyType& key);
    CDataRef find(const KeyType& key) const;
    DataRef  findHashed(const KeyType& key, HashType hash);
    CDataRef findHashed(const KeyType& key, HashType hash) const;

    // find as
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    DataRef findAs(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer());
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    CDataRef findAs(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()) const;

    // contain
    bool     contain(const KeyType& key) const;
    bool     containHashed(const KeyType& key, HashType hash) const;
    SizeType count(const KeyType& key) const;                      // [multi set extend]
    SizeType countHashed(const KeyType& key, HashType hash) const; // [multi set extend]

    // contain as
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    bool containAs(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()) const;
    template <typename AsType, typename AsHasher = Hash<AsType>, typename AsComparer = Equal<>>
    SizeType countAs(AsType&& v, AsHasher&& hasher = AsHasher(), AsComparer&& comparer = AsComparer()) const; // [multi set extend]

    // sort
    template <typename TP = Less<T>>
    void sort(TP&& p = TP());
    template <typename TP = Less<T>>
    void sortStable(TP&& p = TP());

    // set ops
    USet operator&(const USet& rhs) const;  // intersect
    USet operator|(const USet& rhs) const;  // union
    USet operator^(const USet& rhs) const;  // difference
    USet operator-(const USet& rhs) const;  // sub
    bool isSubsetOf(const USet& rhs) const; // sub set

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;

private:
    // helpers
    SizeType  _calcBucketSize(SizeType data_size) const;
    void      _cleanBucket() const;
    bool      _resizeBucket() const;
    SizeType  _bucketIndex(SizeType hash) const;
    SizeType& _bucketData(SizeType hash) const;

private:
    mutable SizeType* m_bucket;
    mutable SizeType  m_bucket_size;
    mutable SizeType  m_bucket_mask;
    DataArr           m_data;
};
} // namespace skr

// USet impl
namespace skr
{
// helpers
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::_calcBucketSize(SizeType data_size) const
{
    static constexpr SizeType min_size_to_hash    = 4;
    static constexpr SizeType basic_bucket_size   = 8;
    static constexpr SizeType avg_bucket_capacity = 2;

    if (data_size >= min_size_to_hash)
    {
        return ceilPowerOf2(SizeType(data_size / avg_bucket_capacity) + basic_bucket_size);
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
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::_cleanBucket() const
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
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::_resizeBucket() const
{
    SizeType new_bucket_size = _calcBucketSize(m_data.capacity());

    if (new_bucket_size != m_bucket_size)
    {
        if (new_bucket_size)
        {
            m_bucket      = m_data.resizeContainer(m_bucket, m_bucket_size, m_bucket_size, new_bucket_size);
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
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::_bucketIndex(SizeType hash) const
{
    return hash & m_bucket_mask;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType& USet<T, TBitBlock, Config, Alloc>::_bucketData(SizeType hash) const
{
    return m_bucket[_bucketIndex(hash)];
}

// ctor & dtor
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc>::USet(Alloc alloc)
    : m_bucket(nullptr)
    , m_bucket_size(0)
    , m_bucket_mask(0)
    , m_data(std::move(alloc))
{
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc>::USet(SizeType reserve_size, Alloc alloc)
    : m_bucket(nullptr)
    , m_bucket_size(0)
    , m_bucket_mask(0)
    , m_data(std::move(alloc))
{
    reserve(reserve_size);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc>::USet(const T* p, SizeType n, Alloc alloc)
    : m_bucket(nullptr)
    , m_bucket_size(0)
    , m_bucket_mask(0)
    , m_data(std::move(alloc))
{
    append(p, n);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc>::USet(std::initializer_list<T> init_list, Alloc alloc)
    : m_bucket(nullptr)
    , m_bucket_size(0)
    , m_bucket_mask(0)
    , m_data(std::move(alloc))
{
    append(init_list);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc>::~USet() { release(); }

// copy & move
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc>::USet(const USet& other, Alloc alloc)
    : m_bucket(nullptr)
    , m_bucket_size(0)
    , m_bucket_mask(0)
    , m_data(std::move(alloc))
{
    *this = other;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc>::USet(USet&& other)
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
SKR_INLINE USet<T, TBitBlock, Config, Alloc>& USet<T, TBitBlock, Config, Alloc>::operator=(const USet& rhs)
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
SKR_INLINE USet<T, TBitBlock, Config, Alloc>& USet<T, TBitBlock, Config, Alloc>::operator=(USet&& rhs)
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
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::operator==(const USet& rhs) const
{
    if (size() == rhs.size())
    {
        return isSubsetOf(rhs);
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::operator!=(const USet& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::size() const
{
    return m_data.size();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::capacity() const
{
    return m_data.capacity();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::slack() const
{
    return m_data.slack();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::sparseSize() const
{
    return m_data.sparseSize();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::holeSize() const
{
    return m_data.holeSize();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::bitArraySize() const
{
    return m_data.bitArraySize();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::freelistHead() const
{
    return m_data.freelistHead();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::bucketSize() const
{
    return m_bucket_size;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::bucketMask() const
{
    return m_bucket_mask;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
bool USet<T, TBitBlock, Config, Alloc>::isCompact() const
{
    return m_data.isCompact();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
bool USet<T, TBitBlock, Config, Alloc>::empty() const
{
    return m_data.empty();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::DataArr& USet<T, TBitBlock, Config, Alloc>::data()
{
    return m_data;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const typename USet<T, TBitBlock, Config, Alloc>::DataArr& USet<T, TBitBlock, Config, Alloc>::data() const
{
    return m_data;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
typename USet<T, TBitBlock, Config, Alloc>::SizeType* USet<T, TBitBlock, Config, Alloc>::bucket()
{
    return m_bucket;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const typename USet<T, TBitBlock, Config, Alloc>::SizeType* USet<T, TBitBlock, Config, Alloc>::bucket() const
{
    return m_bucket;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
Alloc& USet<T, TBitBlock, Config, Alloc>::allocator()
{
    return m_data.allocator();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
const Alloc& USet<T, TBitBlock, Config, Alloc>::allocator() const
{
    return m_data.allocator();
}

// validate
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::hasData(SizeType idx) const
{
    return m_data.hasData(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::isHole(SizeType idx) const
{
    return m_data.isHole(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::isValidIndex(SizeType idx) const
{
    return m_data.isValidIndex(idx);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::isValidPointer(const T* p) const
{
    return m_data.isValidPointer(p);
}

// memory op
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::clear()
{
    m_data.clear();
    _cleanBucket();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::release(SizeType capacity)
{
    m_data.release(capacity);
    _resizeBucket();
    _cleanBucket();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::reserve(SizeType capacity)
{
    m_data.reserve(capacity);
    rehashIfNeed();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::shrink()
{
    m_data.shrink();
    if (_resizeBucket())
    {
        rehash();
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::compact()
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
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::compactStable()
{
    if (m_data.compactStable())
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
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::compactTop()
{
    return m_data.compactTop();
}

// data op
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::KeyType& USet<T, TBitBlock, Config, Alloc>::keyOf(T& v) const
{
    return keyMapperType()(v);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE const typename USet<T, TBitBlock, Config, Alloc>::KeyType& USet<T, TBitBlock, Config, Alloc>::keyOf(const T& v) const
{
    return keyMapperType()(v);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::keyEqual(const T& a, const T& b) const
{
    return ComparerType()(keyOf(a), keyOf(b));
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::HashType USet<T, TBitBlock, Config, Alloc>::hashOf(const T& v) const
{
    return HasherType()(keyOf(v));
}

// rehash
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::needRehash() const
{
    SizeType new_bucket_size = _calcBucketSize(m_data.capacity());
    return m_data.size() > 0 && (new_bucket_size != m_bucket_size);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::rehash() const
{
    // try resize bucket
    _resizeBucket();

    // rehash
    if (m_bucket)
    {
        _cleanBucket();
        for (auto it = m_data.begin(); it; ++it)
        {
            SizeType& id_ref = _bucketData(it->hash);
            it->next         = id_ref;
            id_ref           = it.index();
        }
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::rehashIfNeed() const
{
    if (needRehash())
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
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::isInBucket(SizeType index) const
{
    if (hasData(index))
    {
        DataType& data = m_data[index];
        SizeType  id   = _bucketData(data.hash);

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
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::addToBucketOrAssign(SizeType index)
{
    SKR_Assert(hasData(index));
    SKR_Assert(!isInBucket(index));

    DataType& data = m_data[index];

    DataRef info(&data.data, index);
    if constexpr (!Config::multi_key)
    {
        // check if data has been added to set
        if (DataRef& found_info = findHashed(keyOf(data.data), data.hash))
        {
            // cover the old data
            memory::move(&found_info.data, &data.data, 1);

            // remove old data
            m_data.removeAt(index);

            // modify data
            info               = found_info;
            info.already_exist = true;
        }
        else
        {
            // link to bucket
            if (!rehashIfNeed())
            {
                SizeType& id_ref = _bucketData(data.hash);
                data.next        = id_ref;
                id_ref           = index;
            }
        }
    }
    else
    {
        // link to bucket
        if (!rehashIfNeed())
        {
            SizeType& id_ref = _bucketData(data.hash);
            data.next        = id_ref;
            id_ref           = index;
        }
    }
    return info;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::addToBucketAnyway(SizeType index)
{
    SKR_Assert(hasData(index));
    SKR_Assert(!isInBucket(index));

    DataType& data   = m_data[index];
    SizeType& id_ref = _bucketData(data.hash);
    data->next       = id_ref;
    id_ref           = index;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::removeFromBucket(SizeType index)
{
    SKR_Assert(hasData(index));
    SKR_Assert(isInBucket(index));

    DataType& data = m_data[index];
    for (SizeType& id_ref = _bucketData(data.hash); id_ref != npos; id_ref = m_data[id_ref].next)
    {
        if (id_ref == index)
        {
            id_ref = data.next;
            break;
        }
    }
}

// add (add or assign)
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::add(const T& v)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(v);
    info->hash = hashOf(info->data);

    // link or assign
    addToBucketOrAssign(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::add(T&& v)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::move(v));
    info->hash = hashOf(info->data);

    // link or assign
    addToBucketOrAssign(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::addHashed(const T& v, HashType hash)
{
    SKR_Assert(hashOf(v) == hash);

    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(v);
    info->hash = hash;

    // link or assign
    addToBucketOrAssign(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::addHashed(T&& v, HashType hash)
{
    SKR_Assert(hashOf(v) == hash);

    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::move(v));
    info->hash = hash;

    // link or assign
    addToBucketOrAssign(info.index);
}

// add anyway (add but never check existence)
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::addAnyway(const T& v)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(v);
    info->hash = hashOf(info->data);

    // link directly
    addToBucketAnyway(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::addAnyway(T&& v)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::move(v));
    info->hash = hashOf(info->data);

    // link directly
    addToBucketAnyway(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::addAnywayHashed(const T& v, HashType hash)
{
    SKR_Assert(hashOf(v) == hash);

    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(v);
    info->hash = hash;

    // link directly
    addToBucketAnyway(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::addAnywayHashed(T&& v, HashType hash)
{

    SKR_Assert(hashOf(v) == hash);

    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::move(v));
    info->hash = hash;

    // link directly
    addToBucketAnyway(info.index);
}

// try to add (first check existence, then add, never assign)
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::tryAdd(const T& v)
{
    HashType hash = hashOf(v);
    if (!containHashed(v, hash))
    {
        return addHashed(v, hash);
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::tryAdd(T&& v)
{
    HashType hash = hashOf(v);
    if (!containHashed(v, hash))
    {
        return addHashed(std::move(v), hash);
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::tryAddHashed(const T& v, HashType hash)
{
    if (!containHashed(v, hash))
    {
        return addHashed(std::move(v), hash);
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::tryAddHashed(T&& v, HashType hash)
{
    if (!containHashed(v, hash))
    {
        return addHashed(std::move(v), hash);
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::tryAddAs(AsType&& v, AsHasher&& hasher,
                                                                                                           AsComparer&& comparer)
{
    HashType hash            = hasher(v);
    auto     constant_hasher = [hash](auto& a) { return hash; };
    if (!containAs(std::forward<AsType>(v), constant_hasher, std::forward<AsComparer>(comparer)))
    {
        // create node
        auto info = m_data.addUnsafe();
        new (&info->data) T(v);
        info->hash = hash;

        // link directly
        addToBucketAnyway(info.index);
    }
    return DataRef();
}

// emplace
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename... Args>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::emplace(Args&&... args)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::forward<Args>(args)...);
    info->hash = hashOf(info->data);

    // link or assign
    addToBucketOrAssign(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename... Args>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::emplaceHashed(HashType hash, Args&&... args)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::forward<Args>(args)...);
    info->hash = hash;
    SKR_Assert(hashOf(info->data) == hash);

    // link or assign
    addToBucketOrAssign(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename... Args>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::emplaceAnyway(Args&&... args)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::forward<Args>(args)...);
    info->hash = hashOf(info->data);

    // link directly
    addToBucketAnyway(info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename... Args>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::emplaceAnywayHashed(HashType hash, Args&&... args)
{
    // create node
    auto info = m_data.addUnsafe();
    new (&info->data) T(std::forward<Args>(args)...);
    info->hash = hash;
    SKR_Assert(hashOf(info->data) == hash);

    // link directly
    addToBucketAnyway(info.index);
}

// append
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::append(const USet& set)
{
    for (const auto& v : set)
    {
        add(v);
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::append(std::initializer_list<T> init_list)
{
    for (const auto& v : init_list)
    {
        add(v);
    }
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::append(T* p, SizeType n)
{
    for (SizeType i = 0; i < n; ++i)
    {
        add(p[n]);
    }
}

// remove
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::remove(const KeyType& key)
{
    if (DataRef ref = find(key))
    {
        memory::destruct(ref.data, 1);
        removeFromBucket(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::removeHashed(const KeyType& key, HashType hash)
{
    if (DataRef ref = findHashed(key, hash))
    {
        memory::destruct(ref.data, 1);
        removeFromBucket(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::removeAll(const KeyType& key)
{
    HashType  hash   = HasherType()(key);
    SizeType& id_ref = _bucketData(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(keyOf(data.data), key))
        {
            memory::destruct(&data.data, 1);
            removeFromBucket(id_ref);
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::removeAllHashed(const KeyType& key, HashType hash)
{
    SKR_Assert(HasherType()(key) == hash);

    SizeType& id_ref = _bucketData(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(keyOf(data.data), key))
        {
            memory::destruct(&data.data, 1);
            removeFromBucket(id_ref);
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}

// remove as
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::removeAs(AsType&& v, AsHasher&& hasher,
                                                                                                            AsComparer&& comparer)
{
    if (DataRef info = findAs(std::forward<AsType>(v), std::forward<AsHasher>(hasher), std::forward<AsComparer>(comparer)))
    {
        memory::destruct(info.data, 1);
        removeFromBucket(info.index);
        return info.index;
    }
    return npos;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::removeAllAs(AsType&& v, AsHasher&& hasher,
                                                                                                               AsComparer&& comparer)
{
    HashType  hash   = hasher(v);
    SizeType& id_ref = _bucketData(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (comparer(data.data, v))
        {
            memory::destruct(&data.data, 1);
            removeFromBucket(id_ref);
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}

// modify
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE T& USet<T, TBitBlock, Config, Alloc>::operator[](SizeType index)
{
    SKR_Assert(hasData(index));
    return m_data[index].data;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE const T& USet<T, TBitBlock, Config, Alloc>::operator[](SizeType index) const
{
    SKR_Assert(hasData(index));
    return m_data[index].data;
}

// find
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::find(const KeyType& key)
{
    HashType hash   = HasherType()(key);
    SizeType id_ref = _bucketData(hash);
    DataRef  info;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(keyOf(data.data), key))
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
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::CDataRef USet<T, TBitBlock, Config, Alloc>::find(const KeyType& key) const
{
    DataRef info = const_cast<USet*>(this)->find(key);
    return CDataRef(info.data, info.index);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::findHashed(const KeyType& key, HashType hash)
{
    SKR_Assert(HasherType()(key) == hash);

    SizeType id_ref = _bucketData(hash);
    DataRef  info;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(keyOf(data.data), key))
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
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::CDataRef USet<T, TBitBlock, Config, Alloc>::findHashed(const KeyType& key, HashType hash) const
{
    DataRef info = const_cast<USet*>(this)->findHashed(key, hash);
    return CDataRef(info.data, info.index);
}

// find as
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::DataRef USet<T, TBitBlock, Config, Alloc>::findAs(AsType&& v, AsHasher&& hasher,
                                                                                                         AsComparer&& comparer)
{
    HashType hash   = hasher(v);
    SizeType id_ref = _bucketData(hash);
    DataRef  info;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (comparer(keyOf(data.data), v))
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
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::CDataRef USet<T, TBitBlock, Config, Alloc>::findAs(AsType&& v, AsHasher&& hasher,
                                                                                                          AsComparer&& comparer) const
{
    DataRef info = const_cast<USet*>(this)->findAs(std::forward<AsType>(v), std::forward<AsHasher>(hasher), std::forward<AsComparer>(comparer));
    return CDataRef(info.data, info.index);
}

// contain
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::contain(const KeyType& key) const
{
    return (bool)find(key);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::containHashed(const KeyType& key, HashType hash) const
{
    return (bool)findHashed(key, hash);
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::count(const KeyType& key) const
{
    HashType  hash   = HasherType()(key);
    SizeType& id_ref = _bucketData(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(keyOf(data.data), key))
        {
            ++count;
        }
        id_ref = data.next;
    }
    return count;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::countHashed(const KeyType& key,
                                                                                                               HashType       hash) const
{
    SKR_Assert(HasherType()(key) == hash);

    SizeType& id_ref = _bucketData(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (ComparerType()(keyOf(data.data), key))
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
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::containAs(AsType&& v, AsHasher&& hasher, AsComparer&& comparer) const
{
    return (bool)findAs(std::forward<AsType>(v), std::forward<AsHasher>(hasher), std::forward<AsComparer>(comparer));
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename AsType, typename AsHasher, typename AsComparer>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::SizeType USet<T, TBitBlock, Config, Alloc>::countAs(AsType&& v, AsHasher&& hasher,
                                                                                                           AsComparer&& comparer) const
{
    HashType  hash   = hasher(v);
    SizeType& id_ref = _bucketData(hash);
    SizeType  count  = 0;
    while (id_ref != npos)
    {
        DataType& data = m_data[id_ref];
        if (comparer(keyOf(data.data), v))
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
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::sort(TP&& p)
{
    m_data.template sort([&](const DataType& a, const DataType& b) { return p(keyOf(a), keyOf(b)); });
    rehash();
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
template <typename TP>
SKR_INLINE void USet<T, TBitBlock, Config, Alloc>::sortStable(TP&& p)
{
    m_data.template sortStable([&](const DataType& a, const DataType& b) { return p(keyOf(a), keyOf(b)); });
    rehash();
}

// set ops
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc> USet<T, TBitBlock, Config, Alloc>::operator&(const USet& rhs) const
{
    bool        rhs_smaller = size() > rhs.size();
    const USet& a           = rhs_smaller ? rhs : *this;
    const USet& b           = rhs_smaller ? *this : rhs;

    USet result;
    result.reserve(a.size());

    for (const auto& v : a)
    {
        if (b.contain(keyOf(v)))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc> USet<T, TBitBlock, Config, Alloc>::operator|(const USet& rhs) const
{
    USet result(*this);
    for (const auto& v : rhs)
    {
        result.add(v);
    }
    return result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE USet<T, TBitBlock, Config, Alloc> USet<T, TBitBlock, Config, Alloc>::operator^(const USet& rhs) const
{
    USet result(size());

    for (const auto& v : *this)
    {
        if (!rhs.contain(keyOf(v)))
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
SKR_INLINE USet<T, TBitBlock, Config, Alloc> USet<T, TBitBlock, Config, Alloc>::operator-(const USet& rhs) const
{
    USet result(size());

    for (const auto& v : *this)
    {
        if (!rhs.contain(keyOf(v)))
        {
            result.add(v);
        }
    }

    return result;
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE bool USet<T, TBitBlock, Config, Alloc>::isSubsetOf(const USet& rhs) const
{
    if (rhs.size() <= size())
    {
        for (const auto& v : rhs)
        {
            if (!contain(keyOf(v)))
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
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::It USet<T, TBitBlock, Config, Alloc>::begin()
{
    return It(m_data.data(), m_data.sparseSize(), m_data.bitArray());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::It USet<T, TBitBlock, Config, Alloc>::end()
{
    return It(m_data.data(), m_data.sparseSize(), m_data.bitArray(), m_data.sparseSize());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::CIt USet<T, TBitBlock, Config, Alloc>::begin() const
{
    return CIt(m_data.data(), m_data.sparseSize(), m_data.bitArray());
}
template <typename T, typename TBitBlock, typename Config, typename Alloc>
SKR_INLINE typename USet<T, TBitBlock, Config, Alloc>::CIt USet<T, TBitBlock, Config, Alloc>::end() const
{
    return CIt(m_data.data(), m_data.sparseSize(), m_data.bitArray(), m_data.sparseSize());
}
} // namespace skr
