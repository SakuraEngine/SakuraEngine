#pragma once
#include "SkrBase/config.h"
#include "SkrBase/memory/memory_ops.hpp"
#include "sparse_array_def.hpp"
#include "sparse_array_iterator.hpp"
#include "SkrBase/algo/intro_sort.hpp"
#include "SkrBase/algo/merge_sort.hpp"

// SparseArray def
namespace skr::container
{
template <typename Memory>
struct SparseArray : protected Memory {
    // from core
    using typename Memory::SizeType;
    using typename Memory::DataType;
    using typename Memory::StorageType;
    using typename Memory::BitBlockType;
    using typename Memory::AllocatorCtorParam;
    using BitAlgo                         = algo::BitAlgo<BitBlockType>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // data ref & iterator
    using DataRef  = SparseArrayDataRef<DataType, SizeType, false>;
    using CDataRef = SparseArrayDataRef<DataType, SizeType, true>;
    using StlIt    = SparseArrayIt<DataType, BitBlockType, SizeType, false>;
    using CStlIt   = SparseArrayIt<DataType, BitBlockType, SizeType, true>;

    // ctor & dtor
    SparseArray(AllocatorCtorParam param = {});
    SparseArray(SizeType size, AllocatorCtorParam param = {});
    SparseArray(SizeType size, const DataType& v, AllocatorCtorParam param = {});
    SparseArray(const DataType* p, SizeType n, AllocatorCtorParam param = {});
    SparseArray(std::initializer_list<DataType> init_list, AllocatorCtorParam param = {});
    ~SparseArray();

    // copy & move
    SparseArray(const SparseArray& other);
    SparseArray(SparseArray&& other) noexcept;

    // assign & move assign
    SparseArray& operator=(const SparseArray& rhs);
    SparseArray& operator=(SparseArray&& rhs) noexcept;

    // special assign
    void assign(const DataType* p, SizeType n);
    void assign(std::initializer_list<DataType> init_list);

    // compare
    bool operator==(const SparseArray& rhs) const;
    bool operator!=(const SparseArray& rhs) const;

    // getter
    SizeType            size() const;
    SizeType            capacity() const;
    SizeType            slack() const;
    SizeType            sparse_size() const;
    SizeType            hole_size() const;
    SizeType            bit_array_size() const;
    SizeType            freelist_head() const;
    bool                is_compact() const;
    bool                empty() const;
    StorageType*        data();
    const StorageType*  data() const;
    BitBlockType*       bit_array();
    const BitBlockType* bit_array() const;
    Memory&             memory();
    const Memory&       memory() const;

    // validate
    bool has_data(SizeType idx) const;
    bool is_hole(SizeType idx) const;
    bool is_valid_index(SizeType idx) const;
    bool is_valid_pointer(const DataType* p) const;

    // memory op
    void clear();
    void release(SizeType reserve_capacity = 0);
    void reserve(SizeType expect_capacity);
    void shrink();
    bool compact();
    bool compact_stable();
    bool compact_top();

    // add
    DataRef add(const DataType& v);
    DataRef add(DataType&& v);
    DataRef add_unsafe();
    DataRef add_default();
    DataRef add_zeroed();

    // add at
    void add_at(SizeType idx, const DataType& v);
    void add_at(SizeType idx, DataType&& v);
    void add_at_unsafe(SizeType idx);
    void add_at_default(SizeType idx);
    void add_at_zeroed(SizeType idx);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename... Args>
    void emplace_at(SizeType index, Args&&... args);

    // append
    void append(const SparseArray& arr);
    void append(std::initializer_list<DataType> init_list);
    void append(DataType* p, SizeType n);

    // remove
    void remove_at(SizeType index, SizeType n = 1);
    void remove_at_unsafe(SizeType index, SizeType n = 1);
    template <typename U = DataType>
    bool remove(const U& v);
    template <typename U = DataType>
    bool remove_last(const U& v);
    template <typename U = DataType>
    SizeType remove_all(const U& v);

    // erase, needn't update iterator, erase directly is safe
    StlIt  erase(const StlIt& it);
    CStlIt erase(const CStlIt& it);

