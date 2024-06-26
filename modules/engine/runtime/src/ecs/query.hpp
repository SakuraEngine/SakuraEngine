#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"

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


struct sugoi_query_t {
    sugoi_storage_t* storage = nullptr;
    sugoi_filter_t filter;
    
    sugoi_meta_filter_t meta;
    skr::InlineVector<sugoi_entity_t, 1> all_meta;
    skr::InlineVector<sugoi_entity_t, 1> none_meta;
    skr::InlineVector<sugoi_type_index_t, 1> changed;

    sugoi_parameters_t parameters;
    skr::Vector<uint8_t> data;
    skr::InlineVector<sugoi_query_t*, 2> subqueries;
    sugoi_custom_filter_callback_t customFilter = nullptr;
    void* customFilterUserData = nullptr;

    //cache
    sugoi::phase_entry** phases = nullptr;
    uint32_t phaseCount = 0;
    skr::InlineVector<sugoi_type_set_t, 4> excludes;
    bool includeDisabled = false;
    bool includeDead = false;
    skr::InlineVector<sugoi_group_t*, 32> groups;
};