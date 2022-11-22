#pragma once
#include <variant>

namespace skr
{
    template<class ...Ts>
    using variant = std::variant<Ts...>;
}