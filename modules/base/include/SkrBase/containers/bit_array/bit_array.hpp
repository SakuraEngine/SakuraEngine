#pragma once
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/config.h"
#include "SkrBase/memory/memory_ops.hpp"
#include "SkrBase/algo/bit_algo.hpp"
#include "bit_ref.hpp"
#include "bit_iterator.hpp"

// BitArray def
// TODO. 包装一个更安全的 SizeType 作为查找返回
namespace skr
{
template <typename TBlock, typename Alloc>
struct BitArray final {
    using SizeType = typename Alloc::SizeType;
    using Algo     = algo::BitAlgo<TBlock>;
    using It       = BitIt<TBlock, SizeType, false>;
    using CIt      = BitIt<TBlock, SizeType, true>;
    using TIt      = TrueBitIt<TBlock, SizeType, true>;

    // ctor & dtor
    BitArray(Alloc alloc = {});
    BitArray(SizeType size, bool v, Alloc alloc = {});
    ~BitArray();

    // copy & move ctor
    BitArray(const BitArray& other, Alloc alloc = {});
    BitArray(BitArray&& other) noexcept;

    // copy & move assign
    BitArray& operator=(const BitArray& rhs);
    BitArray& operator=(BitArray&& rhs) noexcept;

    // compare
    bool operator==(const BitArray& rhs) const;
    bool operator!=(const BitArray& rhs) const;

    // getter
    TBlock*       data();
    const TBlock* data() const;
    SizeType      size() const;
    SizeType      capacity() const;
    Alloc&        allocator();
    const Alloc&  allocator() const;
    bool          empty();

    // validate
    bool is_valid_index(SizeType idx);

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void resize(SizeType size, bool new_value);
    void resize_unsafe(SizeType size);

    // add
    SizeType add(bool v);
    SizeType add(bool v, SizeType n);

    // remove
    void remove_at(SizeType start, SizeType n = 1);
    void remove_at_swap(SizeType start, SizeType n = 1);

    // modify
    BitRef<TBlock> operator[](SizeType idx);
    bool           operator[](SizeType idx) const;

    // find
    SizeType find(bool v) const;
    SizeType find_last(bool v) const;

    // contain
    bool contain(bool v) const;

    // set range
    void set_range(SizeType start, SizeType n, bool v);

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;

private:
    // helper
    void _realloc(SizeType new_capacity);
    void _free();
    void _grow(SizeType size);

private:
    TBlock*  _data     = nullptr;
    SizeType _size     = 0;
    SizeType _capacity = 0;
    Alloc    _alloc    = {};
};
} // namespace skr

