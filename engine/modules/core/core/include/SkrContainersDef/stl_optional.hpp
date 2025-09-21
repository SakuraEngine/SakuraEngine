#pragma once
#include <SkrBase/config.h>
#include <optional>

namespace skr
{
template <class T>
using stl_optional [[sfinal_alias]] = std::optional<T>;
constexpr auto nullopt = std::nullopt;
} // namespace skr