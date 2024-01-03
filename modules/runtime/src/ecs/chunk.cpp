
#include "chunk.hpp"
#include "pool.hpp"
#include "pool.hpp"
#include "archetype.hpp"

sugoi_chunk_t* sugoi_chunk_t::create(sugoi::pool_type_t poolType)
{
    using namespace sugoi;
    switch (poolType)
    {
        case PT_small:
            return new (get_default_pool_small().allocate()) sugoi_chunk_t(poolType);
        case PT_default:
            return new (get_default_pool().allocate()) sugoi_chunk_t(poolType);
        case PT_large:
            return new (get_default_pool_large().allocate()) sugoi_chunk_t(poolType);
    };
    return nullptr;
}

void sugoi_chunk_t::destroy(sugoi_chunk_t* chunk)
{
    using namespace sugoi;
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

const sugoi_entity_t* sugoi_chunk_t::get_entities() const
{
    return (const sugoi_entity_t*)data();
}

uint32_t* sugoi_chunk_t::timestamps() noexcept
{
    return (uint32_t*)(data() + type->versionOffset[pt]);
}

EIndex sugoi_chunk_t::get_capacity()
{
    return type->chunkCapacity[pt];
}

extern "C" {
sugoi_group_t* sugoiC_get_group(const sugoi_chunk_t* chunk)
{
    return chunk->group;
}

sugoi_storage_t* sugoiC_get_storage(const sugoi_chunk_t* chunk)
{
    return chunk->group->archetype->storage;
}

uint32_t sugoiC_get_count(const sugoi_chunk_t* chunk)
{
    return chunk->count;
}
}