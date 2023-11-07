#include "SkrBase/misc/hash.h"
#if __has_include("emmintrin.h") && !defined(TARGET_CPU_ARM) && !defined(__EMSCRIPTEN__) && !defined(__wasi__)
    #include <emmintrin.h>
#endif
#define XXH_INLINE_ALL
#include "xxhash3/xxhash.h"

size_t skr_hash(const void* buffer, size_t size, size_t seed)
{
#if SIZE_MAX == UINT64_MAX
    return XXH64(buffer, size, seed);
#elif SIZE_MAX == UINT32_MAX
    return XXH32(buffer, size, seed);
#else
    #error "unsupported hash size!"
#endif
}

uint64_t skr_hash64(const void* buffer, uint64_t size, uint64_t seed)
{
    return XXH3_64bits_withSeed(buffer, size, seed);
}

uint32_t skr_hash32(const void* buffer, uint32_t size, uint32_t seed)
{
    return XXH32(buffer, size, seed);
}