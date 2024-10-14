#pragma once
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/unicode/unicode_algo.hpp"
#include "SkrBase/containers/misc/iterator.hpp"
#include "SkrBase/misc/debug.h"

namespace skr
{
template <typename TS, bool kConst>
struct UTF8Cursor;
template <typename TS, bool kConst>
using UTF8Iter = container::CursorIter<UTF8Cursor<TS, kConst>, false>;
template <typename TS, bool kConst>
using UTF8IterInv = container::CursorIter<UTF8Cursor<TS, kConst>, true>;
template <typename TS, bool kConst>
using UTF8Range = container::CursorRange<UTF8Cursor<TS, kConst>, false>;
template <typename TS, bool kConst>
using UTF8RangeInv = container::CursorRange<UTF8Cursor<TS, kConst>, true>;

template <typename TS, bool kConst>
struct UTF16Cursor;
template <typename TS, bool kConst>
using UTF16Iter = container::CursorIter<UTF16Cursor<TS, kConst>, false>;
template <typename TS, bool kConst>
using UTF16IterInv = container::CursorIter<UTF16Cursor<TS, kConst>, true>;
template <typename TS, bool kConst>
using UTF16Range = container::CursorRange<UTF16Cursor<TS, kConst>, false>;
template <typename TS, bool kConst>
using UTF16RangeInv = container::CursorRange<UTF16Cursor<TS, kConst>, true>;

template <typename TS, bool kConst>
struct UTF8Cursor {
    using DataType = std::conditional_t<kConst, const skr_char8, skr_char8>;
    using SizeType = TS;
    using RefType  = UTF8Seq; // TODO. UTF8SeqRef

    static constexpr SizeType npos = npos_of<SizeType>;

    // iter & range
    using Iter     = UTF8Iter<TS, kConst>;
    using IterInv  = UTF8IterInv<TS, kConst>;
    using Range    = UTF8Range<TS, kConst>;
    using RangeInv = UTF8RangeInv<TS, kConst>;

    // ctor & copy & move & assign & move assign
    inline UTF8Cursor(DataType* data, SizeType size, SizeType index)
        : _data(data)
        , _size(size)
        , _index(index)
    {
        _try_parse_seq_len();
    }
    inline UTF8Cursor(const UTF8Cursor& other)              = default;
    inline UTF8Cursor(UTF8Cursor&& other) noexcept          = default;
    inline UTF8Cursor& operator=(const UTF8Cursor& rhs)     = default;
    inline UTF8Cursor& operator=(UTF8Cursor&& rhs) noexcept = default;

    // factory
    inline static UTF8Cursor Begin(DataType* data, SizeType size)
    {
        return { data, size, 0 };
    }
    inline static UTF8Cursor BeginOverflow(DataType* data, SizeType size)
    {
        return { data, size, npos };
    }
    inline static UTF8Cursor End(DataType* data, SizeType size)
    {
        UTF8Cursor result{ data, size, npos };
        result.reset_to_end();
        return result;
    }
    inline static UTF8Cursor EndOverflow(DataType* data, SizeType size)
    {
        return { data, size, size };
    }

    // getter
    inline RefType ref() const
    {
        return _seq_len == npos ? UTF8Seq::Bad(_data[_index]) :
                                  UTF8Seq{ _data + _index, static_cast<uint8_t>(_seq_len) };
    }
    inline SizeType index() const { return _index; }
    inline SizeType seq_len() const { return _seq_len; }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        _index += _seq_len == npos ? 1 : _seq_len; // _seq_len is always safe, because it has adjusted in _try_parse_seq_len
        _try_parse_seq_len();
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        _seq_len = utf8_adjust_index_to_head(_data, _size, _index - 1, _index);
        _seq_len = _seq_len ? _seq_len : npos;
    }
    inline void reset_to_begin()
    {
        _index = 0;
        _try_parse_seq_len();
    }
    inline void reset_to_end()
    {
        _seq_len = utf8_adjust_index_to_head(_data, _size, _size - 1, _index);
        _seq_len = _seq_len ? _seq_len : npos;
    }

    // reach & validate
    inline bool reach_end() const { return _index == _size; }
    inline bool reach_begin() const { return _index == npos; }
    inline bool is_valid() const { return !reach_end() && !reach_begin(); }

    // compare
    inline bool operator==(const UTF8Cursor& rhs) const { return _data == rhs._data && _index == rhs._index; }
    inline bool operator!=(const UTF8Cursor& rhs) const { return !(*this == rhs); }

