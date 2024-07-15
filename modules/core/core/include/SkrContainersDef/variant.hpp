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