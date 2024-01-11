#ifndef SKR_MEMORY_IMPL
#define SKR_MEMORY_IMPL
#endif

#include "SkrMemory/sysmem_pool.h"
// NOW MI-MALLOC IS A MUST
#include "mimalloc.h"

typedef struct SSysMemoryPool
{
    mi_arena_id_t arena;
    size_t arena_size;
    mi_heap_t* heap; // TODO: THREAD_LOCAL
    void* start;
    char* name; // keep under cacheline, do not use char[N]
} SSysMemoryPool;

SSysMemoryPoolId sakura_sysmem_pool_create(size_t size, const char* pool_name)
{
    mi_arena_id_t arena_id;
    int err = mi_reserve_os_memory_ex(size, false /* commit */, 
        true /* allow large */, true /*exclusive*/, &arena_id);
    assert(err == 0 && "mi_reserve_os_memory_ex failed");

    SSysMemoryPool* pool = mi_calloc(1, sizeof(SSysMemoryPool));
    pool->arena = arena_id;
    pool->start = mi_arena_area(pool->arena, &pool->arena_size);
    pool->heap = mi_heap_new_in_arena(pool->arena);
    pool->name = mi_strdup(pool_name ? pool_name : "sysmem_pool");
    return (SSysMemoryPoolId)pool;
}

void sakura_sysmem_pool_destroy(SSysMemoryPoolId pool)
{
    SSysMemoryPool* p = (SSysMemoryPool*)pool;
    mi_heap_delete(p->heap);
    mi_free(p->name);
    mi_free(p);
}

void* _sakura_sysmem_pool_malloc(SSysMemoryPoolId pool, size_t size)
{
    SSysMemoryPool* p = (SSysMemoryPool*)pool;
    void* ptr = mi_heap_malloc(p->heap, size);
    SkrCAllocN(ptr, size, p->name);
    return ptr;
}

void* _sakura_sysmem_pool_free(SSysMemoryPoolId pool, void* ptr)
{
    mi_free(ptr);
    SSysMemoryPool* p = (SSysMemoryPool*)pool;
    SkrCFreeN(ptr, p->name);
    return NULL;
}
