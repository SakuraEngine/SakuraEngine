#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/containers/misc/cursor_as_iterator.hpp"

namespace skr::container
{
template <typename Array, bool kConst>
struct ArrayCursor {
    using ArrayType = std::conditional_t<kConst, const Array, Array>;
    using DataType  = std::conditional_t<kConst, const typename ArrayType::DataType, typename ArrayType::DataType>;
    using RefType   = DataType&;
    using SizeType  = typename ArrayType::SizeType;

    static constexpr SizeType npos = npos_of<SizeType>;

    // ctor & copy & move & assign & move assign
    inline ArrayCursor(ArrayType* array, SizeType index)
        : _array(array)
        , _index(index)
    {
        SKR_ASSERT((_index >= 0 && _index <= _array->size()) || _index == npos);
    }
    inline ArrayCursor(const ArrayCursor& rhs)            = default;
    inline ArrayCursor(ArrayCursor&& rhs)                 = default;
    inline ArrayCursor& operator=(const ArrayCursor& rhs) = default;
    inline ArrayCursor& operator=(ArrayCursor&& rhs)      = default;

    // factory
    inline static ArrayCursor Begin(ArrayType* array) { return ArrayCursor{ array, 0 }; }
    inline static ArrayCursor BeginOverflow(ArrayType* array) { return ArrayCursor{ array, npos }; }
    inline static ArrayCursor End(ArrayType* array) { return ArrayCursor{ array, array->size() - 1 }; }
    inline static ArrayCursor EndOverflow(ArrayType* array) { return ArrayCursor{ array, array->size() }; }

    // getter
    inline DataType& ref() const
    {
        SKR_ASSERT(is_valid());
        return _array->data()[_index];
    }
    inline DataType* ptr() const
    {
        SKR_ASSERT(is_valid());
        return _array->data() + _index;
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
    inline void reset_to_end() { _index = _array->size() - 1; }

    // erase
    inline void erase_and_move_next()
    {
        SKR_ASSERT(is_valid());
        _array->remove_at(_index);
    }
    inline void erase_and_move_next_swap()
    {
        SKR_ASSERT(is_valid());
        _array->remove_at_swap(_index);
    }
    inline void erase_and_move_prev()
    {
        SKR_ASSERT(is_valid());
        _array->remove_at(_index);
        --_index;
    }
    inline void erase_and_move_prev_swap()
    {
        SKR_ASSERT(is_valid());
        _array->remove_at_swap(_index);
        --_index;
    }

    // reach & validate
    bool reach_end() const { return _index == _array->size(); }
    bool reach_begin() const { return _index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const ArrayCursor& rhs) const { return _array == rhs._array && _index == rhs._index; }
    bool operator!=(const ArrayCursor& rhs) const { return !(*this == rhs); }

    // convert
    // as_iter
    // as_iter_inv
    // as_range
    // as_range_inv

private:
    ArrayType* _array;
    SizeType   _index;
};

template <typename Array, bool kConst>
struct ArrayIter : public CursorIter<ArrayCursor<Array, kConst>> {
    using Super = CursorIter<ArrayCursor<Array, kConst>>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_next();
    }
    inline void erase_and_move_next_swap()
    {
        Super::cursor().erase_and_move_next_swap();
    }
};

template <typename Array, bool kConst>
struct ArrayIterInv : public CursorIterInv<ArrayCursor<Array, kConst>> {
    using Super = CursorIterInv<ArrayCursor<Array, kConst>>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_prev();
    }
    inline void erase_and_move_next_swap()
    {
        Super::cursor().erase_and_move_prev_swap();
    }
};

} // namespace skr::container
