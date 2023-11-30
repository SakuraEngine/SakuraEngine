#pragma once
#include <functional>

namespace skr
{
    template<typename T>
    using function = std::function<T>;
}