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

#define ENTITY_ID_MASK 0x00FFFFFF
#define ENTITY_VERSION_OFFSET 24
#define ENTITY_VERSION_MASK 0x000000FF


#define forloop(i, z, n) for(auto i = std::decay_t<decltype(n)>(z); i<(n); ++i)
