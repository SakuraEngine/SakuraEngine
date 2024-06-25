#pragma once
#include "SkrRT/ecs/storage.hpp"
#include "./../query.hpp"
#include "./../arena.hpp"
#include "./../pool.hpp"
#include "./../stack.hpp"

namespace sugoi
{
extern thread_local fixed_stack_t localStack;
}

struct sugoi_storage_t::Impl {
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