    // remove if
    template <typename Pred>
    bool remove_if(Pred&& pred);
    template <typename Pred>
    bool remove_last_if(Pred&& pred);
    template <typename Pred>
    SizeType remove_all_if(Pred&& pred);

    // modify
    DataType&       operator[](SizeType index);
    const DataType& operator[](SizeType index) const;
    DataType&       at(SizeType index);
    const DataType& at(SizeType index) const;
    DataType&       last(SizeType index = 0);
    const DataType& last(SizeType index = 0) const;

    // find
    template <typename U = DataType>
    DataRef find(const U& v);
    template <typename U = DataType>
    DataRef find_last(const U& v);
    template <typename U = DataType>
    CDataRef find(const U& v) const;
    template <typename U = DataType>
    CDataRef find_last(const U& v) const;

    // find if
    template <typename Pred>
    DataRef find_if(Pred&& pred);
    template <typename Pred>
    DataRef find_last_if(Pred&& pred);
    template <typename Pred>
    CDataRef find_if(Pred&& pred) const;
    template <typename Pred>
    CDataRef find_last_if(Pred&& pred) const;

    // contains
    template <typename U = DataType>
    bool contains(const U& v) const;
    template <typename Pred>
    bool contains_if(Pred&& pred) const;
    template <typename U = DataType>
    SizeType count(const U& v) const;
    template <typename Pred>
    SizeType count_if(Pred&& pred) const;

    // sort
    template <typename Functor = Less<DataType>>
    void sort(Functor&& f = Functor());
    template <typename Functor = Less<DataType>>
    void sort_stable(Functor&& f = Functor());

    // support foreach
    StlIt  begin();
    StlIt  end();
    CStlIt begin() const;
    CStlIt end() const;

private:
    // helper
    void     _realloc(SizeType new_capacity);
    void     _free();
    SizeType _grow(SizeType grow_size);
    void     _set_sparse_size(SizeType value);
    void     _set_freelist_head(SizeType value);
    void     _set_hole_size(SizeType value);
    void     _copy_compacted_data(StorageType* dst, const DataType* src, SizeType size);
    void     _break_freelist_at(SizeType index);
};
} // namespace skr::container

