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