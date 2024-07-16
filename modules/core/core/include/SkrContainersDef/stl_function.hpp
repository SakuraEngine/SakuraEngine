#pragma once
#include <functional>

namespace skr
{
template <typename T>
using stl_function = std::function<T>;
}