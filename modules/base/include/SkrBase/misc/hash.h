#pragma once
#include "SkrBase/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SKR_DEFAULT_HASH_SEED_32 1610612741
#define SKR_DEFAULT_HASH_SEED_64 8053064571610612741
#if UINTPTR_MAX == UINT32_MAX
#define SKR_DEFAULT_HASH_SEED SKR_DEFAULT_HASH_SEED_32
#else
#define SKR_DEFAULT_HASH_SEED SKR_DEFAULT_HASH_SEED_64
#endif

SKR_EXTERN_C SKR_STATIC_API 
size_t skr_hash(const void* buffer, size_t size, size_t seed);

SKR_EXTERN_C SKR_STATIC_API 
uint64_t skr_hash64(const void* buffer, uint64_t size, uint64_t seed);

SKR_EXTERN_C SKR_STATIC_API 
uint32_t skr_hash32(const void* buffer, uint32_t size, uint32_t seed);

#ifdef __cplusplus
}
#endif