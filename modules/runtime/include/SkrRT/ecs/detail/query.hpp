#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/SmallVector.h"

#include "SkrRT/containers/string.hpp"
#include "SkrRT/containers/span.hpp"
#include "SkrRT/containers/hashmap.hpp"

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
    sugoi_parameters_t parameters;
    skr::Vector<uint8_t> data;
    llvm_vecsmall::SmallVector<sugoi_query_t*, 2> subqueries;

    //cache
    sugoi::phase_entry** phases = nullptr;
    uint32_t phaseCount = 0;
    llvm_vecsmall::SmallVector<sugoi_type_set_t, 4> excludes;
    bool includeDisabled = false;
    bool includeDead = false;
    llvm_vecsmall::SmallVector<sugoi_group_t*, 32> groups;
};