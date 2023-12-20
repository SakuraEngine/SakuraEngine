#pragma once
#include <optional>

namespace skr
{
    template<class T>
    using stl_optional = std::optional<T>;
    constexpr auto nullopt = std::nullopt;
}