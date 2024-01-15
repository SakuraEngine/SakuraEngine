#pragma once
#include "SkrBase/algo/bit_algo.hpp"
#include "SkrBase/containers/misc/iterator.hpp"

namespace skr::container
{
template <typename Container, bool kConst>
struct SparseVectorCursor;

template <typename Container, bool kConst>
struct SparseVectorIter : public CursorIter<SparseVectorCursor<Container, kConst>, false> {
    using Super = CursorIter<SparseVectorCursor<Container, kConst>, false>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_next();
    }
};

template <typename Container, bool kConst>
struct SparseVectorIterInv : public CursorIter<SparseVectorCursor<Container, kConst>, true> {
    using Super = CursorIter<SparseVectorCursor<Container, kConst>, true>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_prev();
    }
};

template <typename Container, bool kConst>
struct SparseVectorCursor {
    using ContainerType = std::conditional_t<kConst, const Container, Container>;
    using DataType      = std::conditional_t<kConst, const typename ContainerType::DataType, typename ContainerType::DataType>;
    using SizeType      = typename ContainerType::SizeType;
    using Algo          = algo::BitAlgo<typename ContainerType::BitBlockType>;

    static constexpr SizeType npos = npos_of<SizeType>;

    // ctor & copy & move & assign & move assign
    inline SparseVectorCursor(ContainerType* container, SizeType index)
        : _container(container)
        , _index(index)
    {
        SKR_ASSERT((_index >= 0 && _index <= _container->sparse_size()) || _index == npos);
        SKR_ASSERT(!is_valid() || _container->has_data(_index));
    }
    inline SparseVectorCursor(ContainerType* container)
        : _container(container)
        , _index(npos)
    {
    }
    inline SparseVectorCursor(const SparseVectorCursor& rhs)            = default;
    inline SparseVectorCursor(SparseVectorCursor&& rhs)                 = default;
    inline SparseVectorCursor& operator=(const SparseVectorCursor& rhs) = default;
    inline SparseVectorCursor& operator=(SparseVectorCursor&& rhs)      = default;

    // factory
    inline static SparseVectorCursor Begin(ContainerType* container)
    {
        SparseVectorCursor cursor{ container };
        cursor.reset_to_begin();
        return cursor;
    }
    inline static SparseVectorCursor BeginOverflow(ContainerType* container)
    {
        SparseVectorCursor cursor{ container };
        cursor._reset_to_begin_overflow();
        return cursor;
    }
    inline static SparseVectorCursor End(ContainerType* container)
    {
        SparseVectorCursor cursor{ container };
        cursor.reset_to_end();
        return cursor;
    }
    inline static SparseVectorCursor EndOverflow(ContainerType* container)
    {
        SparseVectorCursor cursor{ container };
        cursor._reset_to_end_overflow();
        return cursor;
    }

    // getter
    inline DataType& ref() const
    {
        SKR_ASSERT(is_valid());
        return _data()[_index]._sparse_vector_data;
    }
    inline DataType* ptr() const
    {
        SKR_ASSERT(is_valid());
        return &_data()[_index]._sparse_vector_data;
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
        if (!_container_empty())
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
        if (!_container_empty())
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
        _container->remove_at(_index);
        move_next();
    }
    inline void erase_and_move_prev()
    {
        SKR_ASSERT(is_valid());
        _container->remove_at(_index);
        move_prev();
    }

    // reach & validate
    bool reach_end() const { return _index == _size(); }
    bool reach_begin() const { return _index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const SparseVectorCursor& rhs) const { return _container == rhs._container && _index == rhs._index; }
    bool operator!=(const SparseVectorCursor& rhs) const { return !(*this == rhs); }

    // convert
    inline SparseVectorIter<ContainerType, kConst>    as_iter() const { return { *this }; }
    inline SparseVectorIterInv<ContainerType, kConst> as_iter_inv() const { return { *this }; }
    inline CursorRange<SparseVectorCursor, false>     as_range() const { return { *this }; }
    inline CursorRange<SparseVectorCursor, true>      as_range_inv() const { return { *this }; }

protected:
    inline auto _data() const { return _container->memory().data(); }
    inline auto _size() const { return _container->memory().sparse_size(); }
    inline auto _bit_data() const { return _container->memory().bit_array(); }
    inline bool _container_empty() const { return _container->empty(); }

    inline void _reset_to_end_overflow() { _index = _size(); }
    inline void _reset_to_begin_overflow() { _index = npos; }

private:
    ContainerType* _container;
    SizeType       _index;
};
} // namespace skr::container