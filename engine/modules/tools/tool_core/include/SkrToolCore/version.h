#pragma once
#include "SkrRuntime/config.h"
#include "SkrBase/config.h"
#include "SkrToolCore/version.generated.h"

struct [[sattr(
    guid = "9DC58264-310C-C98B-DC08-6571E8D07146"
)]] TOOL_CORE_API skr_version_t
{
    uint32_t v;
};
typedef struct skr_version_t skr_version_t;