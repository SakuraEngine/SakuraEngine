#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "sparse_hash_set_iterator.hpp"
#include "SkrBase/containers/key_traits.hpp"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"

// SparseHashSet def
// set 在 add/emplace 时候从不覆盖既存元素，主要是 key 是元素的某个 Field 的情况比较少见，出现这种情况时，覆盖行为也需要用户自己关注，不应该 by default
// 除了 add 需要完整的元素方便添加操作外，其余的操作（find/remove/contain/count）均使用 key 进行操作以便在不构造完整元素的前提下进行查询
// xxx_as 是异构查询的便利函数，用于一些构造开销巨大的对象（比如使用字面量查询 string），更复杂的异构查找需要使用 xxx_ex，异构查找需要保证 hash 的求值方式一致
// add_ex_unsafe 是一个非常底层的 add 操作，它不会做任何构造行为，如果没有既存的查询元素，它会在申请空间后直接返回，在这种情况下，需要用户自行进行初始化和 add to bucket
// TODO. 移除 key_of、key_equal 等 API
// TODO. bucket 与碰撞统计，以及更好的 bucket 分配策略
// TODO. xxxx_as 依旧需要，除了异构查询之外，还有使用某个特定成员作为 key 的情况，这时候，我们会需要使用便利的异构查找
// TODO. compare 成本较小的情况下可以省去 hash 先行比较，可以通过 traits 实现
namespace skr
{
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>

struct SparseHashSet {
    // from config
    using HashType      = THash;
    using KeyType       = typename KeyTraits<T>::KeyType;
    using keyMapperType = typename KeyTraits<T>::KeyMapperType;
    using HasherType    = THasher;
    using ComparerType  = TComparer;

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

    // ctor & dtor
    SparseHashSet(Alloc alloc = {});
    SparseHashSet(SizeType reserve_size, Alloc alloc = {});
    SparseHashSet(const T* p, SizeType n, Alloc alloc = {});
    SparseHashSet(std::initializer_list<T> init_list, Alloc alloc = {});
    ~SparseHashSet();

    // copy & move
    SparseHashSet(const SparseHashSet& other, Alloc alloc = {});
    SparseHashSet(SparseHashSet&& other);

    // assign & move assign
    SparseHashSet& operator=(const SparseHashSet& rhs);
    SparseHashSet& operator=(SparseHashSet&& rhs);

