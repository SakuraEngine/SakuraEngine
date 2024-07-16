#pragma once
#include "SkrContainersDef/bitset.hpp"

// bin serde
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <size_t N, typename TBlock>
struct BinSerde<skr::Bitset<N, TBlock>> {
    inline static bool read(SBinaryReader* r, skr::Bitset<N, TBlock>& v)
    {
        for (int i = 0; i < Bitset<N, TBlock>::NumBlock; i++)
        {
            if (!bin_read(r, v.data()[i])) return false;
        }
        return true;
    }
    inline static bool write(SBinaryWriter* w, const skr::Bitset<N, TBlock>& v)
    {
        for (int i = 0; i < Bitset<N, TBlock>::NumBlock; i++)
        {
            if (!bin_write(w, v.data()[i])) return false;
        }
        return true;
    }
};
} // namespace skr