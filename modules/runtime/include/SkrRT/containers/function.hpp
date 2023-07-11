#pragma once
#include "EASTL/functional.h"

namespace skr
{
    template<typename T>
    using function = eastl::function<T>;
}