    // compare
    bool operator==(const SparseHashSet& rhs) const;
    bool operator!=(const SparseHashSet& rhs) const;

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
    Alloc&         allocator();
    const Alloc&   allocator() const;

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
    DataRef add_ex_unsafe(HashType hash, Comparer&& comparer);

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
    template <typename TP = Less<KeyType>>
    void sort(TP&& p = {});
    template <typename TP = Less<KeyType>>
    void sort_stable(TP&& p = {});

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
    void     _add_to_bucket(const DataType& data, SizeType index);
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::_calc_bucket_size(SizeType data_size) const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::_bucket_index(SizeType hash) const
{
    return hash & _bucket_mask;
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::_clean_bucket() const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::_resize_bucket() const // resize bucket (nocopy)
{
    SizeType new_bucket_size = _calc_bucket_size(_data.capacity());

    if (new_bucket_size != _bucket_size)
    {
        if (new_bucket_size) // has new size
        {
            if constexpr (memory::memory_traits<SizeType>::use_realloc)
            {
                _bucket = _data.allocator().template realloc<SizeType>(_bucket, new_bucket_size);
            }
            else
            {
                // alloc new memory
                SizeType* new_memory = _data.allocator().template alloc<SizeType>(new_bucket_size);

                // move items
                if (_bucket_size)
                {
                    memory::move(new_memory, _bucket, _bucket_size);
                }

                // release old memory
                _data.allocator().free(_bucket);

                _bucket = new_memory;
            }

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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::_is_in_bucket(SizeType index) const
{
    if (has_data(index))
    {
        const auto& node         = _data[index];
        auto        search_index = _bucket[_bucket_index(node._sparse_hash_set_hash)];

        while (search_index != npos)
        {
            if (search_index == index)
            {
                return true;
            }
            search_index = _data[search_index]._sparse_hash_set_next;
        }
        return false;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::_add_to_bucket(const DataType& data, SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(!_bucket || !_is_in_bucket(index));

    if (!rehash_if_need())
    {
        SizeType& index_ref        = _bucket[_bucket_index(data._sparse_hash_set_hash)];
        data._sparse_hash_set_next = index_ref;
        index_ref                  = index;
    }
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::_remove_from_bucket(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(_is_in_bucket(index));

    DataType* pdata      = &_data[index];
    SizeType* pindex_ref = &_bucket[_bucket_index(pdata->_sparse_hash_set_hash)];

    while (*pindex_ref != npos)
    {
        if (*pindex_ref == index)
        {
            *pindex_ref = pdata->_sparse_hash_set_next;
            break;
        }
        pindex_ref = &_data[*pindex_ref]._sparse_hash_set_next;
        pdata      = &_data[*pindex_ref];
    }
}

// ctor & dtor
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashSet(Alloc alloc)
    : _data(std::move(alloc))
{
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashSet(SizeType reserve_size, Alloc alloc)
    : _data(std::move(alloc))
{
    reserve(reserve_size);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashSet(const T* p, SizeType n, Alloc alloc)
    : _data(std::move(alloc))
{
    append(p, n);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashSet(std::initializer_list<T> init_list, Alloc alloc)
    : _data(std::move(alloc))
{
    append(init_list);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::~SparseHashSet() { release(); }

// copy & move
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashSet(const SparseHashSet& other, Alloc alloc)
    : _data(std::move(alloc))
{
    *this = other;
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SparseHashSet(SparseHashSet&& other)
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator=(const SparseHashSet& rhs)
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator=(SparseHashSet&& rhs)
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator==(const SparseHashSet& rhs) const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator!=(const SparseHashSet& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::size() const
{
    return _data.size();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::capacity() const
{
    return _data.capacity();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::slack() const
{
    return _data.slack();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::sparse_size() const
{
    return _data.sparse_size();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::hole_size() const
{
    return _data.hole_size();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::bit_array_size() const
{
    return _data.bit_array_size();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::free_list_head() const
{
    return _data.free_list_head();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::bucket_size() const
{
    return _bucket_size;
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_compact() const
{
    return _data.is_compact();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::empty() const
{
    return _data.empty();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataArr& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::data_arr()
{
    return _data;
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
const typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataArr& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::data_arr() const
{
    return _data;
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
Alloc& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::allocator()
{
    return _data.allocator();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
const Alloc& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::allocator() const
{
    return _data.allocator();
}

// validate
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::has_data(SizeType idx) const
{
    return _data.has_data(idx);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_hole(SizeType idx) const
{
    return _data.is_hole(idx);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_valid_index(SizeType idx) const
{
    return _data.is_valid_index(idx);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_valid_pointer(const T* p) const
{
    return _data.is_valid_pointer(p);
}

// memory op
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::clear()
{
    _data.clear();
    _clean_bucket();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::release(SizeType capacity)
{
    _data.release(capacity);
    _resize_bucket();
    _clean_bucket();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::reserve(SizeType capacity)
{
    _data.reserve(capacity);
    rehash_if_need();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::shrink()
{
    _data.shrink();
    if (_resize_bucket())
    {
        rehash();
    }
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::compact()
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::compact_stable()
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::compact_top()
{
    return _data.compact_top();
}

// data op
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::KeyType& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::key_of(T& v) const
{
    return keyMapperType()(v);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE const typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::KeyType& SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::key_of(const T& v) const
{
    return keyMapperType()(v);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::key_equal(const T& a, const T& b) const
{
    return ComparerType()(key_of(a), key_of(b));
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::HashType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::hash_of(const T& v) const
{
    return HasherType()(key_of(v));
}

// rehash
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::need_rehash() const
{
    SizeType new_bucket_size = _calc_bucket_size(_data.capacity());
    return _data.size() > 0 && (new_bucket_size != _bucket_size);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::rehash() const
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
            SizeType& index_ref       = _bucket[_bucket_index(it->_sparse_hash_set_hash)];
            it->_sparse_hash_set_next = index_ref;
            index_ref                 = it.index();
        }
    }
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::rehash_if_need() const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add(const T& v)
{
    HashType hash = hash_of(v);
    return add_ex(
    hash,
    [&v, this](const KeyType& k) { return ComparerType()(k, key_of(v)); },
    [&v](void* p) { new (p) T(v); });
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add(T&& v)
{
    HashType hash = hash_of(v);
    return add_ex(
    hash,
    [&v, this](const KeyType& k) { return ComparerType()(k, key_of(v)); },
    [&v](void* p) { new (p) T(std::move(v)); });
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer, typename Constructor>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor)
{
    DataRef add_result = add_ex_unsafe(hash, std::forward<Comparer>(comparer));

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        constructor(add_result.data);
        SKR_ASSERT(_data[add_result.index]._sparse_hash_set_hash == hash_of(*add_result.data));
    }

    return add_result;
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::add_ex_unsafe(HashType hash, Comparer&& comparer)
{
    if constexpr (!AllowMultiKey)
    {
        DataRef ref = find_ex(hash, std::forward<Comparer>(comparer));
        if (ref)
        {
            ref.already_exist = true;
            return ref;
        }
        else
        {
            auto data_arr_ref                   = _data.add_unsafe();
            data_arr_ref->_sparse_hash_set_hash = hash;
            _add_to_bucket(*data_arr_ref, data_arr_ref.index);
            return { &data_arr_ref->_sparse_hash_set_data, data_arr_ref.index, false };
        }
    }
    else
    {
        auto data_arr_ref  = _data.add_ex_unsafe();
        data_arr_ref->hash = hash;
        _add_to_bucket(*data_arr_ref, data_arr_ref.index);
        return { &data_arr_ref->data, data_arr_ref.index, false };
    }
}

// emplace
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::emplace(Args&&... args)
{
    // emplace to data array
    auto data_arr_ref = _data.add_unsafe();
    new (&data_arr_ref->_sparse_hash_set_data) T(std::forward<Args>(args)...);
    data_arr_ref->_sparse_hash_set_hash = hash_of(data_arr_ref->_sparse_hash_set_data);

    if constexpr (!AllowMultiKey)
    {
        // check if data has been added to set
        if (DataRef found_info = find_ex(data_arr_ref->_sparse_hash_set_hash, [&data_arr_ref, this](const KeyType& k) { return ComparerType()(k, key_of(data_arr_ref->_sparse_hash_set_data)); }))
        {
            // remove new data
            _data.remove_at(data_arr_ref.index);

            // return old data
            found_info.already_exist = true;
            return found_info;
        }
    }

    _add_to_bucket(*data_arr_ref, data_arr_ref.index);

    return { &data_arr_ref->_sparse_hash_set_data, data_arr_ref.index, false };
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer, typename... Args>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::emplace_ex(HashType hash, Comparer&& comparer, Args&&... args)
{
    DataRef add_result = add_ex_unsafe(hash, comparer);

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        new (add_result.data) T(std::forward<Args>(args)...);
        SKR_ASSERT(_data[add_result.index]._sparse_hash_set_hash == hash_of(*add_result.data));
    }

    return add_result;
}

// append
// TODO. optimize for multimap
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::append(const SparseHashSet& set)
{
    // fill slack
    SizeType count = 0;
    auto     it    = set.begin();
    while (slack() > 0 && it != set.end())
    {
        add(*it);
        ++it;
        ++count;
    }

    // reserve and add
    if (it != set.end())
    {
        auto new_capacity = _data.capacity() + (set.size() - count);
        _data.reserve(new_capacity);

        while (it != set.end())
        {
            add(*it);
            ++it;
        }
    }
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::append(std::initializer_list<T> init_list)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < init_list.size())
    {
        const auto& v = init_list.begin()[read_idx];
        add(v);
        ++read_idx;
    }

    // reserve and add
    if (read_idx < init_list.size())
    {
        auto new_capacity = _data.capacity() + (init_list.size() - read_idx);
        _data.reserve(new_capacity);

        while (read_idx < init_list.size())
        {
            const auto& v = init_list.begin()[read_idx];
            add(v);
            ++read_idx;
        }
    }

    rehash();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::append(const T* p, SizeType n)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < n)
    {
        const auto& v = p[read_idx];
        add(v);
        ++read_idx;
    }

    // reserve and add
    if (read_idx < n)
    {
        auto new_capacity = _data.capacity() + (n - read_idx);
        _data.reserve(new_capacity);

        while (read_idx < n)
        {
            const auto& v = p[read_idx];
            add(v);
            ++read_idx;
        }
    }

    rehash();
}

// remove
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove(const KeyType& key)
{
    HashType hash = HasherType()(key);
    return remove_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove_all(const KeyType& key)
{
    HashType hash = HasherType()(key);
    remove_all_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove_ex(HashType hash, Comparer&& comparer)
{
    if (DataRef ref = find_ex(hash, std::forward<Comparer>(comparer)))
    {
        _remove_from_bucket(ref.index);
        _data.remove_at(ref.index);
        return ref;
    }
    return {};
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::remove_all_ex(HashType hash, Comparer&& comparer)
{
    SizeType search_index = _bucket_data(hash);
    SizeType count        = 0;
    while (search_index != npos)
    {
        DataType& data = _data[search_index];
        SizeType  next = data._sparse_hash_set_next;
        if (data._sparse_hash_set_hash == hash && comparer(key_of(data._sparse_hash_set_data)))
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find(const KeyType& key)
{
    HashType hash = HasherType()(key);
    return find_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CDataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find(const KeyType& key) const
{
    HashType hash = HasherType()(key);
    return find_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::DataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find_ex(HashType hash, Comparer&& comparer)
{
    if (!_bucket) return {};

    SizeType search_index = _bucket[_bucket_index(hash)];
    while (search_index != npos)
    {
        auto& node = _data[search_index];
        if (node._sparse_hash_set_hash == hash && comparer(key_of(node._sparse_hash_set_data)))
        {
            return { &node._sparse_hash_set_data, search_index };
        }
        search_index = node._sparse_hash_set_next;
    }
    return {};
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CDataRef SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::find_ex(HashType hash, Comparer&& comparer) const
{
    DataRef info = const_cast<SparseHashSet*>(this)->find_ex(hash, std::forward<Comparer>(comparer));
    return { info.data, info.index };
}

// contain
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::contain(const KeyType& key) const
{
    return (bool)find(key);
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::count(const KeyType& key) const
{
    HashType hash = HasherType()(key);
    return count_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::contain_ex(HashType hash, Comparer&& comparer) const
{
    return (bool)find_ex(hash, std::forward<Comparer>(comparer));
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::SizeType SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::count_ex(HashType hash, Comparer&& comparer) const
{
    SizeType& search_index = _bucket[_bucket_index(hash)];
    SizeType  count        = 0;

    while (search_index != npos)
    {
        DataType& data = _data[search_index];
        if (data._sparse_hash_set_hash == hash && comparer(key_of(data._sparse_hash_set_data)))
        {
            ++count;
        }
        search_index = data._sparse_hash_set_next;
    }
    return count;
}

// sort
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::sort(TP&& p)
{
    _data.sort([&](const DataType& a, const DataType& b) { return p(key_of(a._sparse_hash_set_data), key_of(b._sparse_hash_set_data)); });
    rehash();
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
template <typename TP>
SKR_INLINE void SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::sort_stable(TP&& p)
{
    _data.sort_stable([&](const DataType& a, const DataType& b) { return p(key_of(a._sparse_hash_set_data), key_of(b._sparse_hash_set_data)); });
    rehash();
}

// set ops
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator&(const SparseHashSet& rhs) const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator|(const SparseHashSet& rhs) const
{
    SparseHashSet result(*this);
    for (const auto& v : rhs)
    {
        result.add(v);
    }
    return result;
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator^(const SparseHashSet& rhs) const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc> SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::operator-(const SparseHashSet& rhs) const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE bool SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::is_sub_set_of(const SparseHashSet& rhs) const
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
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::It SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::begin()
{
    return It(_data.data(), _data.sparse_size(), _data.bit_array());
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::It SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::end()
{
    return It(_data.data(), _data.sparse_size(), _data.bit_array(), _data.sparse_size());
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CIt SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::begin() const
{
    return CIt(_data.data(), _data.sparse_size(), _data.bit_array());
}
template <typename T, typename TBitBlock, typename THash, typename THasher, typename TComparer, bool AllowMultiKey, typename Alloc>
SKR_INLINE typename SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::CIt SparseHashSet<T, TBitBlock, THash, THasher, TComparer, AllowMultiKey, Alloc>::end() const
{
    return CIt(_data.data(), _data.sparse_size(), _data.bit_array(), _data.sparse_size());
}
} // namespace skr
