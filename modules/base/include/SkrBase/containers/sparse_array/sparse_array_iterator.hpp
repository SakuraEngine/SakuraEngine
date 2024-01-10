#pragma once
#include "SkrBase/config.h"
#include "sparse_array_def.hpp"
#include "SkrBase/containers/bit_array/bit_iterator.hpp"

// SparseArray iterator
namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, bool Const>
struct SparseArrayIt {
    using DataType  = std::conditional_t<Const, const SparseArrayData<T, TS>, SparseArrayData<T, TS>>;
    using ValueType = std::conditional_t<Const, const T, T>;
    using BitItType = TrueBitIt<TBitBlock, TS, true>;

    SKR_INLINE explicit SparseArrayIt(DataType* array, TS array_size, const TBitBlock* bit_array, TS start = 0)
        : _array(array)
        , _bit_it(bit_array, array_size, start)
    {
    }

    // impl cpp iterator
    SKR_INLINE SparseArrayIt& operator++()
    {
        ++_bit_it;
        return *this;
    }
    SKR_INLINE bool       operator==(const SparseArrayIt& rhs) const { return _bit_it == rhs._bit_it && _array == rhs._array; }
    SKR_INLINE bool       operator!=(const SparseArrayIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE explicit   operator bool() const { return (bool)_bit_it; }
    SKR_INLINE bool       operator!() const { return !(bool)*this; }
    SKR_INLINE ValueType& operator*() const { return _array[index()]._sparse_array_data; }
    SKR_INLINE ValueType* operator->() const { return &_array[index()]._sparse_array_data; }

    // other data
    SKR_INLINE ValueType* data() const { return &_array[index()]._sparse_array_data; }
    SKR_INLINE TS         index() const { return _bit_it.index(); }

private:
    DataType* _array;
    BitItType _bit_it;
};
} // namespace skr::container

namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, bool kConst>
struct SparseArrayCursor {
    using BitCursorType = TrueBitCursor<TBitBlock, TS, kConst>;
    using StorageType   = std::conditional_t<kConst, const SparseArrayData<T, TS>, SparseArrayData<T, TS>>;
    using BitBlockType  = std::conditional_t<kConst, const TBitBlock, TBitBlock>;
    using DataType      = std::conditional_t<kConst, const T, T>;
    using SizeType      = TS;

    // ctor & copy & move & assign & move assign
    inline SparseArrayCursor(StorageType* _data, BitCursorType bit_it)
        : _bit_it(bit_it)
        , _data(_data)
    {
    }
    inline SparseArrayCursor(const SparseArrayCursor& rhs)            = default;
    inline SparseArrayCursor(SparseArrayCursor&& rhs)                 = default;
    inline SparseArrayCursor& operator=(const SparseArrayCursor& rhs) = default;
    inline SparseArrayCursor& operator=(SparseArrayCursor&& rhs)      = default;

    // factory
    inline static SparseArrayCursor Begin(StorageType* data, BitBlockType* bit_array, SizeType size)
    {
        return { data, BitCursorType::Begin(bit_array, size) };
    }
    inline static SparseArrayCursor BeginOverflow(StorageType* data, BitBlockType* bit_array, SizeType size)
    {
        return { data, BitCursorType::BeginOverflow(bit_array, size) };
    }
    inline static SparseArrayCursor End(StorageType* data, BitBlockType* bit_array, SizeType size)
    {
        return { data, BitCursorType::End(bit_array, size) };
    }
    inline static SparseArrayCursor EndOverflow(StorageType* data, BitBlockType* bit_array, SizeType size)
    {
        return { data, BitCursorType::EndOverflow(bit_array, size) };
    }

    // getter
    inline DataType& ref() const
    {
        SKR_ASSERT(is_valid());
        return _data[index()]._sparse_array_data;
    }
    inline DataType* ptr() const
    {
        SKR_ASSERT(is_valid());
        return &_data[index()]._sparse_array_data;
    }
    inline SizeType index() const { return _bit_it.index(); }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        ++_bit_it;
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        --_bit_it;
    }
    inline void reset_to_begin() { _bit_it.reset_to_begin(); }
    inline void reset_to_end() { _bit_it.reset_to_end(); }

    // reach & validate
    bool reach_end() const { return _bit_it.reach_end(); }
    bool reach_begin() const { return _bit_it.reach_begin(); }
    bool is_valid() const { return _bit_it.is_valid(); }

    // compare
    bool operator==(const SparseArrayCursor& rhs) const { return _bit_it == rhs._bit_it && _data == rhs._data; }
    bool operator!=(const SparseArrayCursor& rhs) const { return !(*this == rhs); }

private:
    BitCursorType _bit_it;
    StorageType*  _data;
};

template <typename T, typename TBitBlock, typename TS, bool kConst>
struct SparseArrayIter {
    using CursorType   = SparseArrayCursor<T, TBitBlock, TS, kConst>;
    using StorageType  = std::conditional_t<kConst, const SparseArrayData<T, TS>, SparseArrayData<T, TS>>;
    using BitBlockType = std::conditional_t<kConst, const TBitBlock, TBitBlock>;
    using DataType     = std::conditional_t<kConst, const T, T>;
    using SizeType     = TS;

    // ctor & copy & move & assign & move assign
    inline SparseArrayIter(StorageType* data, BitBlockType* bit_array, SizeType size)
        : _cursor(CursorType::Begin(data, bit_array, size))
    {
    }
    inline SparseArrayIter(const SparseArrayIter& rhs)            = default;
    inline SparseArrayIter(SparseArrayIter&& rhs)                 = default;
    inline SparseArrayIter& operator=(const SparseArrayIter& rhs) = default;
    inline SparseArrayIter& operator=(SparseArrayIter&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_begin(); }
    inline void move_next() { _cursor.move_next(); }
    inline bool has_next() const { return !_cursor.reach_end(); }

private:
    CursorType _cursor;
};
template <typename T, typename TBitBlock, typename TS, bool kConst>
struct SparseArrayIterInv {
    using CursorType   = SparseArrayCursor<T, TBitBlock, TS, kConst>;
    using StorageType  = std::conditional_t<kConst, const SparseArrayData<T, TS>, SparseArrayData<T, TS>>;
    using BitBlockType = std::conditional_t<kConst, const TBitBlock, TBitBlock>;
    using DataType     = std::conditional_t<kConst, const T, T>;
    using SizeType     = TS;

    // ctor & copy & move & assign & move assign
    inline SparseArrayIterInv(StorageType* data, BitBlockType* bit_array, SizeType size)
        : _cursor(CursorType::End(data, bit_array, size))
    {
    }
    inline SparseArrayIterInv(const SparseArrayIterInv& rhs)            = default;
    inline SparseArrayIterInv(SparseArrayIterInv&& rhs)                 = default;
    inline SparseArrayIterInv& operator=(const SparseArrayIterInv& rhs) = default;
    inline SparseArrayIterInv& operator=(SparseArrayIterInv&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_end(); }
    inline void move_next() { _cursor.move_prev(); }
    inline bool has_next() const { return !_cursor.reach_begin(); }

private:
    CursorType _cursor;
};
} // namespace skr::container