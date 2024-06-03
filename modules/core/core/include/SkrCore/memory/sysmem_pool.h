#pragma once
#include "SkrCore/memory/memory.h"

typedef struct SSysMemoryPoolDesc {
    size_t size;
    const char* pool_name;
    bool use_large_page;
} SSysMemoryPoolDesc;

typedef struct SSysMemoryPool SSysMemoryPool;
typedef struct SSysMemoryPool* SSysMemoryPoolId;
SKR_EXTERN_C SKR_CORE_API SSysMemoryPoolId sakura_sysmem_pool_create(const SSysMemoryPoolDesc* pdesc);
SKR_EXTERN_C SKR_CORE_API void sakura_sysmem_pool_destroy(SSysMemoryPoolId pool);
SKR_EXTERN_C SKR_CORE_API void* _sakura_sysmem_pool_malloc(SSysMemoryPoolId pool, size_t size);
SKR_EXTERN_C SKR_CORE_API void* _sakura_sysmem_pool_free(SSysMemoryPoolId pool, void* ptr);

#if defined(SKR_PROFILE_ENABLE) && defined(TRACY_TRACE_ALLOCATION)

SKR_FORCEINLINE void* SkrSysMemPoolMallocWithCZone(SSysMemoryPoolId pool, size_t size, const char* line)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_sysmem_pool_malloc(pool, size);
    SkrCZoneEnd(z);
    return ptr;
}

SKR_FORCEINLINE void* SkrSysMemPoolFreeWithCZone(SSysMemoryPoolId pool, void* ptr, const char* line)
{
    SkrCZoneC(z, SKR_DEALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    _sakura_sysmem_pool_free(pool, ptr);
    SkrCZoneEnd(z);
    return ptr;
}

// thread unsafe
#define sakura_sysmem_pool_malloc(pool, size) SkrSysMemPoolMallocWithCZone((pool), (size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )
#define sakura_sysmem_pool_free(pool, p) SkrSysMemPoolFreeWithCZone((pool), (p), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__),SKR_ALLOC_STRINGFY(__LINE__)) )

#else

// thread unsafe
#define sakura_sysmem_pool_malloc(pool, size) _sakura_sysmem_pool_malloc((pool), (size))
#define sakura_sysmem_pool_free(pool, p) _sakura_sysmem_pool_free((pool), (p))

#endif