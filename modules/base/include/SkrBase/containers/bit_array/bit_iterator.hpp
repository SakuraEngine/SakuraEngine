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

namespace skr::container
{
template <typename TBlock, typename TS, bool kConst>
struct BitCursor {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);
    using DataType = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType  = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType = TS;
    using Algo     = algo::BitAlgo<TBlock>;

    static constexpr SizeType npos = npos_of<TS>;

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
    bool reach_end() const { return _bit_index == _bit_size; }
    bool reach_begin() const { return _bit_index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const BitCursor& rhs) const { return _data == rhs._data && _bit_index == rhs._bit_index; }
    bool operator!=(const BitCursor& rhs) const { return !(*this == rhs); }

private:
    DataType* _data;
    SizeType  _bit_size;
    SizeType  _bit_index;
};
template <typename TBlock, typename TS, bool kConst>
struct TrueBitCursor {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);
    using DataType = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType  = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType = TS;
    using Algo     = algo::BitAlgo<TBlock>;

    static constexpr SizeType npos = npos_of<TS>;

    // ctor & copy & move & assign & move assign
    inline TrueBitCursor(DataType* data, SizeType size, SizeType index)
        : _data(data)
        , _bit_size(size)
        , _bit_index(index)
    {
        SKR_ASSERT((_bit_index >= 0 && _bit_index <= _bit_size) || _bit_index == npos);
        SKR_ASSERT(!is_valid() || ref() == true);
    }
    inline TrueBitCursor(const TrueBitCursor& rhs)            = default;
    inline TrueBitCursor(TrueBitCursor&& rhs)                 = default;
    inline TrueBitCursor& operator=(const TrueBitCursor& rhs) = default;
    inline TrueBitCursor& operator=(TrueBitCursor&& rhs)      = default;

    // factory
    inline static TrueBitCursor Begin(DataType* data, SizeType size)
    {
        if (data && size > 0)
        {
            SizeType index = Algo::find(data, 0, size, true);
            index          = (index == npos) ? size : index;
            return TrueBitCursor{ data, size, index };
        }
        return TrueBitCursor{ data, size, 0 };
    }
    inline static TrueBitCursor BeginOverflow(DataType* data, SizeType size)
    {
        return TrueBitCursor{ data, size, npos };
    }
    inline static TrueBitCursor End(DataType* data, SizeType size)
    {
        if (data && size > 0)
        {
            SizeType index = Algo::find_last(data, 0, size, true);
            return TrueBitCursor{ data, size, index };
        }
        return TrueBitCursor{ data, size, size - 1 };
    }
    inline static TrueBitCursor EndOverflow(DataType* data, SizeType size)
    {
        return TrueBitCursor{ data, size, size };
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
        SizeType result = Algo::find(_data, _bit_index + 1, _bit_size, true);
        _bit_index      = result == npos ? _bit_size : result;
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        _bit_index = Algo::find_last(_data, 0, _bit_index, true);
    }
    inline void reset_to_begin()
    {
        if (_data && _bit_size > 0)
        {
            SizeType index = Algo::find(_data, 0, _bit_size, true);
            _bit_index     = (index == npos) ? _bit_size : index;
        }
        _bit_index = 0;
    }
    inline void reset_to_end()
    {
        if (_data && _bit_size > 0)
        {
            _bit_index = Algo::find_last(_data, 0, _bit_size, true);
        }
        _bit_index = npos;
    }

    // reach & validate
    bool reach_end() const { return _bit_index == _bit_size; }
    bool reach_begin() const { return _bit_index == npos; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const TrueBitCursor& rhs) const { return _data == rhs._data && _bit_index == rhs._bit_index; }
    bool operator!=(const TrueBitCursor& rhs) const { return !(*this == rhs); }

private:
    DataType* _data;
    SizeType  _bit_size;
    SizeType  _bit_index;
};
template <typename TBlock, typename TS, bool kConst>
struct FalseBitCursor {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);
    using DataType = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType  = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType = TS;
    using Algo     = algo::BitAlgo<TBlock>;

    static constexpr SizeType npos = npos_of<TS>;

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
        SizeType result = Algo::find(_data, _bit_index + 1, _bit_size, false);
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

template <typename TBlock, typename TS, bool kConst>
struct BitIter {
    using CursorType = BitCursor<TBlock, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType    = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline BitIter(DataType* data, SizeType size)
        : _cursor(CursorType::Begin(data, size))
    {
    }
    inline BitIter(const BitIter& rhs)            = default;
    inline BitIter(BitIter&& rhs)                 = default;
    inline BitIter& operator=(const BitIter& rhs) = default;
    inline BitIter& operator=(BitIter&& rhs)      = default;

