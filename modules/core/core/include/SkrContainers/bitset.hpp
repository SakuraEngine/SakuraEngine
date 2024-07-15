#pragma once
#include "SkrContainersDef/bitset.hpp"

// bin serde
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
namespace skr::binary
{
template <size_t N, typename TBlock>
struct WriteTrait<skr::Bitset<N, TBlock>> {
    static bool Write(SBinaryWriter* archive, const skr::Bitset<N, TBlock>& value)
    {
        for (int i = 0; i < Bitset<N, TBlock>::NumBlock; i++)
        {
            if (!skr::binary::Write(archive, (value.data()[i]))) return false;
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
            if (!skr::binary::Read(archive, (value.data()[i]))) return false;
        }
        return true;
    }
};
} // namespace skr::binary