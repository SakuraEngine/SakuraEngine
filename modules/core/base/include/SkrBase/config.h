#pragma once
// platform headers
#ifndef __cplusplus
    #include <stdbool.h>
#endif

#if __has_include("sys/types.h")
    #include <sys/types.h>
#endif

#if __has_include("stdint.h")
    #include <stdint.h>
#endif

#if __has_include("float.h")
    #include <float.h>
#endif

// platform & compiler marcos
#include "SkrBase/config/platform.h"
#include "SkrBase/config/compiler.h"

// keywords
#include "SkrBase/config/key_words.h"
#include "SkrBase/config/values.h"