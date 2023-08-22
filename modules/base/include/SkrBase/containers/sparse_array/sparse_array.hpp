#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/memory/memory_ops.hpp"
#include "sparse_array_def.hpp"
#include "sparse_array_iterator.hpp"
#include "SkrBase/algo/intro_sort.hpp"
#include "SkrBase/algo/merge_sort.hpp"

// SparseArray def
// TODO. auto compact_top when remove items
namespace skr
{
template <typename T, typename TBitBlock, typename Alloc>
struct SparseArray {
    using SizeType                        = typename Alloc::SizeType;
    using DataType                        = SparseArrayData<T, SizeType>;
    using DataRef                         = SparseArrayDataRef<T, SizeType>;
    using CDataRef                        = SparseArrayDataRef<const T, SizeType>;
    using It                              = SparseArrayIt<T, TBitBlock, SizeType, false>;
    using CIt                             = SparseArrayIt<T, TBitBlock, SizeType, true>;
    using BitAlgo                         = algo::BitAlgo<TBitBlock>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // ctor & dtor
    SparseArray(Alloc alloc = {});
    SparseArray(SizeType size, Alloc alloc = {});
    SparseArray(SizeType size, const T& v, Alloc alloc = {});
    SparseArray(const T* p, SizeType n, Alloc alloc = {});
    SparseArray(std::initializer_list<T> init_list, Alloc alloc = {});
    ~SparseArray();

    // copy & move
    SparseArray(const SparseArray& other, Alloc alloc = {});
    SparseArray(SparseArray&& other) noexcept;

    // assign & move assign
    SparseArray& operator=(const SparseArray& rhs);
    SparseArray& operator=(SparseArray&& rhs) noexcept;

    // special assign
    void assign(const T* p, SizeType n);
    void assign(std::initializer_list<T> init_list);

    // compare
    bool operator==(const SparseArray& rhs) const;
    bool operator!=(const SparseArray& rhs) const;

    // getter
    SizeType         size() const;
    SizeType         capacity() const;
    SizeType         slack() const;
    SizeType         sparse_size() const;
    SizeType         hole_size() const;
    SizeType         bit_array_size() const;
    SizeType         free_list_head() const;
    bool             is_compact() const;
    bool             empty() const;
    DataType*        data();
    const DataType*  data() const;
    TBitBlock*       bit_array();
    const TBitBlock* bit_array() const;
    Alloc&           allocator();
    const Alloc&     allocator() const;

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

    // add
    DataRef add(const T& v);
    DataRef add(T&& v);
    DataRef add_unsafe();
    DataRef add_default();
    DataRef add_zeroed();

    // add at
    void add_at(SizeType idx, const T& v);
    void add_at(SizeType idx, T&& v);
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
    void append(std::initializer_list<T> init_list);
    void append(T* p, SizeType n);

    // remove
    void remove_at(SizeType index, SizeType n = 1);
    void remove_at_unsafe(SizeType index, SizeType n = 1);
    template <typename TK>
    DataRef remove(const TK& v);
    template <typename TK>
    DataRef remove_last(const TK& v);
    template <typename TK>
    SizeType remove_all(const TK& v);

    // remove if
    template <typename TP>
    DataRef remove_if(TP&& p);
    template <typename TP>
    DataRef remove_last_if(TP&& p);
    template <typename TP>
    SizeType remove_all_if(TP&& p);

    // modify
    T&       operator[](SizeType index);
    const T& operator[](SizeType index) const;

    // find
    template <typename TK>
    DataRef find(const TK& v);
    template <typename TK>
    DataRef find_last(const TK& v);
    template <typename TK>
    CDataRef find(const TK& v) const;
    template <typename TK>
    CDataRef find_last(const TK& v) const;

    // find if
    template <typename TP>
    DataRef find_if(TP&& p);
    template <typename TP>
    DataRef find_last_if(TP&& p);
    template <typename TP>
    CDataRef find_if(TP&& p) const;
    template <typename TP>
    CDataRef find_last_if(TP&& p) const;

    // contain
    template <typename TK>
    bool contain(const TK& v) const;
    template <typename TP>
    bool contain_if(TP&& p) const;