    // convert
    inline Iter     as_iter() const { return { *this }; }
    inline IterInv  as_iter_inv() const { return { *this }; }
    inline Range    as_range() const { return { *this }; }
    inline RangeInv as_range_inv() const { return { *this }; }

private:
    inline void _try_parse_seq_len()
    {
        if (is_valid())
        {
            _seq_len = utf8_seq_len(_data[_index]);
            // if the last sequence is invalid, set it to 1
            // this operation can avoid visit and move operation overflow
            // TODO. maybe 0 is better
            if (!_seq_len || _index + _seq_len > _size)
            {
                _seq_len = npos;
            }
        }
    }

private:
    DataType* _data    = nullptr;
    SizeType  _size    = 0;
    SizeType  _index   = 0;
    SizeType  _seq_len = 0; // npos means bad sequence
};

template <typename TS, bool kConst>
struct UTF16Cursor {
    using DataType = std::conditional_t<kConst, const skr_char16, skr_char16>;
    using SizeType = TS;
    using RefType  = UTF16Seq; // TODO. UTF16SeqRef

    static constexpr SizeType npos = npos_of<SizeType>;

    // iter & range
    using Iter     = UTF16Iter<TS, kConst>;
    using IterInv  = UTF16IterInv<TS, kConst>;
    using Range    = UTF16Range<TS, kConst>;
    using RangeInv = UTF16RangeInv<TS, kConst>;

    // ctor & copy & move & assign & move assign
    inline UTF16Cursor(DataType* data, SizeType size, SizeType index)
        : _data(data)
        , _size(size)
        , _index(index)
    {
        _try_parse_seq_len();
    }
    inline UTF16Cursor(const UTF16Cursor& other)              = default;
    inline UTF16Cursor(UTF16Cursor&& other) noexcept          = default;
    inline UTF16Cursor& operator=(const UTF16Cursor& rhs)     = default;
    inline UTF16Cursor& operator=(UTF16Cursor&& rhs) noexcept = default;

    // factory
    inline static UTF16Cursor Begin(DataType* data, SizeType size)
    {
        return { data, size, 0 };
    }
    inline static UTF16Cursor BeginOverflow(DataType* data, SizeType size)
    {
        return { data, size, npos };
    }
    inline static UTF16Cursor End(DataType* data, SizeType size)
    {
        UTF16Cursor result{ data, size, npos };
        result.reset_to_end();
        return result;
    }
    inline static UTF16Cursor EndOverflow(DataType* data, SizeType size)
    {
        return { data, size, size };
    }

    // getter
    inline RefType ref() const
    {
        return _seq_len == npos ? UTF16Seq::Bad(_data[_index]) :
                                  UTF16Seq{ _data + _index, static_cast<uint8_t>(_seq_len) };
    }
    inline SizeType index() const { return _index; }
    inline SizeType seq_len() const { return _seq_len; }

    // move & reset
    inline void move_next()
    {
        SKR_ASSERT(is_valid());
        _index += _seq_len == npos ? 1 : _seq_len; // _seq_len is always safe, because it has adjusted in _try_parse_seq_len
        _try_parse_seq_len();
    }
    inline void move_prev()
    {
        SKR_ASSERT(is_valid());
        _seq_len = utf16_adjust_index_to_head(_data, _size, _index - 1, _index);
        _seq_len = _seq_len ? _seq_len : npos;
    }
    inline void reset_to_begin()
    {
        _index = 0;
        _try_parse_seq_len();
    }
    inline void reset_to_end()
    {
        _seq_len = utf16_adjust_index_to_head(_data, _size, _size - 1, _index);
        _seq_len = _seq_len ? _seq_len : npos;
    }

    // reach & validate
    inline bool reach_end() const { return _index == _size; }
    inline bool reach_begin() const { return _index == npos; }
    inline bool is_valid() const { return !reach_end() && !reach_begin(); }

    // compare
    inline bool operator==(const UTF16Cursor& rhs) const { return _data == rhs._data && _index == rhs._index; }
    inline bool operator!=(const UTF16Cursor& rhs) const { return !(*this == rhs); }

    // convert
    inline Iter     as_iter() const { return { *this }; }
    inline IterInv  as_iter_inv() const { return { *this }; }
    inline Range    as_range() const { return { *this }; }
    inline RangeInv as_range_inv() const { return { *this }; }

private:
    inline void _try_parse_seq_len()
    {
        if (is_valid())
        {
            _seq_len = utf16_seq_len(_data[_index]);
            // if the last sequence is invalid, set it to 1
            // this operation can avoid visit and move operation overflow
            // TODO. maybe 0 is better
            if (!_seq_len || _index + _seq_len > _size)
            {
                _seq_len = npos;
            }
        }
    }

private:
    DataType* _data    = nullptr;
    SizeType  _size    = 0;
    SizeType  _index   = 0;
    SizeType  _seq_len = 0; // npos means bad sequence
};
} // namespace skr