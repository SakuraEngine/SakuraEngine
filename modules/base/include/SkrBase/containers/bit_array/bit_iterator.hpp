#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/bit_algo.hpp"
#include "bit_ref.hpp"

// bit iterator
namespace skr::container
{
template <typename TBlock, typename TS, bool Const>
struct BitIt {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);

public:
    using DataPtr = std::conditional_t<Const, const TBlock*, TBlock*>;
    using RefType = std::conditional_t<Const, bool, BitRef<TBlock>>;
    using Algo    = algo::BitAlgo<TBlock>;

    SKR_INLINE BitIt(DataPtr data, TS size, TS start = 0)
        : _data(data)
        , _bit_size(size)
        , _bit_index(start)
    {
    }

    SKR_INLINE BitIt& operator++()
    {
        ++_bit_index;
        return *this;
    }
    SKR_INLINE explicit operator bool() const { return _bit_index < _bit_size; }
    SKR_INLINE bool     operator!() const { return !(bool)*this; }
    SKR_INLINE bool     operator==(const BitIt& rhs) const { return _data == rhs._data && _bit_index == rhs._bit_index; }
    SKR_INLINE bool     operator!=(const BitIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE RefType  operator*() const { return value(); }

    SKR_INLINE RefType value() const
    {
        if constexpr (Const)
        {
            return Algo::get(_data, _bit_index);
        }
        else
        {
            return BitRef<TBlock>::At(_data, _bit_index);
        }
    }
    SKR_INLINE TS index() const { return _bit_index; }

private:
    DataPtr _data;
    TS      _bit_size;
    TS      _bit_index;
};
} // namespace skr::container

// true bit iterator
namespace skr::container
{
template <typename TBlock, typename TS, bool Const>
struct TrueBitIt {
    using DataPtr = std::conditional_t<Const, const TBlock*, TBlock*>;
    using RefType = std::conditional_t<Const, bool, BitRef<TBlock>>;
    using Algo    = algo::BitAlgo<TBlock>;

    SKR_INLINE TrueBitIt(DataPtr data, TS size, TS start = 0)
        : _data(data)
        , _bit_size(size)
        , _bit_index(start)
    {
        SKR_ASSERT(start >= 0 && start <= size);
        if (data)
        {
            TS result  = Algo::find(_data, _bit_index, _bit_size, true);
            _bit_index = result == npos_of<TBlock> ? _bit_size : result;
        }
    }

    SKR_INLINE TrueBitIt& operator++()
    {
        TS result  = Algo::find(_data, _bit_index + 1, _bit_size, true);
        _bit_index = result == npos_of<TBlock> ? _bit_size : result;
        return *this;
    }
    SKR_INLINE TrueBitIt& operator--()
    {
        TS result  = Algo::find_last(_data, 0, _bit_index, true);
        _bit_index = result == npos_of<TBlock> ? _bit_size : result;
        return *this;
    }

    SKR_INLINE bool     operator==(const TrueBitIt& rhs) const { return _bit_index == rhs._bit_index && _data == rhs._data; }
    SKR_INLINE bool     operator!=(const TrueBitIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE explicit operator bool() const { return _bit_index < _bit_size; } // !!!Only support in increasing order currently
    SKR_INLINE bool     operator!() const { return !(bool)*this; }
    SKR_INLINE RefType  value() const
    {
        if constexpr (Const)
        {
            return Algo::get(_data, _bit_index);
        }
        else
        {
            return BitRef<TBlock>::At(_data, _bit_index);
        }
    }
    SKR_INLINE void flip() requires(!Const)
    {
        SKR_ASSERT(_bit_index < _bit_size && "flip an invalid iterator");
        BitRef<TBlock>::At(_data, _bit_index) = false;
    }
    SKR_INLINE TS index() const { return _bit_index; }

private:
    DataPtr _data;
    TS      _bit_size;
    TS      _bit_index;
};
} // namespace skr::container