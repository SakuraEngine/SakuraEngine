#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"

namespace skr::container
{
template <typename T, typename TS, bool kConst>
struct ArrayCursor {
    using DataType = std::conditional_t<kConst, const T, T>;
    using SizeType = TS;

    static constexpr SizeType npos = npos_of<TS>;

    // ctor & copy & move & assign & move assign
    inline ArrayCursor(DataType* data, SizeType size, SizeType index)
        : _data(data)
        , _size(size)
        , _index(index)
    {
        SKR_ASSERT((_index >= 0 && _index <= _size) || _index == npos);
    }
    inline ArrayCursor(const ArrayCursor& rhs)            = default;
    inline ArrayCursor(ArrayCursor&& rhs)                 = default;
    inline ArrayCursor& operator=(const ArrayCursor& rhs) = default;
    inline ArrayCursor& operator=(ArrayCursor&& rhs)      = default;

    // factory
    inline static ArrayCursor Begin(DataType* data, SizeType size) { return ArrayCursor{ data, size, 0 }; }
    inline static ArrayCursor BeginOverflow(DataType* data, SizeType size) { return ArrayCursor{ data, size, npos }; }
    inline static ArrayCursor End(DataType* data, SizeType size) { return ArrayCursor{ data, size, size - 1 }; }
    inline static ArrayCursor EndOverflow(DataType* data, SizeType size) { return ArrayCursor{ data, size, size }; }

    // getter
    inline DataType& ref() const
    {
        SKR_ASSERT(is_valid());
        return _data[_index];
    }
    inline DataType* ptr() const
    {
        SKR_ASSERT(is_valid());
        return _data + _index;
    }
    inline SizeType index() const { return _index; }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        ++_index;
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        --_index;
    }
    inline void reset_to_begin() { _index = 0; }
    inline void reset_to_end() { _index = _size - 1; }

    // reach & validate
    bool reach_end() const { return _index == _size; }
    bool reach_begin() const { return _index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const ArrayCursor& rhs) const { return _data == rhs._data && _index == rhs._index; }
    bool operator!=(const ArrayCursor& rhs) const { return !(*this == rhs); }

private:
    DataType* _data;
    SizeType  _size;
    SizeType  _index;
};

template <typename T, typename TS, bool kConst>
struct ArrayIter {
    using CursorType = ArrayCursor<T, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const T, T>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline ArrayIter(DataType* data, SizeType size)
        : _cursor(CursorType::Begin(data, size))
    {
    }
    inline ArrayIter(const ArrayIter& rhs)            = default;
    inline ArrayIter(ArrayIter&& rhs)                 = default;
    inline ArrayIter& operator=(const ArrayIter& rhs) = default;
    inline ArrayIter& operator=(ArrayIter&& rhs)      = default;

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

template <typename T, typename TS, bool kConst>
struct ArrayIterInv {
    using CursorType = ArrayCursor<T, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const T, T>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline ArrayIterInv(DataType* data, SizeType size)
        : _cursor(CursorType::End(data, size))
    {
    }
    inline ArrayIterInv(const ArrayIterInv& rhs)            = default;
    inline ArrayIterInv(ArrayIterInv&& rhs)                 = default;
    inline ArrayIterInv& operator=(const ArrayIterInv& rhs) = default;
    inline ArrayIterInv& operator=(ArrayIterInv&& rhs)      = default;

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
