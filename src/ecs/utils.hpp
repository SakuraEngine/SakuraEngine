#pragma once
#include "platform/configure.h"
#include "platform/memory.h"

#ifdef _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
FORCEINLINE static void* _aligned_calloc(size_t nelem, size_t elsize, size_t alignment)
{
    void* memory = _aligned_malloc(nelem * elsize, alignment);
    if (memory != NULL) memset(memory, 0, nelem * elsize);
    return memory;
}
    #define dual_malloc malloc
    #define dual_malloc_aligned _aligned_malloc
    #define dual_calloc calloc
    #define dual_calloc_aligned _aligned_calloc
    #define dual_memalign _aligned_malloc
    #define dual_free free
    #define dual_free_aligned _aligned_free
    #define DEBUG_NEW_SOURCE_LINE (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
    #define dual_malloc sakura_malloc
    #define dual_malloc_aligned sakura_malloc_aligned
    #define dual_calloc sakura_calloc
    #define dual_calloc_aligned sakura_calloc_aligned
    #define dual_memalign sakura_malloc_aligned
    #define dual_free sakura_free
    #define dual_free_aligned sakura_free
    #define DEBUG_NEW_SOURCE_LINE
#endif