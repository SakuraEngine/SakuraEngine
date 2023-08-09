#pragma once
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/config.h"
#include "SkrBase/algo/intro_sort.hpp"
#include "SkrBase/algo/merge_sort.hpp"
#include "SkrBase/algo/remove.hpp"
#include "SkrBase/algo/find.hpp"
#include "sparse_array_iterator.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"

// SparseArray def
namespace skr
{
template <typename T, typename TBitBlock, typename Alloc>
class SparseArray
{
public:
    using SizeType                        = typename Alloc::SizeType;
    using DataType                        = SparseArrayData<T, SizeType>;
    using DataRef                         = SparseArrayDataRef<T, SizeType>;
    using CDataRef                        = SparseArrayDataRef<const T, SizeType>;
    using It                              = SparseArrayIt<T, TBitBlock, SizeType, false>;
    using CIt                             = SparseArrayIt<T, TBitBlock, SizeType, true>;
    using Algo                            = algo::BitAlgo<TBitBlock>;
    static inline constexpr SizeType npos = npos_of<SizeType>;

    // ctor & dtor
    SparseArray(Alloc alloc = Alloc());
    SparseArray(SizeType size, Alloc alloc = Alloc());
    SparseArray(SizeType size, const T& v, Alloc alloc = Alloc());
    SparseArray(const T* p, SizeType n, Alloc alloc = Alloc());
    SparseArray(std::initializer_list<T> init_list, Alloc alloc = Alloc());
    ~SparseArray();

    // copy & move
    SparseArray(const SparseArray& other, Alloc alloc = Alloc());
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
    SizeType         sparseSize() const;
    SizeType         holeSize() const;
    SizeType         bitArraySize() const;
    SizeType         freelistHead() const;
    bool             isCompact() const;
    bool             empty() const;
    DataType*        data();
    const DataType*  data() const;
    TBitBlock*       bitArray();
    const TBitBlock* bitArray() const;
    Alloc&           allocator();
    const Alloc&     allocator() const;

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

    // add
    DataRef add(const T& v);
    DataRef add(T&& v);
    DataRef addUnsafe();
    DataRef addDefault();
    DataRef addZeroed();

    // add at
    void addAt(SizeType idx, const T& v);
    void addAt(SizeType idx, T&& v);
    void addAtUnsafe(SizeType idx);
    void addAtDefault(SizeType idx);
    void addAtZeroed(SizeType idx);

    // emplace
    template <typename... Args>
    DataRef emplace(Args&&... args);
    template <typename... Args>
    void emplaceAt(SizeType index, Args&&... args);

    // append
    void append(const SparseArray& arr);
    void append(std::initializer_list<T> init_list);
    void append(T* p, SizeType n);

    // remove
    void removeAt(SizeType index, SizeType n = 1);
    void removeAtUnsafe(SizeType index, SizeType n = 1);
    template <typename TK>
    DataRef remove(const TK& v);
    template <typename TK>
    DataRef removeLast(const TK& v);
    template <typename TK>
    SizeType removeAll(const TK& v);

    // remove if
    template <typename TP>
    DataRef removeIf(TP&& p);
    template <typename TP>
    DataRef removeLastIf(TP&& p);
    template <typename TP>
    SizeType removeAllIf(TP&& p);

    // modify
    T&       operator[](SizeType index);
    const T& operator[](SizeType index) const;

    // find
    template <typename TK>
    DataRef find(const TK& v);
    template <typename TK>
    DataRef findLast(const TK& v);
    template <typename TK>
    CDataRef find(const TK& v) const;
    template <typename TK>
    CDataRef findLast(const TK& v) const;

    // find if
    template <typename TP>
    DataRef findIf(TP&& p);
    template <typename TP>
    DataRef findLastIf(TP&& p);
    template <typename TP>
    CDataRef findIf(TP&& p) const;
    template <typename TP>
    CDataRef findLastIf(TP&& p) const;

    // contain
    template <typename TK>
    bool contain(const TK& v) const;
    template <typename TP>
    bool containIf(TP&& p) const;

