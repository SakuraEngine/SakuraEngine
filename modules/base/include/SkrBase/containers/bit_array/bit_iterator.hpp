#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/bit_algo.hpp"
#include "bit_ref.hpp"

// bit iterator
namespace skr
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
        , _block_index(start >> Algo::PerBlockSizeLog2)
        , _block_mask(TBlock(1) << (start & Algo::PerBlockSizeMask))
    {
    }

    SKR_INLINE BitIt& operator++()
    {
        ++_bit_index;
        _block_mask <<= 1;

        // advance to the next uint32.
        if (!_block_mask)
        {
            _block_mask = 1;
            ++_block_index;
        }
        return *this;
    }
    SKR_INLINE explicit operator bool() const { return _bit_index < _bit_size; }
    SKR_INLINE bool    operator!() const { return !(bool)*this; }
    SKR_INLINE bool    operator==(const BitIt& rhs) const { return _data == rhs._data && _bit_index == rhs._bit_index; }
    SKR_INLINE bool    operator!=(const BitIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE RefType operator*() const { return value(); }

    SKR_INLINE RefType value() const
    {
        if constexpr (Const)
        {
            return _data[_block_index] & _block_mask;
        }
        else
        {
            return RefType(_data[_block_index], _block_mask);
        }
    }
    SKR_INLINE TS index() const { return _bit_index; }

private:
    DataPtr _data;
    TS      _bit_size;
    TS      _bit_index;
    TS      _block_index;
    TBlock  _block_mask;
};
} // namespace skr

// true bit iterator
namespace skr
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
        , _block_index(start >> Algo::PerBlockSizeLog2)
        , _block_mask(TBlock(1) << (start & Algo::PerBlockSizeMask))
        , _step_mask(Algo::FullMask << (start & Algo::PerBlockSizeMask))
    {
        SKR_ASSERT(start >= 0 && start <= size);
        _find_first_set_bit();
    }

    SKR_INLINE TrueBitIt& operator++()
    {
        // skip current mask
        _step_mask &= ~_block_mask;

        // find next mask
        _find_first_set_bit();
        return *this;
    }

    SKR_INLINE bool operator==(const TrueBitIt& rhs) const { return _bit_index == rhs._bit_index && _data == rhs._data; }
    SKR_INLINE bool operator!=(const TrueBitIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE explicit operator bool() const { return _bit_index < _bit_size; }
    SKR_INLINE bool    operator!() const { return !(bool)*this; }
    SKR_INLINE RefType value() const
    {
        if constexpr (Const)
        {
            return _data[_block_index] & _block_mask;
        }
        else
        {
            return RefType(_data[_block_index], _block_mask);
        }
    }
    SKR_INLINE TS index() const { return _bit_index; }

private:
    SKR_INLINE void _find_first_set_bit()
    {
        const TS block_last = (_bit_size - 1) >> Algo::PerBlockSizeLog2;

        // fast search
        TBlock test_val = _data[_block_index] & _step_mask;
        while (!test_val)
        {
            ++_block_index;

            // out of bound
            if (_block_index > block_last)
            {
                _bit_index = _bit_size;
                return;
            }

            test_val   = _data[_block_index];
            _step_mask = Algo::FullMask;
        }

        // set block mask & bit index
        const TBlock newVal = test_val & (test_val - 1);
        _block_mask         = newVal ^ test_val;
        _bit_index          = _block_index * Algo::PerBlockSize + countr_zero(_block_mask);

        // check bound
        if (_bit_index > _bit_size)
        {
            _bit_index = _bit_size;
        }
    }

private:
    DataPtr _data;
    TS      _bit_size;
    TS      _bit_index;
    TS      _block_index;
    TBlock  _block_mask;
    TBlock  _step_mask; // 用于快速的起步搜索
};
} // namespace skr