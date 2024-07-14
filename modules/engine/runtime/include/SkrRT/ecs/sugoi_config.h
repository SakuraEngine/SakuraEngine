#pragma once
#include "SkrRT/config.h"

#ifdef __cplusplus
#include <type_traits>
#endif

#if __cplusplus > 201703L
    #define SUGOI_UNLIKELY [[unlikely]]
#else
    #define SUGOI_UNLIKELY
#endif

// inline defs
#define SUGOI_FORCEINLINE SKR_FORCEINLINE

#include "inttypes.h" // IWYU pragma: keep
typedef uint32_t EIndex;
typedef uint32_t TIndex;
typedef uint32_t SIndex;
typedef uint32_t sugoi_entity_t;
typedef uint32_t sugoi_timestamp_t;

typedef struct sugoi_entity_debug_proxy_t {
    sugoi_entity_t value;
} sugoi_entity_debug_proxy_t;

#define SUGOI_ENTITY_ID_MASK 0x00FFFFFF
#define SUGOI_ENTITY_VERSION_OFFSET 24
#define SUGOI_ENTITY_VERSION_MASK 0x000000FF
#define SUGOI_NULL_ENTITY 0xFFFFFFFF
#define SUGOI_NULL_TYPE 0xFFFFFFFF

#define SUGOI_ENTITY_ID(e) ((e) & SUGOI_ENTITY_ID_MASK)
#define SUGOI_ENTITY_VERSION(e) (((e) >> SUGOI_ENTITY_VERSION_OFFSET) & SUGOI_ENTITY_VERSION_MASK)
#define SUGOI_ENTITY(id, version) (((version & SUGOI_ENTITY_VERSION_MASK) << SUGOI_ENTITY_VERSION_OFFSET) | (id & SUGOI_ENTITY_ID_MASK))

#ifndef forloop
    #define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

#include "SkrCore/memory/memory.h"

SKR_RUNTIME_API extern const char* kDualMemoryName;
#define sugoi_malloc(size) sakura_mallocN((size), kDualMemoryName)
#define sugoi_malloc_aligned(size, alignment) sakura_malloc_alignedN((size), (alignment), kDualMemoryName)
#define sugoi_calloc(count, size) sakura_callocN((count), (size), kDualMemoryName)
#define sugoi_calloc_aligned(count, size, alignment) sakura_calloc_alignedN((count), (size), (alignment), kDualMemoryName)
#define sugoi_memalign sugoi_malloc_aligned
#define sugoi_free(ptr) sakura_freeN((ptr), kDualMemoryName)
#define sugoi_free_aligned(p, alignment) sakura_free_alignedN((p), (alignment), kDualMemoryName)
#define sugoi_realloc(p, newsize) sakura_reallocN((p), (newsize), kDualMemoryName)