    // sort
    template <typename TP = Less<T>>
    void sort(TP&& p = TP());
    template <typename TP = Less<T>>
    void sortStable(TP&& p = TP());

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;

private:
    // helper
    void _setBit(SizeType index, bool v);
    bool _getBit(SizeType index) const;
    void _setBitRange(SizeType start, SizeType n, bool v);
    void _resizeMemory(SizeType new_capacity);
    void _grow(SizeType n);

private:
    TBitBlock* _bit_array;
    SizeType   _bit_array_size;
    SizeType   _num_hole;
    SizeType   _freelist_head;
    SizeType   _sparse_size;
    SizeType   _capacity;
    DataType*  _data;
    Alloc      _alloc;
};
} // namespace skr

// SparseArray impl
namespace skr
{
// helper
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::_setBit(SizeType index, bool v)
{
    Algo::set(_bit_array, index, v);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::_getBit(SizeType index) const
{
    return Algo::get(_bit_array, index);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::_setBitRange(SizeType start, SizeType n, bool v)
{
    Algo::setRange(_bit_array, start, n, v);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::_resizeMemory(SizeType new_capacity)
{
    if (new_capacity)
    {
        // calc new size
        auto new_sparse_size = std::min(_sparse_size, _capacity);

        // realloc
        if constexpr (memory::memory_traits<T>::use_realloc)
        {
            _data = _alloc.resizeContainer(_data, _sparse_size, _capacity, new_capacity);
        }
        else
        {
            // alloc new memory
            DataType* new_memory = _alloc.template alloc<DataType>(new_capacity);

            // move items
            for (SizeType i = 0; i < new_sparse_size; ++i)
            {
                DataType& new_data = *(new_memory + i);
                DataType& old_data = *(_data + i);
                if (hasData(i))
                {
                    new_data.data = std::move(old_data.data);
                }
                else
                {
                    new_data.prev = old_data.prev;
                    new_data.next = old_data.next;
                }
            }

            // destruct items
            if constexpr (memory::memory_traits<T>::call_dtor)
            {
                for (SizeType i = 0; i < _sparse_size; ++i)
                {
                    if (hasData(i))
                    {
                        (_data + i)->data.~T();
                    }
                }
            }

            // release old memory
            _alloc.free(_data);
            _data = new_memory;
        }

        // update size
        _sparse_size = new_sparse_size;
        _capacity    = new_capacity;

        // resize bit array
        SizeType data_block_size = Algo::numBlocks(_capacity);
        SizeType old_block_size  = Algo::numBlocks(_bit_array_size);
        if (data_block_size != old_block_size)
        {
            SizeType new_block_size = data_block_size;

            // resize
            _bit_array = _alloc.resizeContainer(_bit_array, old_block_size, old_block_size, new_block_size);

            // clean memory
            if (data_block_size > old_block_size)
            {
                Algo::setBlocks(_bit_array + old_block_size, SizeType(0), new_block_size - old_block_size, false);
            }

            // update size
            _bit_array_size = new_block_size * Algo::PerBlockSize;
        }
    }
    else if (_data)
    {
        // destruct items
        if constexpr (memory::memory_traits<T>::call_dtor)
        {
            for (SizeType i = 0; i < _sparse_size; ++i)
            {
                if (hasData(i))
                {
                    (_data + i)->data.~T();
                }
            }
        }

        // free memory
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
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::_grow(SizeType n)
{
    auto new_sparse_size = _sparse_size + n;

    // grow memory
    if (new_sparse_size > _capacity)
    {
        auto new_capacity = _alloc.getGrow(new_sparse_size, _capacity);

        // grow memory
        if constexpr (memory::memory_traits<T>::use_realloc)
        {
            _data = _alloc.resizeContainer(_data, _sparse_size, _capacity, new_capacity);
        }
        else
        {
            // alloc new memory
            DataType* new_memory = _alloc.template alloc<DataType>(new_capacity);

            // move items
            for (SizeType i = 0; i < _sparse_size; ++i)
            {
                DataType& new_data = *(new_memory + i);
                DataType& old_data = *(_data + i);
                if (hasData(i))
                {
                    new_data.data = std::move(old_data.data);
                    if constexpr (memory::memory_traits<T>::call_dtor)
                    {
                        old_data->data.~T();
                    }
                }
                else
                {
                    new_data.prev = old_data.prev;
                    new_data.next = old_data.next;
                }
            }

            // release old memory
            _alloc.free(_data);
            _data = new_memory;
        }

        // update capacity
        _capacity = new_capacity;

        // grow bit array
        if (_bit_array_size < _capacity)
        {
            // calc grow size
            SizeType data_block_size = Algo::numBlocks(_capacity);
            SizeType old_block_size  = Algo::numBlocks(_bit_array_size);
            SizeType new_block_size  = _alloc.getGrow(data_block_size, old_block_size);

            // alloc memory and clean
            if (new_block_size > old_block_size)
            {
                // alloc
                _bit_array = _alloc.resizeContainer(_bit_array, old_block_size, old_block_size, new_block_size);

                // clean
                Algo::setBlocks(_bit_array + old_block_size, SizeType(0), new_block_size - old_block_size, false);

                // update size
                _bit_array_size = new_block_size * Algo::PerBlockSize;
            }
        }
    }

    // update size
    _sparse_size = new_sparse_size;
}

// ctor & dtor
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(Alloc alloc)
    : _bit_array(nullptr)
    , _bit_array_size(0)
    , _num_hole(0)
    , _freelist_head(npos)
    , _sparse_size(0)
    , _capacity(0)
    , _data(nullptr)
    , _alloc(std::move(alloc))
{
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(SizeType size, Alloc alloc)
    : _bit_array(nullptr)
    , _bit_array_size(0)
    , _num_hole(0)
    , _freelist_head(npos)
    , _sparse_size(0)
    , _capacity(0)
    , _data(nullptr)
    , _alloc(std::move(alloc))
{
    if (size)
    {
        // resize
        _resizeMemory(size);
        _sparse_size = size;
        _setBitRange(0, size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i].data) T();
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(SizeType size, const T& v, Alloc alloc)
    : _bit_array(nullptr)
    , _bit_array_size(0)
    , _num_hole(0)
    , _freelist_head(npos)
    , _sparse_size(0)
    , _capacity(0)
    , _data(nullptr)
    , _alloc(std::move(alloc))
{
    if (size)
    {
        // resize
        _resizeMemory(size);
        _sparse_size = size;
        _setBitRange(0, size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i].data) T(v);
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(const T* p, SizeType n, Alloc alloc)
    : _bit_array(nullptr)
    , _bit_array_size(0)
    , _num_hole(0)
    , _freelist_head(npos)
    , _sparse_size(0)
    , _capacity(0)
    , _data(nullptr)
    , _alloc(std::move(alloc))
{
    if (n)
    {
        // resize
        _resizeMemory(n);
        _sparse_size = n;
        _setBitRange(0, n, true);

        // call ctor
        for (SizeType i = 0; i < n; ++i)
        {
            new (&_data[i].data) T(p[i]);
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(std::initializer_list<T> init_list, Alloc alloc)
    : _bit_array(nullptr)
    , _bit_array_size(0)
    , _num_hole(0)
    , _freelist_head(npos)
    , _sparse_size(0)
    , _capacity(0)
    , _data(nullptr)
    , _alloc(std::move(alloc))
{
    SizeType size = init_list.size();
    if (size)
    {
        // resize
        _resizeMemory(size);
        _sparse_size = size;
        _setBitRange(0, size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i].data) T(*(init_list.begin() + i));
        }
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::~SparseArray() { release(); }

// copy & move
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE SparseArray<T, TBitBlock, Alloc>::SparseArray(const SparseArray& other, Alloc alloc)
    : _bit_array(nullptr)
    , _bit_array_size(0)
    , _num_hole(0)
    , _freelist_head(npos)
    , _sparse_size(0)
    , _capacity(0)
    , _data(nullptr)
    , _alloc(std::move(alloc))
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
        if constexpr (memory::memory_traits<T>::call_ctor)
        {
            for (SizeType i = 0; i < rhs._sparse_size; ++i)
            {
                DataType*       dst_data = _data + i;
                const DataType* src_data = rhs._data + i;

                if (rhs.hasData(i))
                {
                    new (dst_data) T(*src_data);
                }
                else
                {
                    dst_data->prev = src_data->prev;
                    dst_data->next = src_data->next;
                }
            }
        }
        else
        {
            // copy data
            std::memcpy(_data, rhs._data, sizeof(DataType) * rhs._sparse_size);
        }

        // copy bit array
        std::memcpy(_bit_array, rhs._bit_array, sizeof(TBitBlock) * Algo::numBlocks(rhs._sparse_size));

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
        _setBitRange(0, n, true);

        // call ctor
        for (SizeType i = 0; i < n; ++i)
        {
            new (&_data[i].data) T(p[i]);
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
        _setBitRange(0, size, true);

        // call ctor
        for (SizeType i = 0; i < size; ++i)
        {
            new (&_data[i].data) T(*(init_list.begin() + i));
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
            bool lhs_has_data = hasData(i);
            bool rhs_has_data = rhs.hasData(i);

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
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::sparseSize() const
{
    return _sparse_size;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::holeSize() const
{
    return _num_hole;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::bitArraySize() const
{
    return _bit_array_size;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::freelistHead() const
{
    return _freelist_head;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::isCompact() const
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
SKR_INLINE TBitBlock* SparseArray<T, TBitBlock, Alloc>::bitArray() { return _bit_array; }
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE const TBitBlock* SparseArray<T, TBitBlock, Alloc>::bitArray() const
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
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::hasData(SizeType idx) const
{
    return _getBit(idx);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::isHole(SizeType idx) const
{
    return !_getBit(idx);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::isValidIndex(SizeType idx) const
{
    return idx >= 0 && idx < _sparse_size;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::isValidPointer(const T* p) const
{
    return p >= reinterpret_cast<T*>(_data) && p < reinterpret_cast<T*>(_data + _sparse_size);
}

// memory op
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::clear()
{
    // destruct items
    if constexpr (memory::memory_traits<T>::call_dtor)
    {
        for (SizeType i = 0; i < _sparse_size; ++i)
        {
            if (hasData(i))
            {
                (_data + i)->data.~T();
            }
        }
    }

    // clean up bit array
    if (_bit_array)
    {
        Algo::setBlocks(_bit_array, SizeType(0), Algo::numBlocks(_bit_array_size), false);
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
    _resizeMemory(capacity);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::reserve(SizeType capacity)
{
    if (capacity > _capacity)
    {
        _resizeMemory(capacity);
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::shrink()
{
    auto new_capacity = _alloc.getShrink(_sparse_size, _capacity);
    _resizeMemory(new_capacity);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::compact()
{
    if (!isCompact())
    {
        // fill hole
        SizeType compacted_index = _sparse_size - _num_hole;
        SizeType search_index    = _sparse_size;
        while (_freelist_head != npos)
        {
            SizeType next_index = (_data + _freelist_head)->next;
            if (_freelist_head < compacted_index)
            {
                // find last allocated element
                do
                {
                    --search_index;
                } while (!hasData(search_index));

                // move element to the hole
                *(_data + _freelist_head) = std::move(*(_data + search_index));
                (_data + search_index)->data.~T();
            }
            _freelist_head = next_index;
        }

        // setup bit array
        _setBitRange(compacted_index, _num_hole, false);

        // setup data
        _num_hole    = 0;
        _sparse_size = compacted_index;

        return true;
    }
    else
    {
        return false;
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::compactStable()
{
    if (!isCompact())
    {
        SizeType compacted_index = _sparse_size - _num_hole;
        SizeType read_index      = 0;
        SizeType write_index     = 0;

        // skip first compacted range
        while (hasData(write_index) && write_index != _sparse_size)
            ++write_index;
        read_index = write_index + 1;

        // copy items
        while (read_index < _sparse_size)
        {
            // skip hole
            while (!hasData(read_index) && read_index < _sparse_size)
                ++read_index;

            // move items
            while (read_index < _sparse_size && hasData(read_index))
            {
                (_data + write_index)->data = std::move((_data + read_index)->data);
                (_data + read_index)->data.~T();
                _setBit(write_index, true);
                ++write_index;
                ++read_index;
            }
        }

        // reset data
        _setBitRange(compacted_index, _num_hole, false);
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
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::compactTop()
{
    if (!isCompact())
    {
        bool has_changes = false;
        for (SizeType i(_sparse_size - 1), n(_sparse_size); n; --i, --n)
        {
            DataType& data = _data[i];

            if (isHole(i))
            {
                // remove from freelist
                --_num_hole;
                if (_freelist_head == i)
                {
                    _freelist_head = data.next;
                }
                if (data.next != npos)
                {
                    _data[data.next].prev = data.prev;
                }
                if (data.prev != npos)
                {
                    _data[data.prev].next = data.next;
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
    DataRef info = addUnsafe();
    new (info.data) T(v);
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::add(T&& v)
{
    DataRef info = addUnsafe();
    new (info.data) T(std::move(v));
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::addUnsafe()
{
    SizeType index;

    if (_num_hole)
    {
        // remove and use first index from freelist
        index          = _freelist_head;
        _freelist_head = _data[_freelist_head].next;
        --_num_hole;

        // break link
        if (_num_hole)
        {
            _data[_freelist_head].prev = npos;
        }
    }
    else
    {
        // add new element
        index = _sparse_size;
        _grow(1);
    }

    // setup bit
    _setBit(index, true);

    return DataRef(&_data[index].data, index);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::addDefault()
{
    DataRef info = addUnsafe();
    new (info.data) T();
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::addZeroed()
{
    DataRef info = addUnsafe();
    std::memset(info.data, 0, sizeof(T));
    return info;
}

// add at
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::addAt(SizeType idx, const T& v)
{
    addAtUnsafe(idx);
    new (&_data[idx].data) T(v);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::addAt(SizeType idx, T&& v)
{
    addAtUnsafe(idx);
    new (&_data[idx].data) T(std::move(v));
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::addAtUnsafe(SizeType idx)
{
    SKR_ASSERT(isHole(idx));
    SKR_ASSERT(isValidIndex(idx));

    DataType& data = _data[idx];

    // remove from freelist
    --_num_hole;
    if (_freelist_head == idx)
    {
        _freelist_head = data.next;
    }
    if (data.next != npos)
    {
        _data[data.next].prev = data.prev;
    }
    if (data.prev != npos)
    {
        _data[data.prev].next = data.next;
    }

    // setup bit
    _setBit(idx, true);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::addAtDefault(SizeType idx)
{
    addAtUnsafe(idx);
    new (&_data[idx].data) T();
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::addAtZeroed(SizeType idx)
{
    addAtUnsafe(idx);
    std::memset(&_data[idx].data, 0, sizeof(T));
}

// emplace
template <typename T, typename TBitBlock, typename Alloc>
template <typename... Args>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::emplace(Args&&... args)
{
    DataRef info = addUnsafe();
    new (info.data) T(std::forward<Args>(args)...);
    return info;
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename... Args>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::emplaceAt(SizeType index, Args&&... args)
{
    addAtUnsafe(index);
    new (&_data[index].data) T(std::forward<Args>(args)...);
}

// append
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::append(const SparseArray& arr)
{
    for (const T& data : arr)
    {
        add(data);
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::append(std::initializer_list<T> init_list)
{
    for (const T& data : init_list)
    {
        add(data);
    }
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::append(T* p, SizeType n)
{
    for (SizeType i = 0; i < n; ++i)
    {
        add(p[i]);
    }
}

// remove
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::removeAt(SizeType index, SizeType n)
{
    SKR_ASSERT(isValidIndex(index));
    SKR_ASSERT(isValidIndex(index + n - 1));
    SKR_ASSERT(n > 0);

    if constexpr (memory::memory_traits<T>::call_dtor)
    {
        for (SizeType i = 0; i < n; ++i)
        {
            _data[index + i].data.~T();
        }
    }

    removeAtUnsafe(index, n);
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::removeAtUnsafe(SizeType index, SizeType n)
{
    SKR_ASSERT(isValidIndex(index));
    SKR_ASSERT(isValidIndex(index + n - 1));
    SKR_ASSERT(n > 0);

    for (; n; --n)
    {
        DataType& data = _data[index];

        // link to freelist
        if (_num_hole)
        {
            _data[_freelist_head].prev = index;
        }
        data.prev      = npos;
        data.next      = _freelist_head;
        _freelist_head = index;
        ++_num_hole;

        // set flag
        _setBit(index, false);

        // update index
        ++index;
    }
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::remove(const TK& v)
{
    return removeIf([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::removeLast(const TK& v)
{
    return removeLastIf([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::removeAll(const TK& v)
{
    return removeAllIf([&v](const T& a) { return a == v; });
}

// remove if
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::removeIf(TP&& p)
{
    if (DataRef ref = findIf(std::forward<TP>(p)))
    {
        removeAt(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::removeLastIf(TP&& p)
{
    if (DataRef ref = findLastIf(std::forward<TP>(p)))
    {
        removeAt(ref.index);
        return ref;
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::SizeType SparseArray<T, TBitBlock, Alloc>::removeAllIf(TP&& p)
{
    SizeType count = 0;
    for (SizeType i = 0; i < _sparse_size; ++i)
    {
        if (hasData(i) && p(_data[i].data))
        {
            removeAt(i);
            ++count;
        }
    }
    return count;
}

// modify
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE T& SparseArray<T, TBitBlock, Alloc>::operator[](SizeType index)
{
    SKR_ASSERT(isValidIndex(index));
    return _data[index].data;
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE const T& SparseArray<T, TBitBlock, Alloc>::operator[](SizeType index) const
{
    SKR_ASSERT(isValidIndex(index));
    return _data[index].data;
}

// find
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::find(const TK& v)
{
    return findIf([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::findLast(const TK& v)
{
    return findLastIf([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::find(const TK& v) const
{
    return findIf([&v](const T& a) { return a == v; });
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TK>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::findLast(const TK& v) const
{
    return findLastIf([&v](const T& a) { return a == v; });
}

// find if
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::findIf(TP&& p)
{
    for (SizeType i = 0; i < _sparse_size; ++i)
    {
        if (hasData(i))
        {
            auto& data = _data[i].data;
            if (p(data))
            {
                return DataRef(&data, i);
            }
        }
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::DataRef SparseArray<T, TBitBlock, Alloc>::findLastIf(TP&& p)
{
    for (SizeType i(_sparse_size - 1), n(_sparse_size); n; --i, --n)
    {
        if (hasData(i))
        {
            auto& data = _data[i].data;
            if (p(data))
            {
                return DataRef(&data, i);
            }
        }
    }
    return DataRef();
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::findIf(TP&& p) const
{
    for (SizeType i = 0; i < _sparse_size; ++i)
    {
        if (hasData(i))
        {
            auto& data = _data[i].data;
            if (p(data))
            {
                return CDataRef(&data, i);
            }
        }
    }
    return CDataRef();
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CDataRef SparseArray<T, TBitBlock, Alloc>::findLastIf(TP&& p) const
{
    for (SizeType i(_sparse_size - 1), n(_sparse_size); n; --i, --n)
    {
        if (hasData(i))
        {
            auto& data = _data[i].data;
            if (p(data))
            {
                return CDataRef(&data, i);
            }
        }
    }
    return CDataRef();
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
SKR_INLINE bool SparseArray<T, TBitBlock, Alloc>::containIf(TP&& p) const
{
    return (bool)findIf(std::forward<TP>(p));
}

// sort
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::sort(TP&& p)
{
    if (_sparse_size)
    {
        compact();
        algo::intro_sort(_data, _data + _sparse_size, [&p](const DataType& a, const DataType& b) { return p(a.data, b.data); });
    }
}
template <typename T, typename TBitBlock, typename Alloc>
template <typename TP>
SKR_INLINE void SparseArray<T, TBitBlock, Alloc>::sortStable(TP&& p)
{
    if (_sparse_size)
    {
        compactStable();
        algo::merge_sort(_data, _data + _sparse_size, [&p](const DataType& a, const DataType& b) { return p(a.data, b.data); });
    }
}

// support foreach
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::It SparseArray<T, TBitBlock, Alloc>::begin()
{
    return It(data(), sparseSize(), bitArray());
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::It SparseArray<T, TBitBlock, Alloc>::end()
{
    return It(data(), sparseSize(), bitArray(), sparseSize());
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CIt SparseArray<T, TBitBlock, Alloc>::begin() const
{
    return CIt(data(), sparseSize(), bitArray());
}
template <typename T, typename TBitBlock, typename Alloc>
SKR_INLINE typename SparseArray<T, TBitBlock, Alloc>::CIt SparseArray<T, TBitBlock, Alloc>::end() const
{
    return CIt(data(), sparseSize(), bitArray(), sparseSize());
}
} // namespace skr