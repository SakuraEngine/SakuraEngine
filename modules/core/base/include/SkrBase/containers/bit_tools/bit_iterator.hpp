#pragma once
#include "SkrBase/config.h"
#include "SkrBase/algo/bit_algo.hpp"
#include "bit_ref.hpp"
#include "SkrBase/containers/misc/iterator.hpp"

namespace skr::container
{
template <typename TBlock, typename TSize, bool kConst>
struct BitCursor;
template <typename TBlock, typename TSize, bool kConst>
struct TrueBitCursor;
template <typename TBlock, typename TSize, bool kConst>
struct FalseBitCursor;

template <typename TBlock, typename TSize, bool kConst>
using BitIter = CursorIter<BitCursor<TBlock, TSize, kConst>, false>;
template <typename TBlock, typename TSize, bool kConst>
using BitIterInv = CursorIter<BitCursor<TBlock, TSize, kConst>, true>;

template <typename TBlock, typename TSize, bool kConst>
using TrueBitIter = CursorIter<TrueBitCursor<TBlock, TSize, kConst>, false>;
template <typename TBlock, typename TSize, bool kConst>
using TrueBitIterInv = CursorIter<TrueBitCursor<TBlock, TSize, kConst>, true>;

template <typename TBlock, typename TSize, bool kConst>
using FalseBitIter = CursorIter<FalseBitCursor<TBlock, TSize, kConst>, false>;
template <typename TBlock, typename TSize, bool kConst>
using FalseBitIterInv = CursorIter<FalseBitCursor<TBlock, TSize, kConst>, true>;

template <typename TBlock, typename TSize, bool kConst>
struct BitCursor {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);
    using DataType = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType  = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType = TSize;
    using Algo     = algo::BitAlgo<TBlock>;

    static constexpr SizeType npos = npos_of<TSize>;

    // ctor & copy & move & assign & move assign
    inline BitCursor(DataType* data, SizeType size, SizeType index)
        : _data(data)
        , _bit_size(size)
        , _bit_index(index)
    {
        SKR_ASSERT((_bit_index >= 0 && _bit_index <= _bit_size) || _bit_index == npos);
    }
    inline BitCursor(const BitCursor& rhs)            = default;
    inline BitCursor(BitCursor&& rhs)                 = default;
    inline BitCursor& operator=(const BitCursor& rhs) = default;
    inline BitCursor& operator=(BitCursor&& rhs)      = default;

    // factory
    inline static BitCursor Begin(DataType* data, SizeType size) { return BitCursor{ data, size, 0 }; }
    inline static BitCursor BeginOverflow(DataType* data, SizeType size) { return BitCursor{ data, size, npos }; }
    inline static BitCursor End(DataType* data, SizeType size) { return BitCursor{ data, size, size - 1 }; }
    inline static BitCursor EndOverflow(DataType* data, SizeType size) { return BitCursor{ data, size, size }; }

    // getter
    inline RefType ref() const
    {
        SKR_ASSERT(is_valid());
        if constexpr (kConst)
        {
            return Algo::get(_data, _bit_index);
        }
        else
        {
            return BitRef<TBlock>::At(_data, _bit_index);
        }
    }
    inline SizeType index() const { return _bit_index; }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        ++_bit_index;
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        --_bit_index;
    }
    inline void reset_to_begin() { _bit_index = 0; }
    inline void reset_to_end() { _bit_index = _bit_size - 1; }

    // reach & validate
    inline bool reach_end() const { return _bit_index == _bit_size; }
    inline bool reach_begin() const { return _bit_index == npos; }
    inline bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    inline bool operator==(const BitCursor& rhs) const { return _data == rhs._data && _bit_index == rhs._bit_index; }
    inline bool operator!=(const BitCursor& rhs) const { return !(*this == rhs); }

