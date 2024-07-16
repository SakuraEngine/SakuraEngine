#pragma once
#include "SkrBase/atomic/atomic_mutex.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrContainers/hashmap.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrRT/ecs/storage.hpp"
#include "SkrRT/ecs/entity_registry.hpp"
#include "./query.hpp"
#include "./../arena.hpp"
#include "./../pool.hpp"
#include "./../stack.hpp"

namespace sugoi
{
extern thread_local fixed_stack_t localStack;

struct OverloadData
{
    using phase_alias_t = skr::FlatHashMap<skr::StringView, sugoi_phase_alias_t, skr::Hash<skr::StringView>>;

    skr::shared_atomic_mutex self_mtx;
    sugoi::block_arena_t queryPhaseArena = sugoi::get_default_pool();
    phase_alias_t aliases;
    uint32_t aliasCount = 0;
    sugoi::phase_entry** phases = nullptr;
    uint32_t phaseCount = 0;
};
} // namespace sugoi

template <typename T>
struct Versioned
{
    bool check_version(sugoi_timestamp_t v) const
    {
        SKR_ASSERT(version == v && "Version mismatch");
        return version == v;
    }

    template <typename F, typename VersionGetter>
    void read_versioned(const F& func, const VersionGetter& getter) const
    {
        mtx.lock_shared();
        SKR_DEFER({ mtx.unlock_shared(); });
        
        const sugoi_timestamp_t v = getter();
        check_version(v);
        func(value);
    }

    template <typename F, typename VersionGetter>
    bool update_versioned(const F& func, const VersionGetter& getter)
    {
        mtx.lock();
        SKR_DEFER({ mtx.unlock(); });

        const sugoi_timestamp_t v = getter();
        if (!check_version(v))
            return false;

        func(value);
        skr_atomic_fetch_add_acquire(&version, 1);
        return true;
    }

private:
    T value;
    SAtomicU32 version = 0;
    mutable skr::shared_atomic_mutex mtx;
};

struct sugoi_storage_t::Impl {
public:
    using queries_t = skr::stl_vector<sugoi_query_t*>;
    using groups_t = skr::ParallelFlatHashMap<sugoi_entity_type_t, sugoi_group_t*, sugoi::hasher<sugoi_entity_type_t>, sugoi::equalto<sugoi_entity_type_t>>;
    using archetypes_t = skr::ParallelFlatHashMap<sugoi_type_set_t, archetype_t*, sugoi::hasher<sugoi_type_set_t>, sugoi::equalto<sugoi_type_set_t>>;
    Impl();

    sugoi::block_arena_t archetypeArena;
    sugoi::fixed_pool_t groupPool;

    Versioned<archetypes_t> archetypes;
    Versioned<groups_t> groups;
    Versioned<queries_t> queries;

private:
    friend struct sugoi_storage_t;
    sugoi_timestamp_t groups_timestamp = 0;
    sugoi_timestamp_t archetype_timestamp = 0;
    sugoi_timestamp_t queries_timestamp = 0;

public:
    sugoi::EntityRegistry entity_registry;
    sugoi_timestamp_t storage_timestamp;
    
    // job system
    mutable sugoi::JobScheduler* scheduler;
    mutable void* currentFiber;
    skr::task::counter_t storage_counter;

    // overload
    sugoi::OverloadData overload_data;
};