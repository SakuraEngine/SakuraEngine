#pragma once
#include "SkrRT/config.h"

#ifdef __cplusplus
#include <type_traits>
#endif

#if __cplusplus > 201703L
    #define DUAL_UNLIKELY [[unlikely]]
#else
    #define DUAL_UNLIKELY
#endif

// inline defs
#define DUAL_FORCEINLINE SKR_FORCEINLINE

#include "inttypes.h" // IWYU pragma: keep
typedef uint32_t EIndex;
typedef uint32_t TIndex;
typedef uint32_t SIndex;
typedef uint32_t dual_entity_t;

typedef struct dual_entity_debug_proxy_t {
    dual_entity_t value;
} dual_entity_debug_proxy_t;

#define DUAL_ENTITY_ID_MASK 0x00FFFFFF
#define DUAL_ENTITY_VERSION_OFFSET 24
#define DUAL_ENTITY_VERSION_MASK 0x000000FF
#define DUAL_NULL_ENTITY 0xFFFFFFFF
#define DUAL_NULL_TYPE 0xFFFFFFFF

#define DUAL_ENTITY_ID(e) ((e) & DUAL_ENTITY_ID_MASK)
#define DUAL_ENTITY_VERSION(e) (((e) >> DUAL_ENTITY_VERSION_OFFSET) & DUAL_ENTITY_VERSION_MASK)
#define DUAL_ENTITY(id, version) (((version & DUAL_ENTITY_VERSION_MASK) << DUAL_ENTITY_VERSION_OFFSET) | (id & DUAL_ENTITY_ID_MASK))

#ifndef forloop
    #define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

#include "SkrRT/platform/memory.h"

SKR_RUNTIME_API extern const char* kDualMemoryName;
#define dual_malloc(size) sakura_mallocN((size), kDualMemoryName)
#define dual_malloc_aligned(size, alignment) sakura_malloc_alignedN((size), (alignment), kDualMemoryName)
#define dual_calloc(count, size) sakura_callocN((count), (size), kDualMemoryName)
#define dual_calloc_aligned(count, size, alignment) sakura_calloc_alignedN((count), (size), (alignment), kDualMemoryName)
#define dual_memalign dual_malloc_aligned
#define dual_free(ptr) sakura_freeN((ptr), kDualMemoryName)
#define dual_free_aligned(p, alignment) sakura_free_aligned((p), (alignment), kDualMemoryName)
#define dual_realloc(p, newsize) sakura_reallocN((p), (newsize), kDualMemoryName)