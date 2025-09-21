#pragma once
#include "SkrContainersDef/bitset.hpp"

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <size_t N, typename TBlock>
struct Serialize<skr::Bitset<N, TBlock>>
{
    inline static constexpr auto kNumBlock = skr::Bitset<N, TBlock>::NumBlock;
    inline static void read(ArchiveRead& r, skr::Bitset<N, TBlock>& v)
    {
        Archive::ArrayScope arr_scope(r);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        uint64_t adjusted_count = kNumBlock;
        if (r.is_structured())
        { // check size
            uint64_t arr_count;
            SKR_FAST_CHECK(r.array_size_structured(arr_count), );
            SKR_FAST_CHECK(r.adjust_array_size(arr_count, kNumBlock, adjusted_count), );
        }

        for (int i = 0; i < adjusted_count; i++)
        {
            SKR_FAST_CHECK(r.value(v.data()[i]), );
        }
    }
    inline static void write(ArchiveWrite& w, const skr::Bitset<N, TBlock>& v)
    {
        Archive::ArrayScope arr_scope(w);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        for (int i = 0; i < kNumBlock; i++)
        {
            SKR_FAST_CHECK(w.value(v.data()[i]), );
        }
    }
};
} // namespace skr