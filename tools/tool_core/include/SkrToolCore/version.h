#pragma once
#include "platform/configure.h"
#include "SkrToolCore/module.configure.h"

sreflect_struct("guid" : "9DC58264-310C-C98B-DC08-6571E8D07146")
TOOL_CORE_API skr_version_t
{
    uint32_t v;
};
typedef struct skr_version_t skr_version_t;