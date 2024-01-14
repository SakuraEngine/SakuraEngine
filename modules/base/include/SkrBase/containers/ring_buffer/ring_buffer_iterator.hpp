#pragma once
#include "SkrBase/config.h"

namespace skr::container
{
template <typename T, typename TS, bool kConst>
struct RingBufferIt {
    using DataType  = std::conditional_t<kConst, const T*, T*>;
    using ValueType = std::conditional_t<kConst, const T, T>;

    inline RingBufferIt(DataType* data, TS capacity, TS pos)
        : _data(data)
        , _capacity(capacity)
        , _pos(pos)
    {
    }

    inline RingBufferIt& operator++()
    {
        ++_pos;
        return *this;
    }

    inline bool       operator==(const RingBufferIt& rhs) const { return _data == rhs._data && _pos == rhs._pos; }
    inline bool       operator!=(const RingBufferIt& rhs) const { return !(*this == rhs); }
    inline ValueType& operator*() const
    {
        return _data[_pos % _capacity];
    }
    inline ValueType* operator->() const
    {
        return &_data[_pos % _capacity];
    }

private:
    DataType* _data     = nullptr;
    TS        _capacity = 0;
    TS        _pos      = 0;
};
} // namespace skr::container

namespace skr::container
{
template <typename T, typename TS, bool kConst>
struct RingBufferCursor {
    using DataType = std::conditional_t<kConst, const T, T>;
    using SizeType = TS;

    // ctor & copy & move & assign & move assign
    inline RingBufferCursor(DataType* data, SizeType capacity, SizeType begin, SizeType end, SizeType index)
        : _data(data)
        , _capacity(capacity)
        , _begin(begin)
        , _end(end)
        , _index(index)
    {
        SKR_ASSERT(_begin != 0 && _end != 0);
        SKR_ASSERT(_index >= (begin - 1) && _index <= _end);
    }
    inline RingBufferCursor(const RingBufferCursor& rhs)            = default;
    inline RingBufferCursor(RingBufferCursor&& rhs)                 = default;
    inline RingBufferCursor& operator=(const RingBufferCursor& rhs) = default;
    inline RingBufferCursor& operator=(RingBufferCursor&& rhs)      = default;

    // factory
    inline static RingBufferCursor Begin(DataType* data, SizeType capacity, SizeType begin, SizeType end)
    {
        if (begin == 0)
        {
            begin += capacity;
            end += capacity;
        }
        return RingBufferCursor{ data, capacity, begin, end, begin };
    }
    inline static RingBufferCursor BeginOverflow(DataType* data, SizeType capacity, SizeType begin, SizeType end)
    {
        if (begin == 0)
        {
            begin += capacity;
            end += capacity;
        }
        return RingBufferCursor{ data, capacity, begin, end, begin - 1 };
    }
    inline static RingBufferCursor End(DataType* data, SizeType capacity, SizeType begin, SizeType end)
    {
        if (begin == 0)
        {
            begin += capacity;
            end += capacity;
        }
        return RingBufferCursor{ data, capacity, begin, end, end - 1 };
    }
    inline static RingBufferCursor EndOverflow(DataType* data, SizeType capacity, SizeType begin, SizeType end)
    {
        if (begin == 0)
        {
            begin += capacity;
            end += capacity;
        }
        return RingBufferCursor{ data, capacity, begin, end, end };
    }

    // getter
    inline DataType& ref() const
    {
        SKR_ASSERT(is_valid());
        return _data[_index % _capacity];
    }
    inline DataType* ptr() const
    {
        SKR_ASSERT(is_valid());
        return _data + (_index % _capacity);
    }
    inline SizeType index() const { return _index % _capacity; }

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
    inline void reset_to_begin() { _index = _begin; }
    inline void reset_to_end() { _index = _end - 1; }

    // reach & validate
    bool reach_end() const { return _index == _end; }
    bool reach_begin() const { return _index == _begin - 1; }
    bool is_valid() const { return !(reach_end() || reach_begin()); }

    // compare
    bool operator==(const RingBufferCursor& rhs) const { return _data == rhs._data && _index == rhs._index; }
    bool operator!=(const RingBufferCursor& rhs) const { return !(*this == rhs); }

private:
    DataType* _data;
    SizeType  _capacity;
    SizeType  _begin;
    SizeType  _end;
    SizeType  _index;
};

template <typename T, typename TS, bool kConst>
struct RingBufferIter {
    using CursorType = RingBufferCursor<T, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const T, T>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline RingBufferIter(DataType* data, SizeType capacity, SizeType begin, SizeType end)
        : _cursor(CursorType::Begin(data, capacity, begin, end))
    {
    }
    inline RingBufferIter(const RingBufferIter& rhs)            = default;
    inline RingBufferIter(RingBufferIter&& rhs)                 = default;
    inline RingBufferIter& operator=(const RingBufferIter& rhs) = default;
    inline RingBufferIter& operator=(RingBufferIter&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_begin(); }
    inline void move_next() { _cursor.move_next(); }
    inline bool has_next() const { return !_cursor.reach_end(); }

private:
    CursorType _cursor;
};
template <typename T, typename TS, bool kConst>
struct RingBufferIterInv {
    using CursorType = RingBufferCursor<T, TS, kConst>;
    using DataType   = std::conditional_t<kConst, const T, T>;
    using SizeType   = TS;

    // ctor & copy & move & assign & move assign
    inline RingBufferIterInv(DataType* data, SizeType capacity, SizeType begin, SizeType end)
        : _cursor(CursorType::End(data, capacity, begin, end))
    {
    }
    inline RingBufferIterInv(const RingBufferIterInv& rhs)            = default;
    inline RingBufferIterInv(RingBufferIterInv&& rhs)                 = default;
    inline RingBufferIterInv& operator=(const RingBufferIterInv& rhs) = default;
    inline RingBufferIterInv& operator=(RingBufferIterInv&& rhs)      = default;

    // getter
    inline DataType& ref() const { return _cursor.ref(); }
    inline DataType* ptr() const { return _cursor.ptr(); }
    inline SizeType  index() const { return _cursor.index(); }

    // move & validator
    inline void reset() { _cursor.reset_to_end(); }
    inline void move_next() { _cursor.move_prev(); }
    inline bool has_next() const { return !_cursor.reach_begin(); }

private:
    CursorType _cursor;
};

} // namespace skr::container