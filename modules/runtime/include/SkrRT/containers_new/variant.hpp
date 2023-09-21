#pragma once
#include <EASTL/variant.h>

namespace skr
{
template <typename... TS>
using Variant = eastl::variant<TS...>;
}