private:
    DataType* _data;
    SizeType  _bit_size;
    SizeType  _bit_index;
};
template <typename TBlock, typename TSize, bool kConst>
struct TrueBitCursor {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);
    using DataType = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType  = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType = TSize;
    using Algo     = algo::BitAlgo<TBlock>;

    static constexpr SizeType npos = npos_of<TSize>;

    // ctor & copy & move & assign & move assign
    inline TrueBitCursor(DataType* data, SizeType size, SizeType index)
        : _data(data)
        , _bit_size(size)
        , _bit_index(index)
    {
        SKR_ASSERT((_bit_index >= 0 && _bit_index <= _bit_size) || _bit_index == npos);
        SKR_ASSERT(!is_valid() || ref() == true);
    }
    inline TrueBitCursor(DataType* data, SizeType size)
        : _data(data)
        , _bit_size(size)
        , _bit_index(npos)
    {
    }
    inline TrueBitCursor(const TrueBitCursor& rhs)            = default;
    inline TrueBitCursor(TrueBitCursor&& rhs)                 = default;
    inline TrueBitCursor& operator=(const TrueBitCursor& rhs) = default;
    inline TrueBitCursor& operator=(TrueBitCursor&& rhs)      = default;

    // factory
    inline static TrueBitCursor Begin(DataType* data, SizeType size)
    {
        TrueBitCursor cursor{ data, size };
        cursor.reset_to_begin();
        return cursor;
    }
    inline static TrueBitCursor BeginOverflow(DataType* data, SizeType size)
    {
        TrueBitCursor cursor{ data, size };
        cursor._reset_to_begin_overflow();
        return cursor;
    }
    inline static TrueBitCursor End(DataType* data, SizeType size)
    {
        TrueBitCursor cursor{ data, size };
        cursor.reset_to_end();
        return cursor;
    }
    inline static TrueBitCursor EndOverflow(DataType* data, SizeType size)
    {
        TrueBitCursor cursor{ data, size };
        cursor._reset_to_end_overflow();
        return cursor;
    }

    // getter
    inline RefType ref() const
    {
        SKR_ASSERT(is_valid());
        if constexpr (kConst)
        {
            return Algo::get(_data, _bit_index);
        }
        else
        {
            return BitRef<TBlock>::At(_data, _bit_index);
        }
    }
    inline SizeType index() const { return _bit_index; }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        SizeType result = Algo::find(_data, _bit_index + 1, _bit_size - _bit_index - 1, true);
        _bit_index      = result == npos ? _bit_size : result;
        SKR_ASSERT(_bit_index == npos || _bit_index <= _bit_size);
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        _bit_index = Algo::find_last(_data, (SizeType)0, _bit_index, true);
        SKR_ASSERT(_bit_index == npos || _bit_index <= _bit_size);
    }
    inline void reset_to_begin()
    {
        if (_data && _bit_size > 0)
        {
            SizeType index = Algo::find(_data, (SizeType)0, _bit_size, true);
            _bit_index     = (index == npos) ? _bit_size : index;
        }
        else
        {
            _bit_index = 0;
        }
        SKR_ASSERT(_bit_index == npos || _bit_index <= _bit_size);
    }
    inline void reset_to_end()
    {
        if (_data && _bit_size > 0)
        {
            _bit_index = Algo::find_last(_data, (SizeType)0, _bit_size, true);
        }
        else
        {
            _bit_index = npos;
        }
        SKR_ASSERT(_bit_index == npos || _bit_index <= _bit_size);
    }

    // reach & validate
    inline bool reach_end() const { return _bit_index == _bit_size; }
    inline bool reach_begin() const { return _bit_index == npos; }
    inline bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    inline bool operator==(const TrueBitCursor& rhs) const { return _data == rhs._data && _bit_index == rhs._bit_index; }
    inline bool operator!=(const TrueBitCursor& rhs) const { return !(*this == rhs); }

protected:
    inline void _reset_to_end_overflow() { _bit_index = _bit_size; }
    inline void _reset_to_begin_overflow() { _bit_index = npos; }

private:
    DataType* _data;
    SizeType  _bit_size;
    SizeType  _bit_index;
};
template <typename TBlock, typename TSize, bool kConst>
struct FalseBitCursor {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);
    using DataType = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType  = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType = TSize;
    using Algo     = algo::BitAlgo<TBlock>;

    static constexpr SizeType npos = npos_of<TSize>;

    // ctor & copy & move & assign & move assign
    inline FalseBitCursor(DataType* data, SizeType size, SizeType index)
        : _data(data)
        , _bit_size(size)
        , _bit_index(index)
    {
        SKR_ASSERT((_bit_index >= 0 && _bit_index <= _bit_size) || _bit_index == npos);
        SKR_ASSERT(!is_valid() || ref() == false);
    }
    inline FalseBitCursor(const FalseBitCursor& rhs)            = default;
    inline FalseBitCursor(FalseBitCursor&& rhs)                 = default;
    inline FalseBitCursor& operator=(const FalseBitCursor& rhs) = default;
    inline FalseBitCursor& operator=(FalseBitCursor&& rhs)      = default;

    // factory
    inline static FalseBitCursor Begin(DataType* data, SizeType size)
    {
        if (data && size > 0)
        {
            SizeType index = Algo::find(data, 0, size, false);
            index          = (index == npos) ? size : index;
            return TrueBitCursor{ data, size, index };
        }
        return TrueBitCursor{ data, size, 0 };
    }
    inline static FalseBitCursor BeginOverflow(DataType* data, SizeType size)
    {
        return TrueBitCursor{ data, size, npos };
    }
    inline static FalseBitCursor End(DataType* data, SizeType size)
    {
        if (data && size > 0)
        {
            SizeType index = Algo::find_last(data, 0, size, false);
            return TrueBitCursor{ data, size, index };
        }
        return TrueBitCursor{ data, size, size - 1 };
    }
    inline static FalseBitCursor EndOverflow(DataType* data, SizeType size)
    {
        return FalseBitCursor{ data, size, size };
    }

    // getter
    inline RefType ref() const
    {
        SKR_ASSERT(is_valid());
        if constexpr (kConst)
        {
            return Algo::get(_data, _bit_index);
        }
        else
        {
            return BitRef<TBlock>::At(_data, _bit_index);
        }
    }
    inline SizeType index() const { return _bit_index; }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        SizeType result = Algo::find(_data, _bit_index + 1, _bit_size - _bit_index - 1, false);
        _bit_index      = result == npos ? _bit_size : result;
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        _bit_index = Algo::find_last(_data, 0, _bit_index, false);
    }
    inline void reset_to_begin()
    {
        if (_data && _bit_size > 0)
        {
            SizeType index = Algo::find(_data, 0, _bit_size, false);
            _bit_index     = (index == npos) ? _bit_size : index;
        }
        _bit_index = 0;
    }
    inline void reset_to_end()
    {
        if (_data && _bit_size > 0)
        {
            _bit_index = Algo::find_last(_data, 0, _bit_size, false);
        }
        _bit_index = npos;
    }

    // reach & validate
    bool reach_end() const { return _bit_index == _bit_size; }
    bool reach_begin() const { return _bit_index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const FalseBitCursor& rhs) const { return _data == rhs._data && _bit_index == rhs._bit_index; }
    bool operator!=(const FalseBitCursor& rhs) const { return !(*this == rhs); }

private:
    DataType* _data;
    SizeType  _bit_size;
    SizeType  _bit_index;
};
} // namespace skr::container
