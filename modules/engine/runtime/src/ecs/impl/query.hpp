#pragma once
#include "SkrBase/atomic/atomic_mutex.hpp"
#include "SkrRT/ecs/query.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainers/hashmap.hpp"

namespace sugoi
{
struct phase_entry {
    sugoi_type_index_t type;
    uint32_t phase;
    skr::span<sugoi_query_t*> queries;
    skr::FlatHashMap<sugoi_group_t*, sugoi_query_t*> include;
};
skr::String& get_error();
} // namespace sugoi

struct sugoi_query_t::Impl {
    sugoi_storage_t* storage = nullptr;
    sugoi_filter_t filter;
    sugoi_meta_filter_t meta;
    skr::InlineVector<sugoi_entity_t, 1> all_meta;
    skr::InlineVector<sugoi_entity_t, 1> none_meta;
    skr::InlineVector<sugoi_type_index_t, 1> changed;
    sugoi_parameters_t parameters;

    const bool includeAlias = false;
    const bool includeDisabled = false;
    const bool includeDead = false;

    skr::Vector<uint8_t> data;
    skr::InlineVector<sugoi_query_t*, 2> subqueries;
    sugoi_custom_filter_callback_t customFilter = nullptr;
    void* customFilterUserData = nullptr;

    // cache
    struct GroupsCache {
        using GroupsCacheVector = skr::InlineVector<sugoi_group_t*, 32>;
        template<typename F>
        void read(const F& func) const
        {
            mtx.lock_shared();
            func(groups);
            mtx.unlock_shared();
        }

        template<typename F>
        void write(const F& func)
        {
            mtx.lock();
            func(groups);
            mtx.unlock();
        }
    private:
        friend struct sugoi_storage_t;
        sugoi_timestamp_t group_timestamp = 0;
        mutable skr::shared_atomic_mutex mtx;
        GroupsCacheVector groups;
    } groups_cache;

    // overload cache
    struct OverloadCache {
        sugoi::phase_entry** phases = nullptr;
        uint32_t phaseCount = 0;
        skr::InlineVector<sugoi_type_set_t, 4> excludes;
    } overload_cache;
};