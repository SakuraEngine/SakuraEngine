#pragma once
#include "SkrContainers/stl_vector.hpp"
#include "SkrContainers/hashmap.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrRT/ecs/storage.hpp"
#include "SkrRT/ecs/entity_registry.hpp"
#include "./../query.hpp"
#include "./../arena.hpp"
#include "./../pool.hpp"
#include "./../stack.hpp"

namespace sugoi
{
extern thread_local fixed_stack_t localStack;
}

struct sugoi_storage_t::Impl {
    using queries_t = skr::stl_vector<sugoi_query_t*>;
    using groups_t = skr::FlatHashMap<sugoi_entity_type_t, sugoi_group_t*, sugoi::hasher<sugoi_entity_type_t>, sugoi::equalto<sugoi_entity_type_t>>;
    using archetypes_t = skr::FlatHashMap<sugoi_type_set_t, archetype_t*, sugoi::hasher<sugoi_type_set_t>, sugoi::equalto<sugoi_type_set_t>>;
    using phase_alias_t = skr::FlatHashMap<skr::StringView, sugoi_phase_alias_t, skr::Hash<skr::StringView>>;
    using batchmap_t = skr::FlatHashMap<sugoi_chunk_t*, sugoi_chunk_view_t>;

    Impl();

    archetypes_t archetypes;
    queries_t queries;
    phase_alias_t aliases;
    uint32_t aliasCount = 0;
    sugoi::phase_entry** phases = nullptr;
    uint32_t phaseCount = 0;
    bool queriesBuilt = false;
    groups_t groups;
    sugoi::block_arena_t archetypeArena;
    sugoi::block_arena_t queryBuildArena;
    sugoi::fixed_pool_t groupPool;
    sugoi::EntityRegistry entity_registry;
    sugoi_timestamp_t timestamp;
    std::unique_ptr<sugoi_timestamp_t[]> typeTimestamps;
    mutable sugoi::scheduler_t* scheduler;
    mutable void* currentFiber;
    skr::task::counter_t counter;
};