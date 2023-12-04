#pragma once
#include "SkrBase/config.h"
#include "SkrRT/containers/optional.hpp"
#include <type_traits>

namespace skr::container
{
template <typename TBlock>
struct BitRef {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);

    SKR_INLINE BitRef(TBlock& data, TBlock mask)
        : _data(data)
        , _mask(mask)
    {
    }
    SKR_INLINE         operator bool() const { return (_data & _mask); }
    SKR_INLINE BitRef& operator=(bool v)
    {
        _data = v ? _data | _mask : _data & ~_mask;
        return *this;
    }
    SKR_INLINE BitRef& operator=(const BitRef& rhs)
    {
        *this = (bool)rhs;
        return *this;
    }

private:
    TBlock& _data;
    TBlock  _mask;
};

template <typename TBlock, typename TS>
struct BitDataRef {
    Optional<BitRef<TBlock>> data  = {};
    TS                       index = npos_of<TS>;

    inline BitDataRef() = default;
    inline BitDataRef(TS index)
        : index(index)
    {
    }
    inline BitDataRef(BitRef<TBlock> data, TS index)
        : data(data)
        , index(index)
    {
    }

    inline                 operator bool() const { return data.has_value() || index != npos_of<TS>; }
    inline BitRef<TBlock>& operator*() const { return *data; }
};
} // namespace skr::container