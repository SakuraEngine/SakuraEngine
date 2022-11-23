#pragma once
#include "platform/configure.h"
#include <variant>

namespace skr
{
    template<class ...Ts>
    using variant = std::variant<Ts...>;

    using std::get_if;
    using std::get;
}