    // sort
    template <typename TP = Less<T>>
    void sort(TP&& p = TP());
    template <typename TP = Less<T>>
    void sort_stable(TP&& p = TP());

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;

private:
    // helper
    void _realloc(SizeType new_capacity);
    void _free();
    void _grow(SizeType new_capacity);

private:
    TBitBlock* _bit_array      = nullptr;
    SizeType   _bit_array_size = 0;
    SizeType   _num_hole       = 0;
    SizeType   _freelist_head  = npos;
    SizeType   _sparse_size    = 0;
    SizeType   _capacity       = 0;
    DataType*  _data           = nullptr;
    Alloc      _alloc;
};
} // namespace skr

// SparseArray impl
namespace skr
{
// helper
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::_realloc(SizeType new_capacity)
{
    SKR_ASSERT(new_capacity != _capacity);
    SKR_ASSERT(new_capacity > 0);
    SKR_ASSERT(_sparse_size <= new_capacity);
    SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

    // realloc data array
    if constexpr (memory::memory_traits<T>::use_realloc)
    {
        _data     = _alloc.template realloc<DataType>(_data, new_capacity);
        _capacity = new_capacity;
    }
    else
    {
        // alloc new memory
        DataType* new_memory = _alloc.template alloc<DataType>(new_capacity);

        // move items
        if (_sparse_size)
        {
            for (SizeType i = 0; i < _sparse_size; ++i)
            {
                DataType& new_data = *(new_memory + i);
                DataType& old_data = *(_data + i);
                if (has_data(i))
                {
                    memory::move(&new_data._sparse_array_data, &old_data._sparse_array_data);
                }
                else
                {
                    new_data._sparse_array_freelist_prev = old_data._sparse_array_freelist_prev;
                    new_data._sparse_array_freelist_next = old_data._sparse_array_freelist_next;
                }
            }
        }

        // release old memory
        _alloc.template free<DataType>(_data);

        _data     = new_memory;
        _capacity = new_capacity;
    }

    // realloc bit array
    SizeType new_block_size = BitAlgo::num_blocks(_capacity);
    SizeType old_block_size = BitAlgo::num_blocks(_bit_array_size);

    if (new_block_size != old_block_size)
    {
        // realloc bit array
        if constexpr (memory::memory_traits<TBitBlock>::use_realloc)
        {
            _bit_array      = _alloc.template realloc<TBitBlock>(_bit_array, new_block_size);
            _bit_array_size = new_block_size * BitAlgo::PerBlockSize;
        }
        else
        {
            // alloc new memory
            TBitBlock* new_memory = _alloc.template alloc<TBitBlock>(new_block_size);

            // move items
            if (_bit_array_size)
            {
                memory::move(new_memory, _bit_array, old_block_size);
            }

            // release old memory
            _alloc.template free<TBitBlock>(_bit_array);

            _bit_array      = new_memory;
            _bit_array_size = new_block_size * BitAlgo::PerBlockSize;
        }

        // cleanup new bit array memory
        if (new_block_size > old_block_size)
        {
            memset(_bit_array + old_block_size, 0, (new_block_size - old_block_size) * sizeof(TBitBlock));
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::_free()
{
    if (_data)
    {
        // destruct items
        if constexpr (memory::memory_traits<T>::use_dtor)
        {
            for (auto it = begin(); it != end(); ++it)
            {
                memory::destruct<T>(it.data());
            }
        }

        // release memory
        _alloc.free(_data);
        _alloc.free(_bit_array);

        // reset data
        _bit_array      = nullptr;
        _bit_array_size = 0;
        _num_hole       = 0;
        _freelist_head  = npos;
        _sparse_size    = 0;
        _capacity       = 0;
        _data           = nullptr;
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::_grow(SizeType new_capacity)
{
    auto new_sparse_size = _sparse_size + new_capacity;
    if (new_sparse_size > _capacity)
    {
        auto new_capacity = _alloc.get_grow(new_sparse_size, _capacity);
        SKR_ASSERT(new_capacity >= _capacity);
        if (new_capacity > _capacity)
        {
            _realloc(new_capacity);
        }
    }

    _sparse_size = new_sparse_size;
}

// ctor & dtor
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(Alloc alloc)
    : _alloc(std::move(alloc))
{
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(SizeType size, Alloc alloc)
    : _alloc(std::move(alloc))
{
    if (size)
    {
        // resize
        _realloc(size);
        _sparse_size = size;
        BitAlgo::set_range(_bit_array, SizeType(0), size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i]._sparse_array_data) T();
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(SizeType size, const T& v, Alloc alloc)
    : _alloc(std::move(alloc))
{
    if (size)
    {
        // resize
        _realloc(size);
        _sparse_size = size;
        BitAlgo::set_range(_bit_array, SizeType(0), size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i]._sparse_array_data) T(v);
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(const T* p, SizeType n, Alloc alloc)
    : _alloc(std::move(alloc))
{
    if (n)
    {
        // resize
        _realloc(n);
        _sparse_size = n;
        BitAlgo::set_range(_bit_array, SizeType(0), n, true);

        // call ctor
        for (SizeType i = 0; i < n; ++i)
        {
            new (&_data[i]._sparse_array_data) T(p[i]);
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(std::initializer_list<T> init_list, Alloc alloc)
    : _alloc(std::move(alloc))
{
    SizeType size = init_list.size();
    if (size)
    {
        // resize
        _realloc(size);
        _sparse_size = size;
        BitAlgo::set_range(_bit_array, SizeType(0), size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i]._sparse_array_data) T(*(init_list.begin() + i));
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::~SparseArray() { release(); }

// copy & move
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(const SparseArray& other, Alloc alloc)
    : _alloc(std::move(alloc))
{
    (*this) = other;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(SparseArray&& other) noexcept
    : _bit_array(other._bit_array)
    , _bit_array_size(other._bit_array_size)
    , _num_hole(other._num_hole)
    , _freelist_head(other._freelist_head)
    , _sparse_size(other._sparse_size)
    , _capacity(other._capacity)
    , _data(other._data)
    , _alloc(std::move(other._alloc))
{
    other._bit_array      = nullptr;
    other._bit_array_size = 0;
    other._num_hole       = 0;
    other._freelist_head  = npos;
    other._sparse_size    = 0;
    other._capacity       = 0;
    other._data           = nullptr;
}

// assign & move assign
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>& SparseArray<T, TBitBlock, Alloc>::operator=(const SparseArray& rhs)
{
    if (this != &rhs)
    {
        clear();
        reserve(rhs._sparse_size);
        _sparse_size = rhs._sparse_size;

        // copy data
        if constexpr (memory::memory_traits<T>::use_ctor)
        {
            for (SizeType i = 0; i < rhs._sparse_size; ++i)
            {
                DataType*       dst_data = _data + i;
                const DataType* src_data = rhs._data + i;

                if (rhs.has_data(i))
                {
                    new (&dst_data->_sparse_array_data) T(src_data->_sparse_array_data);
                }
                else
                {
                    dst_data->_sparse_array_freelist_prev = src_data->_sparse_array_freelist_prev;
                    dst_data->_sparse_array_freelist_next = src_data->_sparse_array_freelist_next;
                }
            }
        }
        else
        {
            // copy data
            std::memcpy(_data, rhs._data, sizeof(DataType) * rhs._sparse_size);
        }

        // copy bit array
        std::memcpy(_bit_array, rhs._bit_array, sizeof(TBitBlock) * BitAlgo::num_blocks(rhs._sparse_size));

        // copy other data
        _num_hole      = rhs._num_hole;
        _freelist_head = rhs._freelist_head;
    }
    return *this;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>& SparseArray<T, TBitBlock, Alloc>::operator=(SparseArray&& rhs) noexcept
{
    if (this != &rhs)
    {
        release();

        // move data
        _bit_array      = rhs._bit_array;
        _bit_array_size = rhs._bit_array_size;
        _num_hole       = rhs._num_hole;
        _freelist_head  = rhs._freelist_head;
        _sparse_size    = rhs._sparse_size;
        _capacity       = rhs._capacity;
        _data           = rhs._data;
        _alloc          = std::move(rhs._alloc);

        // invalidate rhs
        rhs._bit_array      = nullptr;
        rhs._bit_array_size = 0;
        rhs._num_hole       = 0;
        rhs._freelist_head  = npos;
        rhs._sparse_size    = 0;
        rhs._capacity       = 0;
        rhs._data           = nullptr;
    }
    return *this;
}

// special assign
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::assign(const T* p, SizeType n)
{
    clear();

    if (n)
    {
        // resize
        reserve(n);
        _sparse_size = n;
        BitAlgo::set_range(_bit_array, SizeType(0), n, true);

        // call ctor
        for (SizeType i = 0; i < n; ++i)
        {
            new (&_data[i]._sparse_array_data) T(p[i]);
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::assign(std::initializer_list<T> init_list)
{
    clear();

    SizeType size = init_list.size();
    if (size)
    {
        // resize
        reserve(size);
        _sparse_size = size;
        BitAlgo::set_range(_bit_array, SizeType(0), size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i]._sparse_array_data) T(*(init_list.begin() + i));
        }
    }
}

// compare
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::operator==(const SparseArray& rhs) const
{
    if (_sparse_size == rhs._sparse_size)
    {
        for (SizeType i = 0; i < _sparse_size; ++i)
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
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::operator!=(const SparseArray& rhs) const
{
    return !((*this) == rhs);
}

// getter
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::size() const
{
    return _sparse_size - _num_hole;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::capacity() const
{
    return _capacity;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::slack() const
{
    return _capacity - _sparse_size + _num_hole;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::sparse_size() const
{
    return _sparse_size;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::hole_size() const
{
    return _num_hole;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::bit_array_size() const
{
    return _bit_array_size;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::free_list_head() const
{
    return _freelist_head;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::is_compact() const
{
    return _num_hole == 0;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::empty() const
{
    return (_sparse_size - _num_hole) == 0;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataType* SparseArray<T, TBitBlock, Alloc>::data()
{
    return _data;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE const typename SparseArray<T, TBitBlock, Alloc>::DataType* SparseArray<T, TBitBlock, Alloc>::data() const
{
    return _data;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE TBitBlock* SparseArray<T, TBitBlock, Alloc>::bit_array() { return _bit_array; }
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE const TBitBlock* SparseArray<T, TBitBlock, Alloc>::bit_array() const
{
    return _bit_array;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE Alloc& SparseArray<T, TBitBlock, Alloc>::allocator() { return _alloc; }
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE const Alloc& SparseArray<T, TBitBlock, Alloc>::allocator() const
{
    return _alloc;
}

// validate
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::has_data(SizeType idx) const
{
    return BitAlgo::get(_bit_array, idx);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::is_hole(SizeType idx) const
{
    return !BitAlgo::get(_bit_array, idx);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::is_valid_index(SizeType idx) const
{
    return idx >= 0 && idx < _sparse_size;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::is_valid_pointer(const T* p) const
{
    return p >= reinterpret_cast<T*>(_data) && p < reinterpret_cast<T*>(_data + _sparse_size);
}

// memory op
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::clear()
{
    // destruct items
    if constexpr (memory::memory_traits<T>::use_dtor)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            memory::destruct<T>(it.data());
        }
    }

    // clean up bit array
    if (_bit_array)
    {
        BitAlgo::set_blocks(_bit_array, SizeType(0), BitAlgo::num_blocks(_bit_array_size), false);
    }

    // clean up data
    _num_hole      = 0;
    _freelist_head = npos;
    _sparse_size   = 0;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::release(SizeType capacity)
{
    clear();
    if (capacity)
    {
        _realloc(capacity);
    }
    else
    {
        _free();
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::reserve(SizeType capacity)
{
    if (capacity > _capacity)
    {
        _realloc(capacity);
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::shrink()
{
    auto new_capacity = _alloc.get_shrink(_sparse_size, _capacity);
    SKR_ASSERT(new_capacity >= _sparse_size);
    if (new_capacity < _capacity)
    {
        _realloc(new_capacity);
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::compact()
{
    if (!is_compact())
    {
        // fill hole
        SizeType compacted_index = _sparse_size - _num_hole;
        SizeType search_index    = _sparse_size;
        while (_freelist_head != npos)
        {
            SizeType next_index = _data[_freelist_head]._sparse_array_freelist_next;
            if (_freelist_head < compacted_index)
            {
                // find last allocated element
                do
                {
                    --search_index;
                } while (!has_data(search_index));

                // move element to the hole
                memory::move<T, T>(&_data[_freelist_head]._sparse_array_data, &_data[search_index]._sparse_array_data);
            }
            _freelist_head = next_index;
        }

        // setup bit array
        BitAlgo::set_range(_bit_array, compacted_index, _num_hole, false);
        BitAlgo::set_range(_bit_array, SizeType(0), compacted_index, true);

        // setup data
        _num_hole      = 0;
        _freelist_head = npos;
        _sparse_size   = compacted_index;

        return true;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::compact_stable()
{
    if (!is_compact())
    {
        SizeType compacted_index = _sparse_size - _num_hole;
        SizeType read_index      = 0;
        SizeType write_index     = 0;

        // skip first compacted range
        while (has_data(write_index) && write_index != _sparse_size)
            ++write_index;
        read_index = write_index + 1;

        // copy items
        while (read_index < _sparse_size)
        {
            // skip hole
            while (!has_data(read_index) && read_index < _sparse_size)
                ++read_index;

            // move items
            while (read_index < _sparse_size && has_data(read_index))
            {
                memory::move(&_data[write_index]._sparse_array_data, &_data[read_index]._sparse_array_data);
                ++write_index;
                ++read_index;
            }
        }

        // setup bit array
        BitAlgo::set_range(_bit_array, compacted_index, _num_hole, false);
        BitAlgo::set_range(_bit_array, SizeType(0), compacted_index, true);

        // reset data
        _num_hole      = 0;
        _freelist_head = npos;
        _sparse_size   = compacted_index;

        return true;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::compact_top()
{
    if (!is_compact())
    {
        bool has_changes = false;
        for (SizeType i(_sparse_size - 1), n(_sparse_size); n; --i, --n)
        {
            DataType& data = _data[i];

            if (is_hole(i))
            {
                // remove from freelist
                --_num_hole;
                if (_freelist_head == i)
                {
                    _freelist_head = data._sparse_array_freelist_next;
                }
                if (data._sparse_array_freelist_next != npos)
                {
                    _data[data._sparse_array_freelist_next]._sparse_array_freelist_prev = data._sparse_array_freelist_prev;
                }
                if (data._sparse_array_freelist_prev != npos)
                {
                    _data[data._sparse_array_freelist_prev]._sparse_array_freelist_next = data._sparse_array_freelist_next;
                }

                // update size
                --_sparse_size;

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
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::add(const T& v)
{
    DataRef info = add_unsafe();
    new (info.data) T(v);
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::add(T&& v)
{
    DataRef info = add_unsafe();
    new (info.data) T(std::move(v));
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::add_unsafe()
{
    SizeType index;

    if (_num_hole) // has hole case
    {
        // remove and use first index from freelist
        index          = _freelist_head;
        _freelist_head = _data[_freelist_head]._sparse_array_freelist_next;
        --_num_hole;

        // break link
        if (_num_hole)
        {
            _data[_freelist_head]._sparse_array_freelist_prev = npos;
        }
    }
    else // no hole case
    {
        index = _sparse_size;
        _grow(1);
    }

    // setup bit
    BitAlgo::set(_bit_array, index, true);

    return { &_data[index]._sparse_array_data, index };
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::add_default()
{
    DataRef info = add_unsafe();
    new (info.data) T();
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::add_zeroed()
{
    DataRef info = add_unsafe();
    std::memset(info.data, 0, sizeof(T));
    return info;
}

// add at
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::add_at(SizeType idx, const T& v)
{
    add_at_unsafe(idx);
    new (&_data[idx]._sparse_array_data) T(v);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::add_at(SizeType idx, T&& v)
{
    add_at_unsafe(idx);
    new (&_data[idx]._sparse_array_data) T(std::move(v));
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::add_at_unsafe(SizeType idx)
{
    SKR_ASSERT(is_hole(idx));
    SKR_ASSERT(is_valid_index(idx));

    DataType& data = _data[idx];

    // remove from freelist
    --_num_hole;
    if (_freelist_head == idx)
    {
        _freelist_head = data._sparse_array_freelist_next;
    }
    if (data._sparse_array_freelist_next != npos)
    {
        _data[data._sparse_array_freelist_next]._sparse_array_freelist_prev = data._sparse_array_freelist_prev;
    }
    if (data._sparse_array_freelist_prev != npos)
    {
        _data[data._sparse_array_freelist_prev]._sparse_array_freelist_next = data._sparse_array_freelist_next;
    }

    // setup bit
    BitAlgo::set(_bit_array, idx, true);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::add_at_default(SizeType idx)
{
    add_at_unsafe(idx);
    new (&_data[idx]._sparse_array_data) T();
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::add_at_zeroed(SizeType idx)
{
    add_at_unsafe(idx);
    std::memset(&_data[idx]._sparse_array_data, 0, sizeof(T));
}

// emplace
template <typename T, typename TBitBlock, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::emplace(Args&&... args)
{
    DataRef info = add_unsafe();
    new (info.data) T(std::forward<Args>(args)...);
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename... Args>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::emplace_at(SizeType index, Args&&... args)
{
    add_at_unsafe(index);
    new (&_data[index]._sparse_array_data) T(std::forward<Args>(args)...);
}

// append
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::append(const SparseArray& arr)
{
    // fill hole
    SizeType count = 0;
    auto     it    = arr.begin();
    while (_num_hole > 0 && it != arr.end())
    {
        add(*it);
        ++it;
        ++count;
    }

    // grow and copy
    if (it != arr.end())
    {
        auto write_idx  = _sparse_size;
        auto grow_count = arr.size() - count;
        _grow(grow_count);
        BitAlgo::set_range(_bit_array, write_idx, grow_count, true);
        while (it != arr.end())
        {
            new (&(_data[write_idx]._sparse_array_data)) T(*it);
            ++write_idx;
            ++it;
        }
        SKR_ASSERT(write_idx == _sparse_size);
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::append(std::initializer_list<T> init_list)
{
    // fill hole
    SizeType read_idx = 0;
    while (_num_hole > 0 && read_idx < init_list.size())
    {
        add(init_list.begin()[read_idx]);
        ++read_idx;
    }

    // grow and copy
    if (read_idx < init_list.size())
    {
        auto write_idx  = _sparse_size;
        auto grow_count = init_list.size() - read_idx;
        _grow(grow_count);
        BitAlgo::set_range(_bit_array, write_idx, grow_count, true);
        while (read_idx < init_list.size())
        {
            new (&(_data[write_idx]._sparse_array_data)) T(init_list.begin()[read_idx]);
            ++write_idx;
            ++read_idx;
        }
        SKR_ASSERT(write_idx == _sparse_size);
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::append(T* p, SizeType n)
{
    // fill hole
    SizeType read_idx = 0;
    while (_num_hole > 0 && read_idx < n)
    {
        add(p[read_idx]);
        ++read_idx;
    }

    // grow
    if (read_idx < n)
    {
        auto write_idx  = _sparse_size;
        auto grow_count = n - read_idx;
        _grow(grow_count);
        BitAlgo::set_range(_bit_array, write_idx, grow_count, true);
        while (read_idx < n)
        {
            new (&(_data[write_idx]._sparse_array_data)) T(p[read_idx]);
            ++write_idx;
            ++read_idx;
        }
        SKR_ASSERT(write_idx == _sparse_size);
    }
}

// remove
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::remove_at(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index));
    SKR_ASSERT(is_valid_index(index + n - 1));
    SKR_ASSERT(n > 0);

    if constexpr (memory::memory_traits<T>::use_dtor)
    {
        for (SizeType i = 0; i < n; ++i)
        {
            SKR_ASSERT(has_data(index + i));
            memory::destruct(&_data[index + i]._sparse_array_data);
        }
    }

    remove_at_unsafe(index, n);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::remove_at_unsafe(SizeType index, SizeType n)
{
    SKR_ASSERT(is_valid_index(index));
    SKR_ASSERT(is_valid_index(index + n - 1));
    SKR_ASSERT(n > 0);

    for (; n; --n)
    {
        SKR_ASSERT(has_data(index));

        DataType& data = _data[index];

        // link to freelist
        if (_num_hole)
        {
            _data[_freelist_head]._sparse_array_freelist_prev = index;
        }
        data._sparse_array_freelist_prev = npos;
        data._sparse_array_freelist_next = _freelist_head;
        _freelist_head                   = index;
        ++_num_hole;

        // set flag
        BitAlgo::set(_bit_array, index, false);

        // update index
        ++index;
    }
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::remove(const TK& v)
{
    return remove_if([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::remove_last(const TK& v)
{
    return remove_last_if([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::remove_all(const TK& v)
{
    return remove_all_if([&v](const T& a) { return a == v; });
}

// remove if
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::remove_if(TP&& p)
{
    if (DataRef ref = find_if(std::forward<TP>(p)))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::remove_last_if(TP&& p)
{
    if (DataRef ref = find_last_if(std::forward<TP>(p)))
    {
        remove_at(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::remove_all_if(TP&& p)
{
    SizeType count = 0;
    for (auto it = begin(); it != end(); ++it)
    {
        if (p(*it))
        {
            remove_at(it.index());
            ++count;
        }
    }
    return count;
}

// modify
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE T& SparseArray<T, TBitBlock, Alloc>::operator[](SizeType index)
{
    SKR_ASSERT(is_valid_index(index));
    return _data[index]._sparse_array_data;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE const T& SparseArray<T, TBitBlock, Alloc>::operator[](SizeType index) const
{
    SKR_ASSERT(is_valid_index(index));
    return _data[index]._sparse_array_data;
}

// find
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::find(const TK& v)
{
    return find_if([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::find_last(const TK& v)
{
    return find_last_if([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::find(const TK& v) const
{
    return find_if([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::find_last(const TK& v) const
{
    return find_last_if([&v](const T& a) { return a == v; });
}

// find if
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::find_if(TP&& p)
{
    for (auto it = begin(); it != end(); ++it)
    {
        if (p(*it))
        {
            return { it.data(), it.index() };
        }
    }
    return {};
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::find_last_if(TP&& p)
{
    // TODO. reserve iterator
    // for (auto it = begin(); it != end(); ++it)
    // {
    //     if (p(*it))
    //     {
    //         return { it.data(), it.index() };
    //     }
    // }
    for (SizeType i = _sparse_size - 1; i >= 0; --i)
    {
        if (has_data(i))
        {
            if (p(_data[i]._sparse_array_data))
            {
                return { &_data[i]._sparse_array_data, i };
            }
        }
    }
    return {};
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::find_if(TP&& p) const
{
    auto ref = const_cast<SparseArray*>(this)->find_if(std::forward<TP>(p));
    return { ref.data, ref.index };
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::find_last_if(TP&& p) const
{
    auto ref = const_cast<SparseArray*>(this)->find_last_if(std::forward<TP>(p));
    return { ref.data, ref.index };
}

// contain
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::contain(const TK& v) const
{
    return (bool)find(v);
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::contain_if(TP&& p) const
{
    return (bool)find_if(std::forward<TP>(p));
}

// sort
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::sort(TP&& p)
{
    if (_sparse_size)
    {
        compact();
        algo::intro_sort(_data, _data + _sparse_size, [&p](const DataType& a, const DataType& b) { return p(a._sparse_array_data, b._sparse_array_data); });
    }
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::sort_stable(TP&& p)
{
    if (_sparse_size)
    {
        compact_stable();
        algo::merge_sort(_data, _data + _sparse_size, [&p](const DataType& a, const DataType& b) { return p(a._sparse_array_data, b._sparse_array_data); });
    }
}

// support foreach
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::It SparseArray<T, TBitBlock, Alloc>::begin()
{
    return It(data(), sparse_size(), bit_array());
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::It SparseArray<T, TBitBlock, Alloc>::end()
{
    return It(data(), sparse_size(), bit_array(), sparse_size());
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CIt SparseArray<T, TBitBlock, Alloc>::begin() const
{
    return CIt(data(), sparse_size(), bit_array());
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CIt SparseArray<T, TBitBlock, Alloc>::end() const
{
    return CIt(data(), sparse_size(), bit_array(), sparse_size());
}
} // namespace skr