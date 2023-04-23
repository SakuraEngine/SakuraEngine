#pragma once
#include "ecs/dual.h"
#include "ecs/SmallVector.h"
#include <EASTL/vector.h>
#include <EASTL/unique_ptr.h>
#include "containers/string.hpp"
#include "containers/span.hpp"
#include "containers/hashmap.hpp"

namespace dual
{
struct phase_entry {
    dual_type_index_t type;
    uint32_t phase;
    skr::span<dual_query_t*> queries;
    skr::flat_hash_map<dual_group_t*, dual_query_t*> include;
};
skr::string& get_error();
} // namespace dual


struct dual_query_t {
    dual_storage_t* storage = nullptr;
    dual_filter_t filter;
    dual_meta_filter_t meta;
    dual_parameters_t parameters;
    eastl::vector<uint8_t> data;
    llvm_vecsmall::SmallVector<dual_query_t*, 2> subqueries;

    //cache
    dual::phase_entry** phases = nullptr;
    uint32_t phaseCount = 0;
    llvm_vecsmall::SmallVector<dual_type_set_t, 4> excludes;
    bool includeDisabled = false;
    bool includeDead = false;
    llvm_vecsmall::SmallVector<dual_group_t*, 32> groups;
    using iterator = eastl::vector<dual_group_t*>::iterator;
};