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
#include <EASTL/variant.h>
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
}

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
}
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
}
}
// BITSET

// VARIANT
namespace skr
{
template <class... Ts>
using variant = eastl::variant<Ts...>;
template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;
using eastl::get_if;
using eastl::get;
using eastl::visit;
using eastl::variant_size_v;
using eastl::variant_npos;
} // namespace skr

namespace skr
{
namespace binary
{
template <class... Ts>
struct ReadTrait<skr::variant<Ts...>> {
    template <size_t I, class T>
    static int ReadByIndex(skr_binary_reader_t* archive, skr::variant<Ts...>& value, size_t index)
    {
        if (index == I)
        {
            T t;
            SKR_ARCHIVE(t);
            value = std::move(t);
            return 0;
        }
        return -1;
    }

    template <size_t... Is>
    static int ReadByIndexHelper(skr_binary_reader_t* archive, skr::variant<Ts...>& value, size_t index, std::index_sequence<Is...>)
    {
        int result;
        (void)(((result = ReadByIndex<Is, Ts>(archive, value, index)) != 0) && ...);
        return result;
    }

    static int Read(skr_binary_reader_t* archive, skr::variant<Ts...>& value)
    {
        uint32_t index;
        SKR_ARCHIVE(index);
        if (index >= sizeof...(Ts))
            return -1;
        return ReadByIndexHelper(archive, value, index, std::make_index_sequence<sizeof...(Ts)>());
    }
};

} // namespace binary

template <class... Ts>
struct SerdeCompleteChecker<binary::ReadTrait<skr::variant<Ts...>>>
    : std::bool_constant<(is_complete_serde_v<binary::ReadTrait<Ts>> && ...)> {
};
} // namespace skr

namespace skr
{
namespace binary
{
template <class... Ts>
struct WriteTrait<skr::variant<Ts...>> {
    static int Write(skr_binary_writer_t* archive, const skr::variant<Ts...>& variant)
    {
        SKR_ARCHIVE((uint32_t)variant.index());
        int ret;
        skr::visit([&](auto&& value) {
            ret = skr::binary::Archive(archive, value);
        },
                     variant);
        return ret;
    }
};
} // namespace binary
template <class... Ts>
struct SerdeCompleteChecker<binary::WriteTrait<skr::variant<Ts...>>>
    : std::bool_constant<(is_complete_serde_v<binary::WriteTrait<Ts>> && ...)> {
};
} // namespace skr
// VARIANT