#pragma once

#include "EASTL/string.h"

namespace skr
{
struct SPath {
    eastl::string fullPath;
    uint64_t hashCode;
};
} // namespace skr