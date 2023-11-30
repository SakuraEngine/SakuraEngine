#pragma once
#include <optional>

namespace skr
{
    template<class T>
    using optional = std::optional<T>;
    constexpr auto nullopt = std::nullopt;
}