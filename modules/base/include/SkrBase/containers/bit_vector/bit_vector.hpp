#pragma once
#include "SkrBase/config.h"
#include "SkrBase/memory/memory_ops.hpp"
#include "SkrBase/algo/bit_algo.hpp"
#include "SkrBase/containers/bit_tools/bit_ref.hpp"
#include "SkrBase/containers/bit_tools/bit_iterator.hpp"
#include <algorithm>

// BitVector def
namespace skr::container
{
template <typename Memory>
struct BitVector final : protected Memory {
    using typename Memory::BitBlockType;
    using typename Memory::SizeType;
    using typename Memory::AllocatorCtorParam;

    // data ref
    using DataRef = BitDataRef<BitBlockType, SizeType>;
    using BitRef  = BitRef<BitBlockType>;

    // cursor & iterator
    using Cursor   = BitCursor<BitBlockType, SizeType, false>;
    using CCursor  = BitCursor<BitBlockType, SizeType, true>;
    using Iter     = BitIter<BitBlockType, SizeType, false>;
    using CIter    = BitIter<BitBlockType, SizeType, true>;
    using IterInv  = BitIterInv<BitBlockType, SizeType, false>;
    using CIterInv = BitIterInv<BitBlockType, SizeType, true>;

    // stl style iterator
    using StlIt  = CursorIterStl<BitCursor<BitBlockType, SizeType, false>, false>;
    using CStlIt = CursorIterStl<BitCursor<BitBlockType, SizeType, true>, false>;

    using Algo = algo::BitAlgo<BitBlockType>;

    // ctor & dtor
    BitVector(AllocatorCtorParam param = {});
    BitVector(SizeType size, bool v, AllocatorCtorParam param = {});
    ~BitVector();

    // copy & move ctor
    BitVector(const BitVector& other);
    BitVector(BitVector&& other) noexcept;

    // copy & move assign
    BitVector& operator=(const BitVector& rhs);
    BitVector& operator=(BitVector&& rhs) noexcept;

    // compare
    bool operator==(const BitVector& rhs) const;
    bool operator!=(const BitVector& rhs) const;

    // getter
    BitBlockType*       data();
    const BitBlockType* data() const;
    SizeType            size() const;
    SizeType            capacity() const;
    bool                empty();
    Memory&             memory();
    const Memory&       memory() const;

    // validate
    bool is_valid_index(SizeType idx);

    // memory op
    void clear();
    void release(SizeType reserve_capacity = 0);
    void reserve(SizeType expect_capacity);
    void resize(SizeType expect_size, bool new_value);
    void resize_unsafe(SizeType expect_size);

    // add
    DataRef add(bool v);
    DataRef add(bool v, SizeType n);

    // remove
    DataRef remove(bool v);
    DataRef remove_last(bool v);
    DataRef remove_swap(bool v);
    DataRef remove_last_swap(bool v);
    void    remove_at(SizeType start, SizeType n = 1);
    void    remove_at_swap(SizeType start, SizeType n = 1);

    // modify
    BitRef operator[](SizeType idx);
    bool   operator[](SizeType idx) const;

    // find
    DataRef find(bool v) const;
    DataRef find_last(bool v) const;

    // contains
    bool contains(bool v) const;

    // set range
    void set_range(SizeType start, SizeType n, bool v);

    // cursor & iterator
    // auto cursor_begin();
    // auto cursor_begin() const;
    // auto cursor_end();
    // auto cursor_end() const;
    // auto iter();
    // auto iter() const;
    // auto iter_inv();
    // auto iter_inv() const;

    // true cursor & iterator
    // auto true_cursor_begin();
    // auto true_cursor_begin() const;
    // auto true_cursor_end();
    // auto true_cursor_end() const;
    // auto true_iter();
    // auto true_iter() const;
    // auto true_iter_inv();
    // auto true_iter_inv() const;

    // false cursor & iterator
    // auto false_cursor_begin();
    // auto false_cursor_begin() const;
    // auto false_cursor_end();
    // auto false_cursor_end() const;
    // auto false_iter();
    // auto false_iter() const;
    // auto false_iter_inv();
    // auto false_iter_inv() const;

