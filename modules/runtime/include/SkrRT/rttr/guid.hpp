#pragma once
#include "SkrRT/platform/guid.hpp"

// FIXME. temporal solution
#define SKR_IS_BIG_ENDIAN 0
#define SKR_IS_LITTLE_ENDIAN 1

namespace skr
{
using GUID = skr_guid_t;
} // namespace skr