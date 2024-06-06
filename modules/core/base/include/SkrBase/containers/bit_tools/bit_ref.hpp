#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/misc/optional.hpp"
#include "SkrBase/algo/bit_algo.hpp"
#include <type_traits>

namespace skr::container
{
template <typename TBlock>
struct BitRef {
    static_assert(std::is_integral_v<TBlock> && !std::is_signed_v<TBlock>);
    using Algo = algo::BitAlgo<TBlock>;

    SKR_INLINE static BitRef<TBlock> At(TBlock* data, TBlock idx)
    {
        return { data + (idx >> Algo::PerBlockSizeLog2), TBlock(1) << (idx & Algo::PerBlockSizeMask) };
    }

    SKR_INLINE BitRef() = default;

    SKR_INLINE BitRef(TBlock* data, TBlock mask)
        : _data(data)
        , _mask(mask)
    {
    }
    SKR_INLINE bool operator~() const
    {
        SKR_ASSERT(_data != nullptr && "BitRef is invalid");
        return !((*_data) & _mask);
    }
    SKR_INLINE operator bool() const
    {
        SKR_ASSERT(_data != nullptr && "BitRef is invalid");
        return ((*_data) & _mask);
    }
    SKR_INLINE BitRef& operator=(bool v)
    {
        SKR_ASSERT(_data != nullptr && "BitRef is invalid");
        (*_data) = v ? (*_data) | _mask : (*_data) & ~_mask;
        return *this;
    }
    SKR_INLINE BitRef& operator=(const BitRef& rhs)
    {
        *this = (bool)rhs;
        return *this;
    }
    SKR_INLINE void flip()
    {
        SKR_ASSERT(_data != nullptr && "BitRef is invalid");
        (*_data) ^= _mask;
    }

    SKR_INLINE bool is_valid() const { return _data != nullptr; }

private:
    TBlock* _data = nullptr;
    TBlock  _mask = Algo::EmptyMask;
};

template <typename TBlock, typename TS>
struct BitDataRef {
    BitRef<TBlock> data  = {};
    TS             index = npos_of<TS>;

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

    inline                 operator bool() const { return data.is_valid() || index != npos_of<TS>; }
    inline BitRef<TBlock>& operator*() const { return *data; }
};
} // namespace skr::container