#pragma once
// platform & compiler marcos
#include "SkrBase/config/platform.h"
#include "SkrBase/config/compiler.h"

// keywords
// #include "SkrBase/config/key_words.h"

// TODO. remove this
// INLINE
#if defined(__cplusplus)
    #define SKR_INLINE inline
#else
    #define SKR_INLINE
#endif