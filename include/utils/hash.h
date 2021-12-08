#include "platform/configure.h"
#define XXH_INLINE_ALL
#include "xxhash3/xxhash.h"

#ifdef __cplusplus
extern "C" {
#endif

FORCEINLINE static size_t skr_hash(const void* buffer, size_t size, size_t seed)
{
#if __WORDSIZE == 64
    return XXH64(buffer, size, seed);
#elif __WORDSIZE == 32
    return XXH32(buffer, size, seed);
#else
    #error "unsupported hash size!"
#endif
}

#ifdef __cplusplus
}
#endif
