#pragma once

#include "EASTL/bitset.h"
#include "serde/binary/writer_fwd.h"
#include "serde/binary/reader_fwd.h"


namespace skr
{
    template<size_t N, typename WordType = uint64_t>
    using bitset = eastl::bitset<N, WordType>;
}


namespace skr
{
namespace binary
{
    template <size_t N, typename WordType>
    struct WriteTrait<const skr::bitset<N, WordType>&> {
        static int Write(skr_binary_writer_t* archive, const skr::bitset<N, WordType>& value)
        {
            for(int i = 0; i < N / (sizeof(WordType) * 8); i++)
            {
                SKR_ARCHIVE(value.data()[i]);
            }
            return 0;
        }
    };

    template <size_t N, typename WordType>
    struct ReadTrait<skr::bitset<N, WordType>> {
        static int Read(skr_binary_reader_t* archive, skr::bitset<N, WordType>& value)
        {
            for(int i = 0; i < N / (sizeof(WordType) * 8); i++)
            {
                SKR_ARCHIVE(value.data()[i]);
            }
            return 0;
        }
    };
}
}