// SparseArray impl
namespace skr::container
{
// helper
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::_realloc(SizeType new_capacity)
{
    Memory::realloc(new_capacity);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::_free()
{
    Memory::free();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::_grow(SizeType grow_size)
{
    return Memory::grow(grow_size);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::_set_sparse_size(SizeType value)
{
    Memory::set_sparse_size(value);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::_set_freelist_head(SizeType value)
{
    Memory::set_freelist_head(value);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::_set_hole_size(SizeType value)
{
    Memory::set_hole_size(value);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::_copy_compacted_data(StorageType* dst, const DataType* src, SizeType size)
{
    if constexpr (!memory::MemoryTraits<DataType>::use_copy && sizeof(DataType) == sizeof(StorageType))
    {
        std::memcpy(dst, src, sizeof(StorageType) * size);
    }
    else
    {
        for (SizeType i = 0; i < size; ++i)
        {
            new (&dst[i]._sparse_array_data) DataType(src[i]);
        }
    }
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::_break_freelist_at(SizeType index)
{
    StorageType* p_node = data() + index;

    if (freelist_head() == index)
    {
        _set_freelist_head(p_node->_sparse_array_freelist_next);
    }
    if (p_node->_sparse_array_freelist_next != npos)
    {
        data()[p_node->_sparse_array_freelist_next]._sparse_array_freelist_prev = p_node->_sparse_array_freelist_prev;
    }
    if (p_node->_sparse_array_freelist_prev != npos)
    {
        data()[p_node->_sparse_array_freelist_prev]._sparse_array_freelist_next = p_node->_sparse_array_freelist_next;
    }
}

// ctor & dtor
template <typename Memory>
SKR_INLINE SparseArray<Memory>::SparseArray(AllocatorCtorParam param)
    : Memory(std::move(param))
{
}
template <typename Memory>
SKR_INLINE SparseArray<Memory>::SparseArray(SizeType size, AllocatorCtorParam param)
    : Memory(std::move(param))
{
    if (size)
    {
        // realloc
        _realloc(size);

        // setup size
        _set_sparse_size(size);
        BitAlgo::set_range(bit_array(), SizeType(0), size, true);

        // call ctor (stl ub)
        if constexpr (memory::MemoryTraits<DataType>::use_ctor)
        {
            for (SizeType i = 0; i < size; ++i)
            {
                new (&data()[i]._sparse_array_data) DataType();
            }
        }
        else
        {
            std::memset(data(), 0, sizeof(StorageType) * size);
        }
    }
}
template <typename Memory>
SKR_INLINE SparseArray<Memory>::SparseArray(SizeType size, const DataType& v, AllocatorCtorParam param)
    : Memory(std::move(param))
{
    if (size)
    {
        // realloc
        _realloc(size);

        // setup size
        _set_sparse_size(size);
        BitAlgo::set_range(bit_array(), SizeType(0), size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&data()[i]._sparse_array_data) DataType(v);
        }
    }
}
template <typename Memory>
SKR_INLINE SparseArray<Memory>::SparseArray(const DataType* p, SizeType n, AllocatorCtorParam param)
    : Memory(std::move(param))
{
    if (n)
    {
        // realloc
        _realloc(n);

        // setup size
        _set_sparse_size(n);
        BitAlgo::set_range(bit_array(), SizeType(0), n, true);

        // call ctor
        _copy_compacted_data(data(), p, n);
    }
}
template <typename Memory>
SKR_INLINE SparseArray<Memory>::SparseArray(std::initializer_list<DataType> init_list, AllocatorCtorParam param)
    : Memory(std::move(param))
{
    SizeType size = init_list.size();
    if (size)
    {
        // realloc
        _realloc(size);

        // setup size
        _set_sparse_size(size);
        BitAlgo::set_range(bit_array(), SizeType(0), size, true);

        // call ctor
        _copy_compacted_data(data(), init_list.begin(), size);
    }
}
template <typename Memory>
SKR_INLINE SparseArray<Memory>::~SparseArray()
{
    // handled in memory
}

// copy & move
template <typename Memory>
SKR_INLINE SparseArray<Memory>::SparseArray(const SparseArray& other)
    : Memory(other)
{
    // handled in memory
}
template <typename Memory>
SKR_INLINE SparseArray<Memory>::SparseArray(SparseArray&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// assign & move assign
template <typename Memory>
SKR_INLINE SparseArray<Memory>& SparseArray<Memory>::operator=(const SparseArray& rhs)
{
    Memory::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE SparseArray<Memory>& SparseArray<Memory>::operator=(SparseArray&& rhs) noexcept
{
    Memory::operator=(std::move(rhs));
    return *this;
}

// special assign
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::assign(const DataType* p, SizeType n)
{
    clear();

    if (n)
    {
        // reserve
        reserve(n);

        // setup size
        _set_sparse_size(n);
        BitAlgo::set_range(bit_array(), SizeType(0), n, true);

        // call ctor
        _copy_compacted_data(data(), p, n);
    }
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::assign(std::initializer_list<DataType> init_list)
{
    clear();

    SizeType size = init_list.size();
    if (size)
    {
        // reserve
        reserve(size);

        // setup size
        _set_sparse_size(size);
        BitAlgo::set_range(bit_array(), SizeType(0), size, true);

        // call ctor
        _copy_compacted_data(data(), init_list.begin(), size);
    }
}

// compare
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::operator==(const SparseArray& rhs) const
{
    if (sparse_size() == rhs.sparse_size())
    {
        for (SizeType i = 0; i < sparse_size(); ++i)
        {
            bool lhs_has_data = has_data(i);
            bool rhs_has_data = rhs.has_data(i);

            if (lhs_has_data != rhs_has_data)
            {
                return false;
            }
            else if (lhs_has_data)
            {
                if ((*this)[i] != rhs[i])
                {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::operator!=(const SparseArray& rhs) const
{
    return !((*this) == rhs);
}

// getter
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::size() const
{
    return sparse_size() - hole_size();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::slack() const
{
    return capacity() - sparse_size() + hole_size();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::sparse_size() const
{
    return Memory::sparse_size();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::hole_size() const
{
    return Memory::hole_size();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::bit_array_size() const
{
    return Memory::bit_array_size();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::freelist_head() const
{
    return Memory::freelist_head();
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::is_compact() const
{
    return hole_size() == 0;
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::empty() const
{
    return (sparse_size() - hole_size()) == 0;
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::StorageType* SparseArray<Memory>::data()
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE const typename SparseArray<Memory>::StorageType* SparseArray<Memory>::data() const
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::BitBlockType* SparseArray<Memory>::bit_array()
{
    return Memory::bit_array();
}
template <typename Memory>
SKR_INLINE const typename SparseArray<Memory>::BitBlockType* SparseArray<Memory>::bit_array() const
{
    return Memory::bit_array();
}
template <typename Memory>
SKR_INLINE Memory& SparseArray<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& SparseArray<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::has_data(SizeType idx) const
{
    return idx < sparse_size() && BitAlgo::get(bit_array(), idx);
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::is_hole(SizeType idx) const
{
    return !BitAlgo::get(bit_array(), idx);
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::is_valid_index(SizeType idx) const
{
    return idx >= 0 && idx < sparse_size();
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::is_valid_pointer(const DataType* p) const
{
    return p >= reinterpret_cast<const DataType*>(data()) && p < reinterpret_cast<const DataType*>(data() + sparse_size());
}

// memory op
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::clear()
{
    Memory::clear();
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::release(SizeType reserve_capacity)
{
    clear();
    if (reserve_capacity)
    {
        _realloc(reserve_capacity);
    }
    else
    {
        _free();
    }
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        _realloc(expect_capacity);
    }
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::shrink()
{
    compact_top();
    Memory::shrink();
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::compact()
{
    if (!is_compact())
    {
        // fill hole
        SizeType compacted_index = sparse_size() - hole_size();
        SizeType search_index    = sparse_size();
        SizeType free_node       = freelist_head();
        while (free_node != npos)
        {
            SizeType next_index = data()[free_node]._sparse_array_freelist_next;
            if (free_node < compacted_index)
            {
                // find last allocated element
                do
                {
                    --search_index;
                } while (!has_data(search_index));

                // move element to the hole
                memory::move<DataType, DataType>(&data()[free_node]._sparse_array_data, &data()[search_index]._sparse_array_data);
            }
            free_node = next_index;
        }

        // setup bit array
        BitAlgo::set_range(bit_array(), compacted_index, hole_size(), false);
        BitAlgo::set_range(bit_array(), SizeType(0), compacted_index, true);

        // setup data
        _set_hole_size(0);
        _set_freelist_head(npos);
        _set_sparse_size(compacted_index);

        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::compact_stable()
{
    if (!is_compact())
    {
        SizeType compacted_index = sparse_size() - hole_size();
        SizeType read_index      = 0;
        SizeType write_index     = 0;

        // skip first compacted range
        while (has_data(write_index) && write_index != sparse_size())
            ++write_index;
        read_index = write_index + 1;

        // copy items
        while (read_index < sparse_size())
        {
            // skip hole
            while (!has_data(read_index) && read_index < sparse_size())
                ++read_index;

            // move items
            while (read_index < sparse_size() && has_data(read_index))
            {
                memory::move(&data()[write_index]._sparse_array_data, &data()[read_index]._sparse_array_data);
                ++write_index;
                ++read_index;
            }
        }

        // setup bit array
        BitAlgo::set_range(bit_array(), compacted_index, hole_size(), false);
        BitAlgo::set_range(bit_array(), SizeType(0), compacted_index, true);

        // reset data
        _set_hole_size(0);
        _set_freelist_head(0);
        _set_sparse_size(compacted_index);

        return true;
    }
    else
    {
        return false;
    }
}
template <typename Memory>
SKR_INLINE bool SparseArray<Memory>::compact_top()
{
    if (!is_compact())
    {
        bool has_changes = false;
        for (SizeType i(sparse_size() - 1), n(sparse_size()); n; --i, --n)
        {
            if (is_hole(i))
            {
                // remove from freelist
                _set_hole_size(hole_size() - 1);
                _break_freelist_at(i);

                // update size
                _set_sparse_size(sparse_size() - 1);

                has_changes = true;
            }
            else
            {
                break;
            }
        }
        return has_changes;
    }
    else
    {
        return false;
    }
}

// add
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::add(const DataType& v)
{
    DataRef info = add_unsafe();
    new (info.ptr()) DataType(v);
    return info;
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::add(DataType&& v)
{
    DataRef info = add_unsafe();
    new (info.ptr()) DataType(std::move(v));
    return info;
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::add_unsafe()
{
    SizeType index;

    if (hole_size()) // has hole case
    {
        // remove and use first index from freelist
        index = freelist_head();
        _set_freelist_head(data()[index]._sparse_array_freelist_next);
        _set_hole_size(hole_size() - 1);

        // break link
        if (hole_size())
        {
            data()[freelist_head()]._sparse_array_freelist_prev = npos;
        }
    }
    else // no hole case
    {
        index = _grow(1);
    }

    // setup bit
    BitAlgo::set(bit_array(), index, true);

    return { &data()[index]._sparse_array_data, index };
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::add_default()
{
    DataRef info = add_unsafe();
    new (info.data) DataType();
    return info;
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::add_zeroed()
{
    DataRef info = add_unsafe();
    std::memset(info.ptr(), 0, sizeof(DataType));
    return info;
}

// add at
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::add_at(SizeType idx, const DataType& v)
{
    add_at_unsafe(idx);
    new (&data()[idx]._sparse_array_data) DataType(v);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::add_at(SizeType idx, DataType&& v)
{
    add_at_unsafe(idx);
    new (&data()[idx]._sparse_array_data) DataType(std::move(v));
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::add_at_unsafe(SizeType idx)
{
    SKR_ASSERT(is_hole(idx));
    SKR_ASSERT(is_valid_index(idx));

    // remove from freelist
    _set_hole_size(hole_size() - 1);
    _break_freelist_at(idx);

    // setup bit
    BitAlgo::set(bit_array(), idx, true);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::add_at_default(SizeType idx)
{
    add_at_unsafe(idx);
    new (&data()[idx]._sparse_array_data) DataType();
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::add_at_zeroed(SizeType idx)
{
    add_at_unsafe(idx);
    std::memset(&data()[idx]._sparse_array_data, 0, sizeof(DataType));
}

// emplace
template <typename Memory>
template <typename... Args>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::emplace(Args&&... args)
{
    DataRef info = add_unsafe();
    new (info.ptr()) DataType(std::forward<Args>(args)...);
    return info;
}
template <typename Memory>
template <typename... Args>
SKR_INLINE void SparseArray<Memory>::emplace_at(SizeType index, Args&&... args)
{
    add_at_unsafe(index);
    new (&data()[index]._sparse_array_data) DataType(std::forward<Args>(args)...);
}

// append
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::append(const SparseArray& arr)
{
    // fill hole
    SizeType count = 0;
    auto     it    = arr.begin();
    while (hole_size() > 0 && it != arr.end())
    {
        add(*it);
        ++it;
        ++count;
    }

    // grow and copy
    if (it != arr.end())
    {
        auto write_idx  = sparse_size();
        auto grow_count = arr.size() - count;
        _grow(grow_count);
        BitAlgo::set_range(bit_array(), write_idx, grow_count, true);
        while (it != arr.end())
        {
            new (&(data()[write_idx]._sparse_array_data)) DataType(*it);
            ++write_idx;
            ++it;
        }
        SKR_ASSERT(write_idx == sparse_size());
    }
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::append(std::initializer_list<DataType> init_list)
{
    // fill hole
    SizeType read_idx = 0;
    while (hole_size() > 0 && read_idx < init_list.size())
    {
        add(init_list.begin()[read_idx]);
        ++read_idx;
    }

    // grow and copy
    if (read_idx < init_list.size())
    {
        auto write_idx  = sparse_size();
        auto grow_count = init_list.size() - read_idx;
        _grow(grow_count);
        BitAlgo::set_range(bit_array(), write_idx, grow_count, true);
        _copy_compacted_data(data() + write_idx, init_list.begin() + read_idx, grow_count);
    }
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::append(DataType* p, SizeType n)
{
    // fill hole
    SizeType read_idx = 0;
    while (hole_size() > 0 && read_idx < n)
    {
        add(p[read_idx]);
        ++read_idx;
    }

    // grow
    if (read_idx < n)
    {
        auto write_idx  = sparse_size();
        auto grow_count = n - read_idx;
        _grow(grow_count);
        BitAlgo::set_range(bit_array(), write_idx, grow_count, true);
        _copy_compacted_data(data() + write_idx, p + read_idx, grow_count);
    }
}

// remove
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::remove_at(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index));
    SKR_ASSERT(is_valid_index(index + n - 1));
    SKR_ASSERT(n > 0);

    if constexpr (memory::MemoryTraits<DataType>::use_dtor)
    {
        for (SizeType i = 0; i < n; ++i)
        {
            SKR_ASSERT(has_data(index + i));
            memory::destruct(&data()[index + i]._sparse_array_data);
        }
    }

    remove_at_unsafe(index, n);
}
template <typename Memory>
SKR_INLINE void SparseArray<Memory>::remove_at_unsafe(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index));
    SKR_ASSERT(is_valid_index(index + n - 1));
    SKR_ASSERT(n > 0);

    for (; n; --n)
    {
        SKR_ASSERT(has_data(index));

        StorageType& cur_data = data()[index];

        // link to freelist
        if (hole_size())
        {
            data()[freelist_head()]._sparse_array_freelist_prev = index;
        }
        cur_data._sparse_array_freelist_prev = npos;
        cur_data._sparse_array_freelist_next = freelist_head();
        _set_freelist_head(index);
        _set_hole_size(hole_size() + 1);

        // set flag
        BitAlgo::set(bit_array(), index, false);

        // update index
        ++index;
    }
}
template <typename Memory>
template <typename U>
SKR_INLINE bool SparseArray<Memory>::remove(const U& v)
{
    return remove_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE bool SparseArray<Memory>::remove_last(const U& v)
{
    return remove_last_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::remove_all(const U& v)
{
    return remove_all_if([&v](const DataType& a) { return a == v; });
}

// erase, needn't update iterator, erase directly is safe
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::StlIt SparseArray<Memory>::erase(const StlIt& it)
{
    remove_at(it.index());
    StlIt new_it(it);
    ++new_it;
    return new_it;
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::CStlIt SparseArray<Memory>::erase(const CStlIt& it)
{
    remove_at(it.index());
    CStlIt new_it(it);
    ++new_it;
    return new_it;
}

// remove if
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseArray<Memory>::remove_if(Pred&& pred)
{
    if (DataRef ref = find_if(std::forward<Pred>(pred)))
    {
        remove_at(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseArray<Memory>::remove_last_if(Pred&& pred)
{
    if (DataRef ref = find_last_if(std::forward<Pred>(pred)))
    {
        remove_at(ref.index());
        return true;
    }
    return false;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::remove_all_if(Pred&& pred)
{
    SizeType count = 0;
    for (auto it = begin(); it != end(); ++it)
    {
        if (pred(*it))
        {
            remove_at(it.index());
            ++count;
        }
    }
    return count;
}

// modify
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataType& SparseArray<Memory>::operator[](SizeType index)
{
    SKR_ASSERT(!empty() && is_valid_index(index) && has_data(index));
    return data()[index]._sparse_array_data;
}
template <typename Memory>
SKR_INLINE const typename SparseArray<Memory>::DataType& SparseArray<Memory>::operator[](SizeType index) const
{
    SKR_ASSERT(!empty() && is_valid_index(index) && has_data(index));
    return data()[index]._sparse_array_data;
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataType& SparseArray<Memory>::at(SizeType index)
{
    SKR_ASSERT(!empty() && is_valid_index(index) && has_data(index));
    return data()[index]._sparse_array_data;
}
template <typename Memory>
SKR_INLINE const typename SparseArray<Memory>::DataType& SparseArray<Memory>::at(SizeType index) const
{
    SKR_ASSERT(!empty() && is_valid_index(index) && has_data(index));
    return data()[index]._sparse_array_data;
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::DataType& SparseArray<Memory>::last(SizeType index)
{
    index = sparse_size() - index - 1;
    SKR_ASSERT(!empty() && is_valid_index(index) && has_data(index));
    return *(data() + index);
}
template <typename Memory>
SKR_INLINE const typename SparseArray<Memory>::DataType& SparseArray<Memory>::last(SizeType index) const
{
    index = sparse_size() - index - 1;
    SKR_ASSERT(!empty() && is_valid_index(index) && has_data(index));
    return *(data() + index);
}

// find
template <typename Memory>
template <typename U>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::find(const U& v)
{
    return find_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::find_last(const U& v)
{
    return find_last_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename SparseArray<Memory>::CDataRef SparseArray<Memory>::find(const U& v) const
{
    return find_if([&v](const DataType& a) { return a == v; });
}
template <typename Memory>
template <typename U>
SKR_INLINE typename SparseArray<Memory>::CDataRef SparseArray<Memory>::find_last(const U& v) const
{
    return find_last_if([&v](const DataType& a) { return a == v; });
}

// find if
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::find_if(Pred&& pred)
{
    for (auto it = begin(); it != end(); ++it)
    {
        if (pred(*it))
        {
            return { it.data(), it.index() };
        }
    }
    return {};
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseArray<Memory>::DataRef SparseArray<Memory>::find_last_if(Pred&& pred)
{
    // TODO. reverse iterator
    // for (auto it = begin(); it != end(); ++it)
    // {
    //     if (p(*it))
    //     {
    //         return { it.data(), it.index() };
    //     }
    // }
    for (SizeType i = sparse_size() - 1; i >= 0; --i)
    {
        if (has_data(i))
        {
            if (pred(data()[i]._sparse_array_data))
            {
                return { &data()[i]._sparse_array_data, i };
            }
        }
    }
    return {};
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseArray<Memory>::CDataRef SparseArray<Memory>::find_if(Pred&& pred) const
{
    return const_cast<SparseArray*>(this)->find_if(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseArray<Memory>::CDataRef SparseArray<Memory>::find_last_if(Pred&& pred) const
{
    return const_cast<SparseArray*>(this)->find_last_if(std::forward<Pred>(pred));
}

// contains
template <typename Memory>
template <typename U>
SKR_INLINE bool SparseArray<Memory>::contains(const U& v) const
{
    return (bool)find(v);
}
template <typename Memory>
template <typename Pred>
SKR_INLINE bool SparseArray<Memory>::contains_if(Pred&& pred) const
{
    return (bool)find_if(std::forward<Pred>(pred));
}
template <typename Memory>
template <typename U>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::count(const U& v) const
{
    SizeType count = 0;
    for (auto it = begin(); it != end(); ++it)
    {
        if (*it == v)
        {
            ++count;
        }
    }
    return count;
}
template <typename Memory>
template <typename Pred>
SKR_INLINE typename SparseArray<Memory>::SizeType SparseArray<Memory>::count_if(Pred&& pred) const
{
    SizeType count = 0;
    for (auto it = begin(); it != end(); ++it)
    {
        if (pred(*it))
        {
            ++count;
        }
    }
    return count;
}

// sort
template <typename Memory>
template <typename Functor>
SKR_INLINE void SparseArray<Memory>::sort(Functor&& f)
{
    if (sparse_size())
    {
        compact();
        algo::intro_sort(
        data(),
        data() + sparse_size(),
        [&f](const StorageType& a, const StorageType& b) {
            return f(a._sparse_array_data, b._sparse_array_data);
        });
    }
}
template <typename Memory>
template <typename Functor>
SKR_INLINE void SparseArray<Memory>::sort_stable(Functor&& f)
{
    if (sparse_size())
    {
        compact_stable();
        algo::merge_sort(
        data(),
        data() + sparse_size(),
        [&f](const StorageType& a, const StorageType& b) {
            return f(a._sparse_array_data, b._sparse_array_data);
        });
    }
}

// support foreach
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::StlIt SparseArray<Memory>::begin()
{
    return StlIt(data(), sparse_size(), bit_array());
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::StlIt SparseArray<Memory>::end()
{
    return StlIt(data(), sparse_size(), bit_array(), sparse_size());
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::CStlIt SparseArray<Memory>::begin() const
{
    return CStlIt(data(), sparse_size(), bit_array());
}
template <typename Memory>
SKR_INLINE typename SparseArray<Memory>::CStlIt SparseArray<Memory>::end() const
{
    return CStlIt(data(), sparse_size(), bit_array(), sparse_size());
}
} // namespace skr::container