// BitArray impl
namespace skr
{
// helper
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::_realloc(SizeType new_capacity)
{
    SKR_ASSERT(new_capacity != _capacity);
    SKR_ASSERT(new_capacity > 0);
    SKR_ASSERT(_size <= new_capacity);
    SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

    SizeType old_block_capacity = Algo::num_blocks(_capacity);
    SizeType new_block_capacity = Algo::num_blocks(new_capacity);

    SKR_ASSERT(new_block_capacity > 0);

    if (new_block_capacity > old_block_capacity)
    {
        if constexpr (memory::memory_traits<TBlock>::use_realloc)
        {
            _data     = _alloc.template realloc<TBlock>(_data, new_capacity);
            _capacity = new_capacity;
        }
        else
        {
            // alloc new memory
            TBlock* new_memory = _alloc.template alloc<TBlock>(new_capacity);

            // move items
            if (_size)
            {
                memory::move(new_memory, _data, Algo::num_blocks(_size));
            }

            // release old memory
            _alloc.template free<TBlock>(_data);

            _data     = new_memory;
            _capacity = new_capacity;
        }
    }
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::_free()
{
    if (_data)
    {
        _alloc.template free<TBlock>(_data);
        _data     = nullptr;
        _capacity = 0;
    }
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::_grow(SizeType size)
{
    if (_size + size > _capacity)
    {
        // calc new capacity
        SizeType old_block_capacity = Algo::num_blocks(_capacity);
        SizeType new_block_size     = Algo::num_blocks(_size + size);
        SizeType new_block_capacity = _alloc.get_grow(new_block_size, old_block_capacity);

        // realloc
        _realloc(new_block_capacity);

        // update capacity
        _capacity = new_block_capacity << Algo::PerBlockSizeLog2;
    }

    // update size
    _size += size;
}

// ctor & dtor
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(Alloc alloc)
    : _alloc(std::move(alloc))
{
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(SizeType size, bool v, Alloc alloc)
    : _alloc(std::move(alloc))
{
    resize(size, v);
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::~BitArray() { release(); }

// copy & move ctor
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(const BitArray& other, Alloc alloc)
    : _alloc(std::move(alloc))
{
    // resize
    _realloc(other._capacity);

    // copy
    _size = other.size();
    if (other._size)
        std::memcpy(_data, other._data, Algo::num_blocks(_size) * sizeof(TBlock));
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(BitArray&& other) noexcept
    : _data(other._data)
    , _size(other._size)
    , _capacity(other._capacity)
    , _alloc(std::move(other._alloc))
{
    other._data     = nullptr;
    other._size     = 0;
    other._capacity = 0;
}

// copy & move assign
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>& BitArray<TBlock, Alloc>::operator=(const BitArray& rhs)
{
    if (this != &rhs)
    {
        _realloc(rhs._size);
        _size = rhs._size;
        if (_size)
            std::memcpy(_data, rhs._data, Algo::num_blocks(_size) * sizeof(TBlock));
    }

    return *this;
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>& BitArray<TBlock, Alloc>::operator=(BitArray&& rhs) noexcept
{
    if (this != &rhs)
    {
        // release
        release();

        // copy data
        _data     = rhs._data;
        _size     = rhs._size;
        _capacity = rhs._capacity;
        _alloc    = std::move(rhs._alloc);

        // invalidate rhs
        rhs._data     = nullptr;
        rhs._size     = 0;
        rhs._capacity = 0;
    }

    return *this;
}

// compare
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::operator==(const BitArray& rhs) const
{
    if (_size != rhs._size)
        return false;

    auto block_count   = int_div_floor(_size, (SizeType)Algo::PerBlockSize);
    auto last_mask     = Algo::last_block_mask(_size);
    bool memcmp_result = std::memcmp(_data, rhs._data, block_count * sizeof(TBlock)) == 0;
    bool last_result   = last_mask == Algo::FullMask || (_data[block_count] & last_mask) == (rhs._data[block_count] & last_mask);
    return memcmp_result && last_result;
}
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::operator!=(const BitArray& rhs) const { return !(*this == rhs); }

// getter
template <typename TBlock, typename Alloc>
SKR_INLINE TBlock* BitArray<TBlock, Alloc>::data() { return _data; }
template <typename TBlock, typename Alloc>
SKR_INLINE const TBlock* BitArray<TBlock, Alloc>::data() const { return _data; }
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::size() const
{
    return _size;
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::capacity() const
{
    return _capacity;
}
template <typename TBlock, typename Alloc>
SKR_INLINE Alloc& BitArray<TBlock, Alloc>::allocator() { return _alloc; }
template <typename TBlock, typename Alloc>
SKR_INLINE const Alloc& BitArray<TBlock, Alloc>::allocator() const { return _alloc; }
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::empty() { return _size == 0; }

// validate
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::is_valid_index(SizeType idx) { return idx >= 0 && idx < _size; }

// memory op
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::clear()
{
    Algo::set_blocks(_data, SizeType(0), Algo::num_blocks(_size), false);
    _size = 0;
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::release(SizeType capacity)
{
    _size = 0;
    if (capacity)
    {
        _realloc(capacity);
    }
    else
    {
        _free();
    }
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::reserve(SizeType capacity)
{
    if (capacity > _capacity)
    {
        _realloc(capacity);
    }
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::resize(SizeType size, bool new_value)
{
    // realloc memory if need
    if (size > _capacity)
    {
        _realloc(size);
    }

    // init if need
    if (size > _size)
    {
        Algo::set_range(_data, _size, size - _size, new_value);
    }

    // set size
    _size = size;
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::resize_unsafe(SizeType size)
{
    // do resize if need
    if (size > _capacity)
    {
        _realloc(size);
    }

    // set size
    _size = size;
}

// add
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::add(bool v)
{
    // do grow
    auto old_size = _size;
    _grow(1);

    // set value
    (*this)[old_size] = v;
    return old_size;
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::add(bool v, SizeType n)
{
    // do grow
    auto old_size = _size;
    _grow(n);

    // set value
    set_range(old_size, n, v);
    return old_size;
}

// remove
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::remove_at(SizeType start, SizeType n)
{
    SKR_ASSERT(start >= 0 && n > 0 && start + n < _size);
    if (start + n != _size)
    {
        It write(data(), size(), start);
        It read(data(), size(), start + n);

        while (read)
        {
            *write = *read;
            ++write;
            ++read;
        }
    }
    _size -= n;
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::remove_at_swap(SizeType start, SizeType n)
{
    SKR_ASSERT(start >= 0 && n > 0 && start + n < _size);
    if (start + n != _size)
    {
        // adjust n
        auto move_n = std::min(n, _size - start - n);

        // copy items
        for (SizeType i = 0; i < move_n; ++i)
        {
            (*this)[start + i] = (*this)[_size - move_n + i];
        }
    }
    _size -= n;
}

// modify
template <typename TBlock, typename Alloc>
SKR_INLINE BitRef<TBlock> BitArray<TBlock, Alloc>::operator[](SizeType idx)
{
    SKR_ASSERT(is_valid_index(idx));
    return BitRef<TBlock>(_data[idx >> Algo::PerBlockSizeLog2], TBlock(1) << (idx & Algo::PerBlockSizeMask));
}
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::operator[](SizeType idx) const
{
    SKR_ASSERT(is_valid_index(idx));
    return _data[idx >> Algo::PerBlockSizeLog2] & (1 << (idx & Algo::PerBlockSizeMask));
}

// find
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::find(bool v) const
{
    return Algo::find(_data, SizeType(0), _size, v);
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::find_last(bool v) const
{
    return Algo::find_last(_data, SizeType(0), _size, v);
}

// contain
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::contain(bool v) const { return find(v) != npos_of<SizeType>; }

// set range
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::set_range(SizeType start, SizeType n, bool v)
{
    SKR_ASSERT(start >= 0 && n > 0 && start + n <= _size);
    Algo::set_range(_data, start, n, v);
}

// support foreach
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::It BitArray<TBlock, Alloc>::begin()
{
    return It(data(), size());
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::It BitArray<TBlock, Alloc>::end()
{
    return It(data(), size(), _size);
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::CIt BitArray<TBlock, Alloc>::begin() const
{
    return CIt(data(), size());
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::CIt BitArray<TBlock, Alloc>::end() const
{
    return CIt(data(), size(), _size);
}

} // namespace skr
