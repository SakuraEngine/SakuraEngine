#pragma once
#include "SkrBase/misc/assert.h"
#include "SkrBase/misc/debugbreak.h"

#ifdef __cplusplus
    #include "SkrBase/misc/bit.hpp"
    #include "SkrBase/misc/integer_tools.hpp"
#endif

#define SKR_DEBUG_BREAK() debug_break()