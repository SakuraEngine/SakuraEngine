#pragma once
#include "ecs/dual.h"
#include "ecs/SmallVector.h"
#include <EASTL/vector.h>
#include <EASTL/unique_ptr.h>
#include "containers/string.hpp"

namespace dual
{
struct query_cache_t {
    query_cache_t() = default;
    eastl::vector<uint8_t> data;
    query_cache_t(query_cache_t&&) = default;
    query_cache_t& operator=(query_cache_t&&) = default;
    query_cache_t(const query_cache_t&) = delete;
    query_cache_t& operator=(const query_cache_t&) = delete;
    bool includeDisabled = false;
    bool includeDead = false;
    dual_filter_t filter;
    llvm_vecsmall::SmallVector<dual_group_t*, 32> groups;
    using iterator = eastl::vector<dual_group_t*>::iterator;
};

skr::string& get_error();
} // namespace dual

struct dual_query_t {
    dual_storage_t* storage = nullptr;
    dual_filter_t filter;
    dual_meta_filter_t meta;
    dual_filter_t buildedFilter;
    dual_parameters_t parameters;
};