    // syntax
    const BitVector& readOnly() const;

private:
    // helper
    void   _realloc(SizeType new_bit_capacity);
    void   _free();
    void   _grow(SizeType grow_size);
    BitRef _bit_ref_at(SizeType idx) const;
    void   _set_size(SizeType value);
};
} // namespace skr::container

// BitVector impl
namespace skr::container
{
// helper
template <typename Memory>
SKR_INLINE void BitVector<Memory>::_realloc(SizeType new_bit_capacity)
{
    SizeType new_block_capacity = Algo::num_blocks(new_bit_capacity);
    SizeType old_block_capacity = Algo::num_blocks(capacity());
    if (new_block_capacity != old_block_capacity)
    {
        Memory::realloc(new_block_capacity);
    }
}
template <typename Memory>
SKR_INLINE void BitVector<Memory>::_free()
{
    Memory::free();
}
template <typename Memory>
SKR_INLINE void BitVector<Memory>::_grow(SizeType grow_size)
{
    Memory::grow(grow_size);
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::BitRef BitVector<Memory>::_bit_ref_at(SizeType idx) const
{
    return BitRef::At(const_cast<BitBlockType*>(data()), idx);
}
template <typename Memory>
void BitVector<Memory>::_set_size(SizeType value)
{
    Memory::set_size(value);
}

// ctor & dtor
template <typename Memory>
SKR_INLINE BitVector<Memory>::BitVector(AllocatorCtorParam param)
    : Memory(std::move(param))
{
}
template <typename Memory>
SKR_INLINE BitVector<Memory>::BitVector(SizeType size, bool v, AllocatorCtorParam param)
    : Memory(std::move(param))
{
    resize(size, v);
}
template <typename Memory>
SKR_INLINE BitVector<Memory>::~BitVector()
{
    // handled in memory
}

// copy & move ctor
template <typename Memory>
SKR_INLINE BitVector<Memory>::BitVector(const BitVector& other)
    : Memory(other)
{
    // handled in memory
}
template <typename Memory>
SKR_INLINE BitVector<Memory>::BitVector(BitVector&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// copy & move assign
template <typename Memory>
SKR_INLINE BitVector<Memory>& BitVector<Memory>::operator=(const BitVector& rhs)
{
    Memory::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE BitVector<Memory>& BitVector<Memory>::operator=(BitVector&& rhs) noexcept
{
    Memory::operator=(std::move(rhs));
    return *this;
}

// compare
template <typename Memory>
SKR_INLINE bool BitVector<Memory>::operator==(const BitVector& rhs) const
{
    if (size() != rhs.size())
        return false;

    auto block_count   = int_div_floor(size(), (SizeType)Algo::PerBlockSize);
    auto last_mask     = Algo::last_block_mask(size());
    bool memcmp_result = memory::compare<BitBlockType>(data(), rhs.data(), block_count);
    bool last_result   = last_mask == Algo::FullMask || (data()[block_count] & last_mask) == (rhs.data()[block_count] & last_mask);
    return memcmp_result && last_result;
}
template <typename Memory>
SKR_INLINE bool BitVector<Memory>::operator!=(const BitVector& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::BitBlockType* BitVector<Memory>::data()
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE const typename BitVector<Memory>::BitBlockType* BitVector<Memory>::data() const
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::SizeType BitVector<Memory>::size() const
{
    return Memory::size();
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::SizeType BitVector<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
SKR_INLINE bool BitVector<Memory>::empty()
{
    return size() == 0;
}
template <typename Memory>
SKR_INLINE Memory& BitVector<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& BitVector<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
SKR_INLINE bool BitVector<Memory>::is_valid_index(SizeType idx)
{
    return idx >= 0 && idx < size();
}

// memory op
template <typename Memory>
SKR_INLINE void BitVector<Memory>::clear()
{
    Memory::clear();
}
template <typename Memory>
SKR_INLINE void BitVector<Memory>::release(SizeType reserve_capacity)
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
SKR_INLINE void BitVector<Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        _realloc(expect_capacity);
    }
}
template <typename Memory>
SKR_INLINE void BitVector<Memory>::resize(SizeType expect_size, bool new_value)
{
    // realloc memory if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // init if need
    if (expect_size > size())
    {
        Algo::set_range(data(), size(), expect_size - size(), new_value);
    }

    // set size
    _set_size(expect_size);
}
template <typename Memory>
SKR_INLINE void BitVector<Memory>::resize_unsafe(SizeType expect_size)
{
    // do resize if need
    if (expect_size > capacity())
    {
        _realloc(expect_size);
    }

    // set size
    _set_size(expect_size);
}

// add
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::add(bool v)
{
    // do grow
    auto old_size = size();
    _grow(1);

    // set value
    (*this)[old_size] = v;

    return { _bit_ref_at(old_size), old_size };
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::add(bool v, SizeType n)
{
    // do grow
    auto old_size = size();
    _grow(n);

    // set value
    set_range(old_size, n, v);

    return { _bit_ref_at(old_size), old_size };
}

// remove
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::remove(bool v)
{
    if (auto result = find(v))
    {
        remove_at(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::remove_last(bool v)
{
    if (auto result = find_last(v))
    {
        remove_at(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::remove_swap(bool v)
{
    if (auto result = find(v))
    {
        remove_at_swap(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::remove_last_swap(bool v)
{
    if (auto result = find_last(v))
    {
        remove_at_swap(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE void BitVector<Memory>::remove_at(SizeType start, SizeType n)
{
    SKR_ASSERT(start >= 0 && n > 0 && start + n < size());
    if (start + n != size())
    {
        Cursor write = { data(), size(), start };
        Cursor read  = { data(), size(), start + n };

        while (!read.reach_end())
        {
            write.ref() = read.ref();
            write.move_next();
            read.move_next();
        }
    }
    _set_size(size() - n);
}
template <typename Memory>
SKR_INLINE void BitVector<Memory>::remove_at_swap(SizeType start, SizeType n)
{
    SKR_ASSERT(start >= 0 && n > 0 && start + n < size());
    if (start + n != size())
    {
        // adjust n
        auto move_n = std::min(n, size() - start - n);

        // copy items
        for (SizeType i = 0; i < move_n; ++i)
        {
            (*this)[start + i] = (*this)[size() - move_n + i];
        }
    }
    _set_size(size() - n);
}

// modify
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::BitRef BitVector<Memory>::operator[](SizeType idx)
{
    SKR_ASSERT(is_valid_index(idx));
    return _bit_ref_at(idx);
}
template <typename Memory>
SKR_INLINE bool BitVector<Memory>::operator[](SizeType idx) const
{
    SKR_ASSERT(is_valid_index(idx));
    return data()[idx >> Algo::PerBlockSizeLog2] & (1 << (idx & Algo::PerBlockSizeMask));
}

// find
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::find(bool v) const
{
    auto result = Algo::find(data(), SizeType(0), size(), v);
    return result == npos_of<SizeType> ? DataRef{} : DataRef{ _bit_ref_at(result), result };
}
template <typename Memory>
SKR_INLINE typename BitVector<Memory>::DataRef BitVector<Memory>::find_last(bool v) const
{
    auto result = Algo::find_last(data(), SizeType(0), size(), v);
    return result == npos_of<SizeType> ? DataRef{} : DataRef{ _bit_ref_at(result), result };
}

// contains
template <typename Memory>
SKR_INLINE bool BitVector<Memory>::contains(bool v) const { return find(v) != npos_of<SizeType>; }

// set range
template <typename Memory>
SKR_INLINE void BitVector<Memory>::set_range(SizeType start, SizeType n, bool v)
{
    SKR_ASSERT(start >= 0 && n > 0 && start + n <= size());
    Algo::set_range(data(), start, n, v);
}

// syntax
template <typename Memory>
SKR_INLINE const BitVector<Memory>& BitVector<Memory>::readOnly() const
{
    return *this;
}

} // namespace skr::container
