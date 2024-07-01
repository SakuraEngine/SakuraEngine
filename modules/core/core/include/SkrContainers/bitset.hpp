#pragma once
#include "SkrBase/types.h"
#include "SkrBase/containers/bit_set/bit_set.hpp"

namespace skr
{
template <size_t N, typename TBlock = std::conditional_t<N <= 32, uint32_t, uint64_t>>
using Bitset = container::Bitset<N, TBlock>;
}

namespace skr::binary
{
template <size_t N, typename TBlock>
struct WriteTrait<skr::Bitset<N, TBlock>> {
    static bool Write(SBinaryWriter* archive, const skr::Bitset<N, TBlock>& value)
    {
        for (int i = 0; i < Bitset<N, TBlock>::NumBlock; i++)
        {
            SKR_ARCHIVE(value.data()[i]);
        }
        return true;
    }
};

template <size_t N, typename TBlock>
struct ReadTrait<skr::Bitset<N, TBlock>> {
    static bool Read(SBinaryReader* archive, skr::Bitset<N, TBlock>& value)
    {
        for (int i = 0; i < Bitset<N, TBlock>::NumBlock; i++)
        {
            SKR_ARCHIVE(value.data()[i]);
        }
        return true;
    }
};
} // namespace skr::binary