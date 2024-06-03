#pragma once
#include "SkrBase/types.h"
#include "SkrBase/containers/variant/variant.hpp"

namespace skr
{
template <class... Ts>
using variant = skr::container::variant<Ts...>;
template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;
using skr::container::get_if;
using skr::container::get;
using skr::container::visit;
using skr::container::variant_size_v;
using skr::container::variant_npos;
} // namespace skr

namespace skr
{
namespace binary
{
template <class... Ts>
struct ReadTrait<skr::variant<Ts...>> {
    template <size_t I, class T>
    static bool ReadByIndex(SBinaryReader* archive, skr::variant<Ts...>& value, size_t index)
    {
        if (index == I)
        {
            T t;
            SKR_ARCHIVE(t);
            value = std::move(t);
            return true;
        }
        return false;
    }

    template <size_t... Is>
    static bool ReadByIndexHelper(SBinaryReader* archive, skr::variant<Ts...>& value, size_t index, std::index_sequence<Is...>)
    {
        bool result;
        (void)(((result = ReadByIndex<Is, Ts>(archive, value, index)) != 0) && ...);
        return result;
    }

    static bool Read(SBinaryReader* archive, skr::variant<Ts...>& value)
    {
        uint32_t index;
        SKR_ARCHIVE(index);
        if (index >= sizeof...(Ts))
            return false;
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
    static int Write(SBinaryWriter* archive, const skr::variant<Ts...>& variant)
    {
        SKR_ARCHIVE((uint32_t)variant.index());
        bool ret;
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