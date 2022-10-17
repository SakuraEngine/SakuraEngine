#pragma once
#include "platform/configure.h"

#if __cplusplus > 201703L
    #define DUAL_UNLIKELY [[unlikely]]
#else
    #define DUAL_UNLIKELY
#endif

// inline defs
#ifndef DUAL_FORCEINLINE
    #if defined(_MSC_VER)
        #define DUAL_FORCEINLINE __forceinline
    #else
        #define DUAL_FORCEINLINE inline
    #endif
#endif

#include "inttypes.h"
typedef uint32_t EIndex;
typedef uint32_t TIndex;
typedef uint16_t SIndex;
typedef uint32_t dual_entity_t;

typedef struct dual_entity_debug_proxy_t {
    dual_entity_t value;
} dual_entity_debug_proxy_t;

#define DUAL_ENTITY_ID_MASK 0x00FFFFFF
#define DUAL_ENTITY_VERSION_OFFSET 24
#define DUAL_ENTITY_VERSION_MASK 0x000000FF
#define DUAL_NULL_ENTITY 0xFFFFFFFF
#define DUAL_NULL_TYPE 0xFFFFFFFF

#ifndef forloop
    #define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

#include "platform/memory.h"

#define dual_malloc sakura_malloc
#define dual_malloc_aligned sakura_malloc_aligned
#define dual_calloc sakura_calloc
#define dual_calloc_aligned sakura_calloc_aligned
#define dual_memalign sakura_malloc_aligned
#define dual_free sakura_free
#define dual_free_aligned sakura_free_aligned
#define dual_realloc sakura_realloc