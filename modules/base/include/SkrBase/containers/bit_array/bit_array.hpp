#pragma once
#include "SkrBase/config.h"
#include "SkrBase/memory/memory_ops.hpp"
#include "SkrBase/algo/bit_algo.hpp"
#include "bit_ref.hpp"
#include "bit_iterator.hpp"
#include <algorithm>

// BitArray def
namespace skr::container
{
template <typename Memory>
struct BitArray final : protected Memory {
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
    BitArray(AllocatorCtorParam param = {});
    BitArray(SizeType size, bool v, AllocatorCtorParam param = {});
    ~BitArray();

    // copy & move ctor
    BitArray(const BitArray& other);
    BitArray(BitArray&& other) noexcept;

    // copy & move assign
    BitArray& operator=(const BitArray& rhs);
    BitArray& operator=(BitArray&& rhs) noexcept;

    // compare
    bool operator==(const BitArray& rhs) const;
    bool operator!=(const BitArray& rhs) const;

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

    // support foreach
    StlIt  begin();
    StlIt  end();
    CStlIt begin() const;
    CStlIt end() const;

    // syntax
    const BitArray& readOnly() const;

private:
    // helper
    void   _realloc(SizeType new_bit_capacity);
    void   _free();
    void   _grow(SizeType grow_size);
    BitRef _bit_ref_at(SizeType idx) const;
    void   _set_size(SizeType value);
};
} // namespace skr::container

