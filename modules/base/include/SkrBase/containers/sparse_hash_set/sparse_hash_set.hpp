#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/sparse_hash_set/sparse_hash_set_def.hpp"
#include "sparse_hash_set_iterator.hpp"
#include "SkrBase/containers/key_traits.hpp"
#include "SkrBase/containers/sparse_array/sparse_array.hpp"

// SparseHashSet def
// set 在 add/emplace 时候从不覆盖既存元素，主要是 key 是元素的某个 Field 的情况比较少见，出现这种情况时，覆盖行为也需要用户自己关注，不应该 by default
// 除了 add 需要完整的元素方便添加操作外，其余的操作（find/remove/contains/count）均使用 key 进行操作以便在不构造完整元素的前提下进行查询
// xxx_as 是异构查询的便利函数，用于一些构造开销巨大的对象（比如使用字面量查询 string），更复杂的异构查找需要使用 xxx_ex，异构查找需要保证 hash 的求值方式一致
// find_or_add_ex_unsafe 是一个非常底层的 add 操作，它不会做任何构造行为，如果没有既存的查询元素，它会在申请空间后直接返回，在这种情况下，需要用户自行进行初始化和 add to bucket
// TODO. rehash when copy/move
namespace skr::container
{
template <typename Memory>
struct SparseHashSet : protected SparseArray<Memory> {
    using Super = SparseArray<Memory>;

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
    using typename Memory::ComparerType;
    using typename Memory::SetDataType;
    using typename Memory::SetStorageType;
    using Memory::allow_multi_key;

    // helper
    using DataArr                         = SparseArray<Memory>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref & iterator
    using DataRef  = SparseHashSetDataRef<SetDataType, SizeType>;
    using CDataRef = SparseHashSetDataRef<const SetDataType, SizeType>;
    using It       = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, false>;
    using CIt      = SparseHashSetIt<SetDataType, BitBlockType, SizeType, HashType, true>;

    // ctor & dtor
    SparseHashSet(AllocatorCtorParam param = {});
    SparseHashSet(SizeType reserve_size, AllocatorCtorParam param = {});
    SparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param = {});
    SparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param = {});
    ~SparseHashSet();

    // copy & move
    SparseHashSet(const SparseHashSet& other, AllocatorCtorParam param = {});
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
    bool            is_compact() const;
    bool            empty() const;
    DataArr&        data_arr();
    const DataArr&  data_arr() const;
    SizeType*       bucket();
    const SizeType* bucket() const;
    Memory&         memory();
    const Memory&   memory() const;

    // validate
    bool has_data(SizeType idx) const;
    bool is_hole(SizeType idx) const;
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const SetDataType* p) const;

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void shrink();
    bool compact();
    bool compact_stable();
    bool compact_top();

    // data op
    KeyType&       key_of(SetDataType& v) const;
    const KeyType& key_of(const SetDataType& v) const;
    bool           key_equal(const SetDataType& a, const SetDataType& b) const;
    HashType       hash_of(const SetDataType& v) const;

    // rehash
    bool need_rehash() const;
    void rehash();
    bool rehash_if_need();

    //  try to add (first check existence, then add, never assign)
    DataRef find_or_add(const SetDataType& v);
    DataRef find_or_add(SetDataType&& v); // move behavior may not happened here, just for easy to use
    template <typename Comparer, typename Constructor>
    DataRef find_or_add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor);
    template <typename Comparer>
    DataRef find_or_add_ex_unsafe(HashType hash, Comparer&& comparer);

    // add or assign (first check existence, then add or assign)
    DataRef add_or_assign(const SetDataType& v);
    DataRef add_or_assign(SetDataType&& v);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args); // first construct, then add or destroy
    template <typename Comparer, typename... Args>
    DataRef emplace_ex(HashType hash, Comparer&& comparer, Args&&... args); // first check existence, if not exist, then add

    // append
    void append(const SparseHashSet& set);
    void append(std::initializer_list<SetDataType> init_list);
    void append(const SetDataType* p, SizeType n);

    // remove
    DataRef  remove(const KeyType& key);
    SizeType remove_all(const KeyType& key); // [multi set extend]
    template <typename Comparer>
    DataRef remove_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    SizeType remove_all_ex(HashType hash, Comparer&& comparer); // [multi set extend]

    // erase, needn't update iterator, erase directly is safe
    It  erase(const It& it);
    CIt erase(const CIt& it);

    // find
    DataRef  find(const KeyType& key);
    CDataRef find(const KeyType& key) const;
    template <typename Comparer>
    DataRef find_ex(HashType hash, Comparer&& comparer);
    template <typename Comparer>
    CDataRef find_ex(HashType hash, Comparer&& comparer) const;

    // contains
    bool     contains(const KeyType& key) const;
    SizeType count(const KeyType& key) const; // [multi set extend]
    template <typename Comparer>
    bool contains_ex(HashType hash, Comparer&& comparer) const;
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
    SizeType _bucket_index(SizeType hash) const; // get bucket data index by hash
    void     _clean_bucket();                    // remove all elements from bucket
    bool     _resize_bucket();                   // resize hash bucket
    bool     _is_in_bucket(SizeType index) const;
    void     _add_to_bucket(const SetStorageType& data, SizeType index);
    void     _remove_from_bucket(SizeType index);
};
} // namespace skr::container