    // getter
    inline RefType  ref() const { return _cursor.ref(); }
    inline SizeType index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_begin(); }
    inline void move_next() { _cursor.move_next(); }
    inline bool has_next() const { return !_cursor.reach_end(); }

private:
    CursorType _cursor;
};
template <typename TBlock, typename TS, bool kConst>
struct BitIterInv {
    using CursorType = BitCursor<TBlock, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType    = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline BitIterInv(DataType* data, SizeType size)
        : _cursor(CursorType::End(data, size))
    {
    }
    inline BitIterInv(const BitIterInv& rhs)            = default;
    inline BitIterInv(BitIterInv&& rhs)                 = default;
    inline BitIterInv& operator=(const BitIterInv& rhs) = default;
    inline BitIterInv& operator=(BitIterInv&& rhs)      = default;

    // getter
    inline RefType  ref() const { return _cursor.ref(); }
    inline SizeType index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_end(); }
    inline void move_next() { _cursor.move_prev(); }
    inline bool has_next() const { return !_cursor.reach_begin(); }

private:
    CursorType _cursor;
};

template <typename TBlock, typename TS, bool kConst>
struct TrueBitIter {
    using CursorType = TrueBitCursor<TBlock, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType    = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline TrueBitIter(DataType* data, SizeType size)
        : _cursor(CursorType::Begin(data, size))
    {
    }
    inline TrueBitIter(const TrueBitIter& rhs)            = default;
    inline TrueBitIter(TrueBitIter&& rhs)                 = default;
    inline TrueBitIter& operator=(const TrueBitIter& rhs) = default;
    inline TrueBitIter& operator=(TrueBitIter&& rhs)      = default;

    // getter
    inline RefType  ref() const { return _cursor.ref(); }
    inline SizeType index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_begin(); }
    inline void move_next() { _cursor.move_next(); }
    inline bool has_next() const { return !_cursor.reach_end(); }

private:
    CursorType _cursor;
};
template <typename TBlock, typename TS, bool kConst>
struct TrueBitIterInv {
    using CursorType = TrueBitCursor<TBlock, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType    = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline TrueBitIterInv(DataType* data, SizeType size)
        : _cursor(CursorType::End(data, size))
    {
    }
    inline TrueBitIterInv(const TrueBitIterInv& rhs)            = default;
    inline TrueBitIterInv(TrueBitIterInv&& rhs)                 = default;
    inline TrueBitIterInv& operator=(const TrueBitIterInv& rhs) = default;
    inline TrueBitIterInv& operator=(TrueBitIterInv&& rhs)      = default;

    // getter
    inline RefType  ref() const { return _cursor.ref(); }
    inline SizeType index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_end(); }
    inline void move_next() { _cursor.move_prev(); }
    inline bool has_next() const { return !_cursor.reach_begin(); }

private:
    CursorType _cursor;
};

template <typename TBlock, typename TS, bool kConst>
struct FalseBitIter {
    using CursorType = FalseBitCursor<TBlock, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType    = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline FalseBitIter(DataType* data, SizeType size)
        : _cursor(CursorType::Begin(data, size))
    {
    }
    inline FalseBitIter(const FalseBitIter& rhs)            = default;
    inline FalseBitIter(FalseBitIter&& rhs)                 = default;
    inline FalseBitIter& operator=(const FalseBitIter& rhs) = default;
    inline FalseBitIter& operator=(FalseBitIter&& rhs)      = default;

    // getter
    inline RefType  ref() const { return _cursor.ref(); }
    inline SizeType index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_begin(); }
    inline void move_next() { _cursor.move_next(); }
    inline bool has_next() const { return !_cursor.reach_end(); }

private:
    CursorType _cursor;
};
template <typename TBlock, typename TS, bool kConst>
struct FalseBitIterInv {
    using CursorType = FalseBitCursor<TBlock, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const TBlock, TBlock>;
    using RefType    = std::conditional_t<kConst, bool, BitRef<TBlock>>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline FalseBitIterInv(DataType* data, SizeType size)
        : _cursor(CursorType::End(data, size))
    {
    }
    inline FalseBitIterInv(const FalseBitIterInv& rhs)            = default;
    inline FalseBitIterInv(FalseBitIterInv&& rhs)                 = default;
    inline FalseBitIterInv& operator=(const FalseBitIterInv& rhs) = default;
    inline FalseBitIterInv& operator=(FalseBitIterInv&& rhs)      = default;

    // getter
    inline RefType  ref() const { return _cursor.ref(); }
    inline SizeType index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_end(); }
    inline void move_next() { _cursor.move_prev(); }
    inline bool has_next() const { return !_cursor.reach_begin(); }

private:
    CursorType _cursor;
};
} // namespace skr::container
