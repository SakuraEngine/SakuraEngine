#pragma once
#include "SkrRT/platform/configure.h"
#include "SkrToolCore/module.configure.h"
#ifndef __meta__
    #include "SkrToolCore/version.generated.h"
#endif

sreflect_struct("guid" : "9DC58264-310C-C98B-DC08-6571E8D07146")
TOOL_CORE_API skr_version_t {
    uint32_t v;
};
typedef struct skr_version_t skr_version_t;