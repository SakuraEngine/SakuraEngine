#pragma once
#include "SkrBase/types.h"
#include "SkrBase/containers/bit_set/bit_set.hpp"

namespace skr
{
template <size_t N, typename TBlock = uint64_t>
using Bitset = container::Bitset<N, TBlock>;
}

namespace skr::binary
{
template <size_t N, typename TBlock>
struct WriteTrait<skr::Bitset<N, TBlock>> {
    static int Write(skr_binary_writer_t* archive, const skr::Bitset<N, TBlock>& value)
    {
        for (int i = 0; i < Bitset<N, TBlock>::NumBlock; i++)
        {
            SKR_ARCHIVE(value.data()[i]);
        }
        return 0;
    }
};

template <size_t N, typename TBlock>
struct ReadTrait<skr::Bitset<N, TBlock>> {
    static int Read(skr_binary_reader_t* archive, skr::Bitset<N, TBlock>& value)
    {
        for (int i = 0; i < Bitset<N, TBlock>::NumBlock; i++)
        {
            SKR_ARCHIVE(value.data()[i]);
        }
        return 0;
    }
};
} // namespace skr::binary