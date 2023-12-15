#pragma once
#include "SkrRT/config.h"
#include "SkrRT/serde/binary/writer_fwd.h"
#include "SkrRT/serde/binary/reader_fwd.h"
// TODO: REMOVE EASTL
#include <EASTL/bitset.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/fixed_set.h>
#include <EASTL/fixed_string.h>
#include <EASTL/fixed_map.h>
#include <EASTL/shared_ptr.h>

// SMART POINTER
namespace skr
{
template <typename T>
using shared_ptr = eastl::shared_ptr<T>;

template <typename T>
using weak_ptr = eastl::weak_ptr<T>;

template <typename T>
using unique_ptr = eastl::unique_ptr<T>;

using eastl::make_shared;
using eastl::make_unique;
} // namespace skr

// FIXED CONTAINERS
namespace skr
{
template <typename T, size_t N>
using FixedVector = eastl::fixed_vector<T, N>;

template <typename T, size_t N>
using FixedSet = eastl::fixed_set<T, N>;

template <typename K, typename V, size_t N>
using FixedMap = eastl::fixed_map<K, V, N>;

template <typename CharT, size_t N>
using FixedString = eastl::fixed_string<CharT, N>;
} // namespace skr
// FIXED CONTAINERS

// BITSET
namespace skr
{
template <size_t N, typename WordType = uint64_t>
using bitset = eastl::bitset<N, WordType>;
}

namespace skr
{
namespace binary
{
template <size_t N, typename WordType>
struct WriteTrait<skr::bitset<N, WordType>> {
    static int Write(skr_binary_writer_t* archive, const skr::bitset<N, WordType>& value)
    {
        for (int i = 0; i <= N / (sizeof(WordType) * 8); i++)
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
        for (int i = 0; i <= N / (sizeof(WordType) * 8); i++)
        {
            SKR_ARCHIVE(value.data()[i]);
        }
        return 0;
    }
};
} // namespace binary
} // namespace skr
  // BITSET