// SparseHashSet impl
namespace skr::container
{
// helpers
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::_bucket_index(SizeType hash) const
{
    return Memory::bucket_index(hash);
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::_clean_bucket()
{
    Memory::clean_bucket();
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::_resize_bucket() // resize bucket (nocopy)
{
    return Memory::resize_bucket();
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::_is_in_bucket(SizeType index) const
{
    if (has_data(index))
    {
        const auto& node         = data_arr()[index];
        auto        search_index = bucket()[_bucket_index(node._sparse_hash_set_hash)];

        while (search_index != npos)
        {
            if (search_index == index)
            {
                return true;
            }
            search_index = data_arr()[search_index]._sparse_hash_set_next;
        }
        return false;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::_add_to_bucket(const SetStorageType& data, SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(!bucket() || !_is_in_bucket(index));

    if (!rehash_if_need())
    {
        SizeType& index_ref        = bucket()[_bucket_index(data._sparse_hash_set_hash)];
        data._sparse_hash_set_next = index_ref;
        index_ref                  = index;
    }
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::_remove_from_bucket(SizeType index)
{
    SKR_ASSERT(has_data(index));
    SKR_ASSERT(_is_in_bucket(index));

    SetStorageType* pdata      = &data_arr()[index];
    SizeType*       pindex_ref = &bucket()[_bucket_index(pdata->_sparse_hash_set_hash)];

    while (*pindex_ref != npos)
    {
        if (*pindex_ref == index)
        {
            *pindex_ref = pdata->_sparse_hash_set_next;
            break;
        }
        pindex_ref = &data_arr()[*pindex_ref]._sparse_hash_set_next;
        pdata      = &data_arr()[*pindex_ref];
    }
}

// ctor & dtor
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(AllocatorCtorParam param)
    : Super(std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(SizeType reserve_size, AllocatorCtorParam param)
    : Super(std::move(param))
{
    reserve(reserve_size);
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(const SetDataType* p, SizeType n, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(p, n);
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(std::initializer_list<SetDataType> init_list, AllocatorCtorParam param)
    : Super(std::move(param))
{
    append(init_list);
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::~SparseHashSet()
{
    // handled in memory
}

// copy & move
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(const SparseHashSet& other, AllocatorCtorParam param)
    : Super(other, std::move(param))
{
    // handled in memory
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>::SparseHashSet(SparseHashSet&& other)
    : Super(std::move(other))
{
    // handled in memory
}

// assign & move assign
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>& SparseHashSet<Memory>::operator=(const SparseHashSet& rhs)
{
    Super::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory>& SparseHashSet<Memory>::operator=(SparseHashSet&& rhs)
{
    Super::operator=(std::move(rhs));
    return *this;
}

// compare
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::operator==(const SparseHashSet& rhs) const
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
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::operator!=(const SparseHashSet& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename Memory>
typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::size() const
{
    return data_arr().size();
}
template <typename Memory>
typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::capacity() const
{
    return data_arr().capacity();
}
template <typename Memory>
typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::slack() const
{
    return data_arr().slack();
}
template <typename Memory>
typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::sparse_size() const
{
    return data_arr().sparse_size();
}
template <typename Memory>
typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::hole_size() const
{
    return data_arr().hole_size();
}
template <typename Memory>
typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::bit_array_size() const
{
    return data_arr().bit_array_size();
}
template <typename Memory>
typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::free_list_head() const
{
    return data_arr().free_list_head();
}
template <typename Memory>
bool SparseHashSet<Memory>::is_compact() const
{
    return data_arr().is_compact();
}
template <typename Memory>
bool SparseHashSet<Memory>::empty() const
{
    return data_arr().empty();
}
template <typename Memory>
typename SparseHashSet<Memory>::DataArr& SparseHashSet<Memory>::data_arr()
{
    return *this;
}
template <typename Memory>
const typename SparseHashSet<Memory>::DataArr& SparseHashSet<Memory>::data_arr() const
{
    return *this;
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::SizeType* SparseHashSet<Memory>::bucket()
{
    return Memory::bucket();
}
template <typename Memory>
SKR_INLINE const typename SparseHashSet<Memory>::SizeType* SparseHashSet<Memory>::bucket() const
{
    return Memory::bucket();
}

template <typename Memory>
SKR_INLINE Memory& SparseHashSet<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& SparseHashSet<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::has_data(SizeType idx) const
{
    return data_arr().has_data(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::is_hole(SizeType idx) const
{
    return data_arr().is_hole(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::is_valid_index(SizeType idx) const
{
    return data_arr().is_valid_index(idx);
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::is_valid_pointer(const SetDataType* p) const
{
    return data_arr().is_valid_pointer(p);
}

// memory op
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::clear()
{
    data_arr().clear();
    _clean_bucket();
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::release(SizeType capacity)
{
    data_arr().release(capacity);
    _resize_bucket();
    _clean_bucket();
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::reserve(SizeType capacity)
{
    data_arr().reserve(capacity);
    rehash_if_need();
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::shrink()
{
    data_arr().shrink();
    if (_resize_bucket())
    {
        rehash();
    }
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::compact()
{
    if (data_arr().compact())
    {
        rehash();
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::compact_stable()
{
    if (data_arr().compact_stable())
    {
        rehash();
        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::compact_top()
{
    return data_arr().compact_top();
}

// data op
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::KeyType& SparseHashSet<Memory>::key_of(SetDataType& v) const
{
    return KeyMapperType()(v);
}
template <typename Memory>
SKR_INLINE const typename SparseHashSet<Memory>::KeyType& SparseHashSet<Memory>::key_of(const SetDataType& v) const
{
    return KeyMapperType()(v);
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::key_equal(const SetDataType& a, const SetDataType& b) const
{
    return ComparerType()(key_of(a), key_of(b));
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::HashType SparseHashSet<Memory>::hash_of(const SetDataType& v) const
{
    return HasherType()(key_of(v));
}

// rehash
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::need_rehash() const
{
    return Memory::need_rehash();
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::rehash()
{
    // resize bucket
    _resize_bucket();

    // rehash
    if (bucket())
    {
        _clean_bucket();
        for (auto it = data_arr().begin(); it; ++it)
        {
            // link to head
            SizeType& index_ref       = bucket()[_bucket_index(it->_sparse_hash_set_hash)];
            it->_sparse_hash_set_next = index_ref;
            index_ref                 = it.index();
        }
    }
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::rehash_if_need()
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
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_or_add(const SetDataType& v)
{
    HashType hash = hash_of(v);
    return find_or_add_ex(
    hash,
    [&v, this](const KeyType& k) { return ComparerType()(k, key_of(v)); },
    [&v](void* p) { new (p) SetDataType(v); });
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_or_add(SetDataType&& v)
{
    HashType hash = hash_of(v);
    return find_or_add_ex(
    hash,
    [&v, this](const KeyType& k) { return ComparerType()(k, key_of(v)); },
    [&v](void* p) { new (p) SetDataType(std::move(v)); });
}
template <typename Memory>
template <typename Comparer, typename Constructor>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_or_add_ex(HashType hash, Comparer&& comparer, Constructor&& constructor)
{
    DataRef add_result = find_or_add_ex_unsafe(hash, std::forward<Comparer>(comparer));

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        constructor(add_result.data);
        SKR_ASSERT(data_arr()[add_result.index]._sparse_hash_set_hash == hash_of(*add_result.data));
    }

    return add_result;
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_or_add_ex_unsafe(HashType hash, Comparer&& comparer)
{
    if constexpr (!allow_multi_key)
    {
        DataRef ref = find_ex(hash, std::forward<Comparer>(comparer));
        if (ref)
        {
            ref.already_exist = true;
            return ref;
        }
        else
        {
            auto data_arr_ref                   = data_arr().add_unsafe();
            data_arr_ref->_sparse_hash_set_hash = hash;
            _add_to_bucket(*data_arr_ref, data_arr_ref.index);
            return { &data_arr_ref->_sparse_hash_set_data, data_arr_ref.index, false };
        }
    }
    else
    {
        auto data_arr_ref                   = data_arr().add_unsafe();
        data_arr_ref->_sparse_hash_set_hash = hash;
        _add_to_bucket(*data_arr_ref, data_arr_ref.index);
        return { &data_arr_ref->_sparse_hash_set_data, data_arr_ref.index, false };
    }
}

// add or assign (first check existence, then add or assign)
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::add_or_assign(const SetDataType& v)
{
    HashType hash       = hash_of(v);
    DataRef  add_result = find_or_add_ex_unsafe(hash, [this, &v](const KeyType& k) { return ComparerType()(k, key_of(v)); });

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        new (add_result.data) SetDataType(v);
        SKR_ASSERT(data_arr()[add_result.index]._sparse_hash_set_hash == hash_of(*add_result.data));
    }
    else // else assign it
    {
        *add_result.data = v;
    }

    return add_result;
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::add_or_assign(SetDataType&& v)
{
    HashType hash       = hash_of(v);
    DataRef  add_result = find_or_add_ex_unsafe(hash, [this, &v](const KeyType& k) { return ComparerType()(k, key_of(v)); });

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        new (add_result.data) SetDataType(std::move(v));
        SKR_ASSERT(data_arr()[add_result.index]._sparse_hash_set_hash == hash_of(*add_result.data));
    }
    else // else assign it
    {
        *add_result.data = std::move(v);
    }

    return add_result;
}

// emplace
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::emplace(Args&&... args)
{
    // emplace to data array
    auto data_arr_ref = data_arr().add_unsafe();
    new (&data_arr_ref->_sparse_hash_set_data) SetDataType(std::forward<Args>(args)...);
    data_arr_ref->_sparse_hash_set_hash = hash_of(data_arr_ref->_sparse_hash_set_data);

    if constexpr (!allow_multi_key)
    {
        // check if data has been added to set
        if (DataRef found_info = find_ex(data_arr_ref->_sparse_hash_set_hash, [&data_arr_ref, this](const KeyType& k) { return ComparerType()(k, key_of(data_arr_ref->_sparse_hash_set_data)); }))
        {
            // remove new data
            data_arr().remove_at(data_arr_ref.index);

            // return old data
            found_info.already_exist = true;
            return found_info;
        }
    }

    _add_to_bucket(*data_arr_ref, data_arr_ref.index);

    return { &data_arr_ref->_sparse_hash_set_data, data_arr_ref.index, false };
}
template <typename Memory>
template <typename Comparer, typename... Args>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::emplace_ex(HashType hash, Comparer&& comparer, Args&&... args)
{
    DataRef add_result = find_or_add_ex_unsafe(hash, comparer);

    // if not exist, construct it
    if (!add_result.already_exist)
    {
        new (add_result.data) SetDataType(std::forward<Args>(args)...);
        SKR_ASSERT(data_arr()[add_result.index]._sparse_hash_set_hash == hash_of(*add_result.data));
    }

    return add_result;
}

// append
// TODO. optimize for multimap
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::append(const SparseHashSet& set)
{
    // fill slack
    SizeType count = 0;
    auto     it    = set.begin();
    while (slack() > 0 && it != set.end())
    {
        find_or_add(*it);
        ++it;
        ++count;
    }

    // reserve and add
    if (it != set.end())
    {
        auto new_capacity = data_arr().capacity() + (set.size() - count);
        data_arr().reserve(new_capacity);

        while (it != set.end())
        {
            find_or_add(*it);
            ++it;
        }
    }
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::append(std::initializer_list<SetDataType> init_list)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < init_list.size())
    {
        const auto& v = init_list.begin()[read_idx];
        find_or_add(v);
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
            find_or_add(v);
            ++read_idx;
        }
    }

    rehash();
}
template <typename Memory>
SKR_INLINE void SparseHashSet<Memory>::append(const SetDataType* p, SizeType n)
{
    // fill slack
    SizeType read_idx = 0;
    while (slack() > 0 && read_idx < n)
    {
        const auto& v = p[read_idx];
        find_or_add(v);
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
            find_or_add(v);
            ++read_idx;
        }
    }

    rehash();
}

// remove
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::remove(const KeyType& key)
{
    HashType hash = HasherType()(key);
    return remove_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::remove_all(const KeyType& key)
{
    HashType hash = HasherType()(key);
    remove_all_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::remove_ex(HashType hash, Comparer&& comparer)
{
    if (DataRef ref = find_ex(hash, std::forward<Comparer>(comparer)))
    {
        _remove_from_bucket(ref.index);
        data_arr().remove_at(ref.index);
        return ref;
    }
    return {};
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::remove_all_ex(HashType hash, Comparer&& comparer)
{
    SizeType search_index = _bucket_data(hash);
    SizeType count        = 0;
    while (search_index != npos)
    {
        SetStorageType& data = data_arr()[search_index];
        SizeType        next = data._sparse_hash_set_next;
        if (data._sparse_hash_set_hash == hash && comparer(key_of(data._sparse_hash_set_data)))
        {
            _remove_from_bucket(search_index);
            data_arr().remove_at(search_index);
            ++count;
        }
        search_index = next;
    }
    return count;
}

// erase, needn't update iterator, erase directly is safe
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::It SparseHashSet<Memory>::erase(const It& it)
{
    _remove_from_bucket(it.index());
    data_arr().remove_at(it.index());
    It new_it(it);
    ++new_it;
    return new_it;
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CIt SparseHashSet<Memory>::erase(const CIt& it)
{
    _remove_from_bucket(it.index());
    data_arr().remove_at(it.index());
    CIt new_it(it);
    ++new_it;
    return new_it;
}

// find
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find(const KeyType& key)
{
    HashType hash = HasherType()(key);
    return find_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find(const KeyType& key) const
{
    HashType hash = HasherType()(key);
    return find_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<Memory>::DataRef SparseHashSet<Memory>::find_ex(HashType hash, Comparer&& comparer)
{
    if (!bucket()) return {};

    SizeType search_index = bucket()[_bucket_index(hash)];
    while (search_index != npos)
    {
        auto& node = data_arr()[search_index];
        if (node._sparse_hash_set_hash == hash && comparer(key_of(node._sparse_hash_set_data)))
        {
            return { &node._sparse_hash_set_data, search_index };
        }
        search_index = node._sparse_hash_set_next;
    }
    return {};
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<Memory>::CDataRef SparseHashSet<Memory>::find_ex(HashType hash, Comparer&& comparer) const
{
    DataRef info = const_cast<SparseHashSet*>(this)->find_ex(hash, std::forward<Comparer>(comparer));
    return { info.data, info.index };
}

// contains
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::contains(const KeyType& key) const
{
    return (bool)find(key);
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::count(const KeyType& key) const
{
    HashType hash = HasherType()(key);
    return count_ex(hash, [&key](const KeyType& k) { return ComparerType()(key, k); });
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE bool SparseHashSet<Memory>::contains_ex(HashType hash, Comparer&& comparer) const
{
    return (bool)find_ex(hash, std::forward<Comparer>(comparer));
}
template <typename Memory>
template <typename Comparer>
SKR_INLINE typename SparseHashSet<Memory>::SizeType SparseHashSet<Memory>::count_ex(HashType hash, Comparer&& comparer) const
{
    SizeType& search_index = bucket()[_bucket_index(hash)];
    SizeType  count        = 0;

    while (search_index != npos)
    {
        SetStorageType& data = data_arr()[search_index];
        if (data._sparse_hash_set_hash == hash && comparer(key_of(data._sparse_hash_set_data)))
        {
            ++count;
        }
        search_index = data._sparse_hash_set_next;
    }
    return count;
}

// sort
template <typename Memory>
template <typename TP>
SKR_INLINE void SparseHashSet<Memory>::sort(TP&& p)
{
    data_arr().sort([&](const SetStorageType& a, const SetStorageType& b) {
        return p(key_of(a._sparse_hash_set_data), key_of(b._sparse_hash_set_data));
    });
    rehash();
}
template <typename Memory>
template <typename TP>
SKR_INLINE void SparseHashSet<Memory>::sort_stable(TP&& p)
{
    data_arr().sort_stable([&](const SetStorageType& a, const SetStorageType& b) {
        return p(key_of(a._sparse_hash_set_data), key_of(b._sparse_hash_set_data));
    });
    rehash();
}

// set ops
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator&(const SparseHashSet& rhs) const
{
    bool                 rhs_smaller = size() > rhs.size();
    const SparseHashSet& a           = rhs_smaller ? rhs : *this;
    const SparseHashSet& b           = rhs_smaller ? *this : rhs;

    SparseHashSet result;
    result.reserve(a.size());

    for (const auto& v : a)
    {
        if (b.contains(key_of(v)))
        {
            result.find_or_add(v);
        }
    }

    return result;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator|(const SparseHashSet& rhs) const
{
    SparseHashSet result(*this);
    for (const auto& v : rhs)
    {
        result.find_or_add(v);
    }
    return result;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator^(const SparseHashSet& rhs) const
{
    SparseHashSet result(size());

    for (const auto& v : *this)
    {
        if (!rhs.contains(key_of(v)))
        {
            result.find_or_add(v);
        }
    }

    for (const auto& v : rhs)
    {
        if (!contains(v))
        {
            result.find_or_add(v);
        }
    }

    return result;
}
template <typename Memory>
SKR_INLINE SparseHashSet<Memory> SparseHashSet<Memory>::operator-(const SparseHashSet& rhs) const
{
    SparseHashSet result(size());

    for (const auto& v : *this)
    {
        if (!rhs.contains(key_of(v)))
        {
            result.find_or_add(v);
        }
    }

    return result;
}
template <typename Memory>
SKR_INLINE bool SparseHashSet<Memory>::is_sub_set_of(const SparseHashSet& rhs) const
{
    if (rhs.size() >= size())
    {
        for (const auto& v : *this)
        {
            if (!rhs.contains(key_of(v)))
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
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::It SparseHashSet<Memory>::begin()
{
    return It(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::It SparseHashSet<Memory>::end()
{
    return It(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CIt SparseHashSet<Memory>::begin() const
{
    return CIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array());
}
template <typename Memory>
SKR_INLINE typename SparseHashSet<Memory>::CIt SparseHashSet<Memory>::end() const
{
    return CIt(data_arr().data(), data_arr().sparse_size(), data_arr().bit_array(), data_arr().sparse_size());
}
} // namespace skr::container
