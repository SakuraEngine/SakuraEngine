
#include "chunk.hpp"
#include "pool.hpp"
#include "ecs/constants.hpp"
#include <stdint.h>
#include "pool.hpp"
#include "archetype.hpp"

dual_chunk_t* dual_chunk_t::create(dual::pool_type_t poolType)
{
    using namespace dual;
    switch (poolType)
    {
        case PT_small:
            return new (get_default_pool_small().allocate()) dual_chunk_t(poolType);
        case PT_default:
            return new (get_default_pool().allocate()) dual_chunk_t(poolType);
        case PT_large:
            return new (get_default_pool_large().allocate()) dual_chunk_t(poolType);
    };
    return nullptr;
}

void dual_chunk_t::destroy(dual_chunk_t* chunk)
{
    using namespace dual;
    switch (chunk->pt)
    {
        case PT_small:
            return get_default_pool_small().free(chunk);
        case PT_default:
            return get_default_pool().free(chunk);
        case PT_large:
            return get_default_pool_large().free(chunk);
    };
}

const dual_entity_t* dual_chunk_t::get_entities() const
{
    return (const dual_entity_t*)data();
}

uint32_t* dual_chunk_t::timestamps() noexcept
{
    return (uint32_t*)(data() + type->versionOffset[pt]);
}

EIndex dual_chunk_t::get_capacity()
{
    return type->chunkCapacity[pt];
}

extern "C" {
dual_group_t* dualC_get_group(const dual_chunk_t* chunk)
{
    return chunk->group;
}

dual_storage_t* dualC_get_storage(const dual_chunk_t* chunk)
{
    return chunk->group->archetype->storage;
}

uint32_t dualC_get_count(const dual_chunk_t* chunk)
{
    return chunk->count;
}
}