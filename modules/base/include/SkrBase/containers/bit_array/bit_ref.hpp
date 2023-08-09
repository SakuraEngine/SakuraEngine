#pragma once
#include "SkrBase/config.h"
#include <type_traits>

namespace skr
{
template <typename TBlock>
struct BitRef {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);

    SKR_INLINE BitRef(TBlock& data, TBlock mask)
        : _data(data)
        , _mask(mask)
    {
    }
    SKR_INLINE operator bool() const { return (_data & _mask); }
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
} // namespace skr