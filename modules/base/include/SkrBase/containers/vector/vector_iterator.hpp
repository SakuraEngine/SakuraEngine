#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/containers/misc/iterator.hpp"

namespace skr::container
{
template <typename Container, bool kConst>
struct VectorCursor;

template <typename Container, bool kConst>
struct VectorIter : public CursorIter<VectorCursor<Container, kConst>, false> {
    using Super = CursorIter<VectorCursor<Container, kConst>, false>;
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
struct VectorIterInv : public CursorIter<VectorCursor<Container, kConst>, true> {
    using Super = CursorIter<VectorCursor<Container, kConst>, true>;
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
struct VectorCursor {
    using ContainerType = std::conditional_t<kConst, const Container, Container>;
    using DataType      = std::conditional_t<kConst, const typename ContainerType::DataType, typename ContainerType::DataType>;
    using SizeType      = typename ContainerType::SizeType;

    static constexpr SizeType npos = npos_of<SizeType>;

    // ctor & copy & move & assign & move assign
    inline VectorCursor(ContainerType* vector, SizeType index)
        : _vector(vector)
        , _index(index)
    {
        SKR_ASSERT((_index >= 0 && _index <= _vector->size()) || _index == npos);
    }
    inline VectorCursor(const VectorCursor& rhs)            = default;
    inline VectorCursor(VectorCursor&& rhs)                 = default;
    inline VectorCursor& operator=(const VectorCursor& rhs) = default;
    inline VectorCursor& operator=(VectorCursor&& rhs)      = default;

    // factory
    inline static VectorCursor Begin(ContainerType* vector) { return VectorCursor{ vector, 0 }; }
    inline static VectorCursor BeginOverflow(ContainerType* vector) { return VectorCursor{ vector, npos }; }
    inline static VectorCursor End(ContainerType* vector) { return VectorCursor{ vector, vector->size() - 1 }; }
    inline static VectorCursor EndOverflow(ContainerType* vector) { return VectorCursor{ vector, vector->size() }; }

    // getter
    inline DataType& ref() const
    {
        SKR_ASSERT(is_valid());
        return _vector->data()[_index];
    }
    inline DataType* ptr() const
    {
        SKR_ASSERT(is_valid());
        return _vector->data() + _index;
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
    inline void reset_to_end() { _index = _vector->size() - 1; }

    // erase
    inline void erase_and_move_next()
    {
        SKR_ASSERT(is_valid());
        _vector->remove_at(_index);
    }
    inline void erase_and_move_next_swap()
    {
        SKR_ASSERT(is_valid());
        _vector->remove_at_swap(_index);
    }
    inline void erase_and_move_prev()
    {
        SKR_ASSERT(is_valid());
        _vector->remove_at(_index);
        --_index;
    }
    inline void erase_and_move_prev_swap()
    {
        SKR_ASSERT(is_valid());
        _vector->remove_at_swap(_index);
        --_index;
    }

    // reach & validate
    bool reach_end() const { return _index == _vector->size(); }
    bool reach_begin() const { return _index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const VectorCursor& rhs) const { return _vector == rhs._vector && _index == rhs._index; }
    bool operator!=(const VectorCursor& rhs) const { return !(*this == rhs); }

    // convert
    inline VectorIter<Container, kConst>    as_iter() const { return { *this }; }
    inline VectorIterInv<Container, kConst> as_iter_inv() const { return { *this }; }
    inline CursorRange<VectorCursor, false> as_range() const { return { *this }; }
    inline CursorRange<VectorCursor, true>  as_range_inv() const { return { *this }; }

private:
    ContainerType* _vector;
    SizeType       _index;
};

} // namespace skr::container
