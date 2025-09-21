#pragma once
#include <SkrBase/config.h>
#include <functional>

namespace skr
{
template <typename T>
using stl_function [[sfinal_alias]] = std::function<T>;
}