#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/containers/misc/cursor_as_iterator.hpp"

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
using ArrayIter = CursorIter<ArrayCursor<T, TS, kConst>>;

template <typename T, typename TS, bool kConst>
using ArrayIterInv = CursorIterInv<ArrayCursor<T, TS, kConst>>;
} // namespace skr::container
