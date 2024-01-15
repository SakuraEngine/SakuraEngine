#pragma once
#include "SkrBase/algo/bit_algo.hpp"
#include "SkrBase/containers/misc/iterator.hpp"

namespace skr::container
{
template <typename Container, bool kConst>
struct SparseArrayCursor;

template <typename Container, bool kConst>
struct SparseArrayIter : public CursorIter<SparseArrayCursor<Container, kConst>, false> {
    using Super = CursorIter<SparseArrayCursor<Container, kConst>, false>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_next();
    }
};

template <typename Container, bool kConst>
struct SparseArrayIterInv : public CursorIter<SparseArrayCursor<Container, kConst>, true> {
    using Super = CursorIter<SparseArrayCursor<Container, kConst>, true>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_prev();
    }
};

template <typename Container, bool kConst>
struct SparseArrayCursor {
    using ContainerType = std::conditional_t<kConst, const Container, Container>;
    using DataType      = std::conditional_t<kConst, const typename ContainerType::DataType, typename ContainerType::DataType>;
    using SizeType      = typename ContainerType::SizeType;
    using Algo          = algo::BitAlgo<typename ContainerType::BitBlockType>;

    static constexpr SizeType npos = npos_of<SizeType>;

    // ctor & copy & move & assign & move assign
    inline SparseArrayCursor(ContainerType* array, SizeType index)
        : _array(array)
        , _index(index)
    {
        SKR_ASSERT((_index >= 0 && _index <= _array->sparse_size()) || _index == npos);
        SKR_ASSERT(!is_valid() || _array->has_data(_index));
    }
    inline SparseArrayCursor(ContainerType* array)
        : _array(array)
        , _index(npos)
    {
    }
    inline SparseArrayCursor(const SparseArrayCursor& rhs)            = default;
    inline SparseArrayCursor(SparseArrayCursor&& rhs)                 = default;
    inline SparseArrayCursor& operator=(const SparseArrayCursor& rhs) = default;
    inline SparseArrayCursor& operator=(SparseArrayCursor&& rhs)      = default;

    // factory
    inline static SparseArrayCursor Begin(ContainerType* array)
    {
        SparseArrayCursor cursor{ array };
        cursor.reset_to_begin();
        return cursor;
    }
    inline static SparseArrayCursor BeginOverflow(ContainerType* array)
    {
        SparseArrayCursor cursor{ array };
        cursor._reset_to_begin_overflow();
        return cursor;
    }
    inline static SparseArrayCursor End(ContainerType* array)
    {
        SparseArrayCursor cursor{ array };
        cursor.reset_to_end();
        return cursor;
    }
    inline static SparseArrayCursor EndOverflow(ContainerType* array)
    {
        SparseArrayCursor cursor{ array };
        cursor._reset_to_end_overflow();
        return cursor;
    }

    // getter
    inline DataType& ref() const
    {
        SKR_ASSERT(is_valid());
        return _data()[_index]._sparse_array_data;
    }
    inline DataType* ptr() const
    {
        SKR_ASSERT(is_valid());
        return &_data()[_index]._sparse_array_data;
    }
    inline SizeType index() const { return _index; }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        _index = Algo::find(_bit_data(), _index + 1, _size() - _index - 1, true);
        _index = (_index == npos) ? _size() : _index;
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        _index = Algo::find_last(_bit_data(), (SizeType)0, _index, true);
    }
    inline void reset_to_begin()
    {
        if (!_array_empty())
        {
            _index = Algo::find(_bit_data(), (SizeType)0, _size(), true);
            _index = (_index == npos) ? _size() : _index;
        }
        else
        {
            _reset_to_end_overflow();
        }
    }
    inline void reset_to_end()
    {
        if (!_array_empty())
        {
            _index = Algo::find_last(_bit_data(), (SizeType)0, _size(), true);
        }
        else
        {
            _reset_to_begin_overflow();
        }
    }

    // erase
    inline void erase_and_move_next()
    {
        SKR_ASSERT(is_valid());
        _array->remove_at(_index);
        move_next();
    }
    inline void erase_and_move_prev()
    {
        SKR_ASSERT(is_valid());
        _array->remove_at(_index);
        move_prev();
    }

    // reach & validate
    bool reach_end() const { return _index == _size(); }
    bool reach_begin() const { return _index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const SparseArrayCursor& rhs) const { return _array == rhs._array && _index == rhs._index; }
    bool operator!=(const SparseArrayCursor& rhs) const { return !(*this == rhs); }

    // convert
    inline SparseArrayIter<ContainerType, kConst>    as_iter() const { return { *this }; }
    inline SparseArrayIterInv<ContainerType, kConst> as_iter_inv() const { return { *this }; }
    inline CursorRange<SparseArrayCursor, false>     as_range() const { return { *this }; }
    inline CursorRange<SparseArrayCursor, true>      as_range_inv() const { return { *this }; }

protected:
    inline auto _data() const { return _array->memory().data(); }
    inline auto _size() const { return _array->memory().sparse_size(); }
    inline auto _bit_data() const { return _array->memory().bit_array(); }
    inline bool _array_empty() const { return _array->empty(); }

    inline void _reset_to_end_overflow() { _index = _size(); }
    inline void _reset_to_begin_overflow() { _index = npos; }

private:
    ContainerType* _array;
    SizeType       _index;
};
} // namespace skr::container