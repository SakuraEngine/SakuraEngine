#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/containers/misc/iterator.hpp"

namespace skr::container
{
template <typename Container, bool kConst>
struct ArrayCursor;

template <typename Container, bool kConst>
struct ArrayIter : public CursorIter<ArrayCursor<Container, kConst>, false> {
    using Super = CursorIter<ArrayCursor<Container, kConst>, false>;
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

template <typename Container, bool kConst>
struct ArrayIterInv : public CursorIter<ArrayCursor<Container, kConst>, true> {
    using Super = CursorIter<ArrayCursor<Container, kConst>, true>;
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

template <typename Container, bool kConst>
struct ArrayCursor {
    using ContainerType = std::conditional_t<kConst, const Container, Container>;
    using DataType      = std::conditional_t<kConst, const typename ContainerType::DataType, typename ContainerType::DataType>;
    using SizeType      = typename ContainerType::SizeType;

    static constexpr SizeType npos = npos_of<SizeType>;

    // ctor & copy & move & assign & move assign
    inline ArrayCursor(ContainerType* array, SizeType index)
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
    inline static ArrayCursor Begin(ContainerType* array) { return ArrayCursor{ array, 0 }; }
    inline static ArrayCursor BeginOverflow(ContainerType* array) { return ArrayCursor{ array, npos }; }
    inline static ArrayCursor End(ContainerType* array) { return ArrayCursor{ array, array->size() - 1 }; }
    inline static ArrayCursor EndOverflow(ContainerType* array) { return ArrayCursor{ array, array->size() }; }

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
    inline ArrayIter<Container, kConst>    as_iter() const { return { *this }; }
    inline ArrayIterInv<Container, kConst> as_iter_inv() const { return { *this }; }
    inline CursorRange<ArrayCursor, false> as_range() const { return { *this }; }
    inline CursorRange<ArrayCursor, true>  as_range_inv() const { return { *this }; }

private:
    ContainerType* _array;
    SizeType       _index;
};

} // namespace skr::container