// BitArray impl
namespace skr::container
{
// helper
template <typename Memory>
SKR_INLINE void BitArray<Memory>::_realloc(SizeType new_bit_capacity)
{
    SizeType new_block_capacity = Algo::num_blocks(new_bit_capacity);
    SizeType old_block_capacity = Algo::num_blocks(capacity());
    if (new_block_capacity != old_block_capacity)
    {
        Memory::realloc(new_block_capacity);
    }
}
template <typename Memory>
SKR_INLINE void BitArray<Memory>::_free()
{
    Memory::free();
}
template <typename Memory>
SKR_INLINE void BitArray<Memory>::_grow(SizeType grow_size)
{
    Memory::grow(grow_size);
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::BitRef BitArray<Memory>::_bit_ref_at(SizeType idx) const
{
    return BitRef::At(const_cast<BitBlockType*>(data()), idx);
}
template <typename Memory>
void BitArray<Memory>::_set_size(SizeType value)
{
    Memory::set_size(value);
}

// ctor & dtor
template <typename Memory>
SKR_INLINE BitArray<Memory>::BitArray(AllocatorCtorParam param)
    : Memory(std::move(param))
{
}
template <typename Memory>
SKR_INLINE BitArray<Memory>::BitArray(SizeType size, bool v, AllocatorCtorParam param)
    : Memory(std::move(param))
{
    resize(size, v);
}
template <typename Memory>
SKR_INLINE BitArray<Memory>::~BitArray()
{
    // handled in memory
}

// copy & move ctor
template <typename Memory>
SKR_INLINE BitArray<Memory>::BitArray(const BitArray& other)
    : Memory(other)
{
    // handled in memory
}
template <typename Memory>
SKR_INLINE BitArray<Memory>::BitArray(BitArray&& other) noexcept
    : Memory(std::move(other))
{
    // handled in memory
}

// copy & move assign
template <typename Memory>
SKR_INLINE BitArray<Memory>& BitArray<Memory>::operator=(const BitArray& rhs)
{
    Memory::operator=(rhs);
    return *this;
}
template <typename Memory>
SKR_INLINE BitArray<Memory>& BitArray<Memory>::operator=(BitArray&& rhs) noexcept
{
    Memory::operator=(std::move(rhs));
    return *this;
}

// compare
template <typename Memory>
SKR_INLINE bool BitArray<Memory>::operator==(const BitArray& rhs) const
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
SKR_INLINE bool BitArray<Memory>::operator!=(const BitArray& rhs) const
{
    return !(*this == rhs);
}

// getter
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::BitBlockType* BitArray<Memory>::data()
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE const typename BitArray<Memory>::BitBlockType* BitArray<Memory>::data() const
{
    return Memory::data();
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::SizeType BitArray<Memory>::size() const
{
    return Memory::size();
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::SizeType BitArray<Memory>::capacity() const
{
    return Memory::capacity();
}
template <typename Memory>
SKR_INLINE bool BitArray<Memory>::empty()
{
    return size() == 0;
}
template <typename Memory>
SKR_INLINE Memory& BitArray<Memory>::memory()
{
    return *this;
}
template <typename Memory>
SKR_INLINE const Memory& BitArray<Memory>::memory() const
{
    return *this;
}

// validate
template <typename Memory>
SKR_INLINE bool BitArray<Memory>::is_valid_index(SizeType idx)
{
    return idx >= 0 && idx < size();
}

// memory op
template <typename Memory>
SKR_INLINE void BitArray<Memory>::clear()
{
    Memory::clear();
}
template <typename Memory>
SKR_INLINE void BitArray<Memory>::release(SizeType reserve_capacity)
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
SKR_INLINE void BitArray<Memory>::reserve(SizeType expect_capacity)
{
    if (expect_capacity > capacity())
    {
        _realloc(expect_capacity);
    }
}
template <typename Memory>
SKR_INLINE void BitArray<Memory>::resize(SizeType expect_size, bool new_value)
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
SKR_INLINE void BitArray<Memory>::resize_unsafe(SizeType expect_size)
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
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::add(bool v)
{
    // do grow
    auto old_size = size();
    _grow(1);

    // set value
    (*this)[old_size] = v;

    return { _bit_ref_at(old_size), old_size };
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::add(bool v, SizeType n)
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
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::remove(bool v)
{
    if (auto result = find(v))
    {
        remove_at(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::remove_last(bool v)
{
    if (auto result = find_last(v))
    {
        remove_at(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::remove_swap(bool v)
{
    if (auto result = find(v))
    {
        remove_at_swap(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::remove_last_swap(bool v)
{
    if (auto result = find_last(v))
    {
        remove_at_swap(result.index);
        return result;
    }
    return {};
}
template <typename Memory>
SKR_INLINE void BitArray<Memory>::remove_at(SizeType start, SizeType n)
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
SKR_INLINE void BitArray<Memory>::remove_at_swap(SizeType start, SizeType n)
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
SKR_INLINE typename BitArray<Memory>::BitRef BitArray<Memory>::operator[](SizeType idx)
{
    SKR_ASSERT(is_valid_index(idx));
    return _bit_ref_at(idx);
}
template <typename Memory>
SKR_INLINE bool BitArray<Memory>::operator[](SizeType idx) const
{
    SKR_ASSERT(is_valid_index(idx));
    return data()[idx >> Algo::PerBlockSizeLog2] & (1 << (idx & Algo::PerBlockSizeMask));
}

// find
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::find(bool v) const
{
    auto result = Algo::find(data(), SizeType(0), size(), v);
    return result == npos_of<SizeType> ? DataRef{} : DataRef{ _bit_ref_at(result), result };
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::DataRef BitArray<Memory>::find_last(bool v) const
{
    auto result = Algo::find_last(data(), SizeType(0), size(), v);
    return result == npos_of<SizeType> ? DataRef{} : DataRef{ _bit_ref_at(result), result };
}

// contains
template <typename Memory>
SKR_INLINE bool BitArray<Memory>::contains(bool v) const { return find(v) != npos_of<SizeType>; }

// set range
template <typename Memory>
SKR_INLINE void BitArray<Memory>::set_range(SizeType start, SizeType n, bool v)
{
    SKR_ASSERT(start >= 0 && n > 0 && start + n <= size());
    Algo::set_range(data(), start, n, v);
}

// support foreach
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::StlIt BitArray<Memory>::begin()
{
    return { Cursor::Begin(data(), size()) };
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::StlIt BitArray<Memory>::end()
{
    return { Cursor::EndOverflow(data(), size()) };
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::CStlIt BitArray<Memory>::begin() const
{
    return { Cursor::End(data(), size()) };
}
template <typename Memory>
SKR_INLINE typename BitArray<Memory>::CStlIt BitArray<Memory>::end() const
{
    return { Cursor::BeginOverflow(data(), size()) };
}

// syntax
template <typename Memory>
SKR_INLINE const BitArray<Memory>& BitArray<Memory>::readOnly() const
{
    return *this;
}

} // namespace skr::container
