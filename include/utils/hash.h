#pragma once
#include "platform/configure.h"
#if __has_include("emmintrin.h") && !defined(TARGET_CPU_ARM) && !defined(__EMSCRIPTEN__) && !defined(__wasi__)
    #include <emmintrin.h>
#endif
#define XXH_INLINE_ALL
#include "xxhash3/xxhash.h"

#ifdef __cplusplus
extern "C" {
#endif

FORCEINLINE static size_t skr_hash(const void* buffer, size_t size, size_t seed)
{
#if SIZE_MAX == UINT64_MAX
    return XXH64(buffer, size, seed);
#elif SIZE_MAX == UINT32_MAX
    return XXH32(buffer, size, seed);
#else
    #error "unsupported hash size!"
#endif
}

#ifdef __cplusplus
}
#endif
