#pragma once

#include "ecs/dual.h"
#include "SmallVector.h"
#include <vector>
#include <string>

namespace dual
{
struct query_cache_t {
    std::unique_ptr<char[]> data;
    bool includeDisabled;
    bool includeDead;
    dual_filter_t filter;
    llvm_vecsmall::SmallVector<dual_group_t*, 32> groups;
    using iterator = std::vector<dual_group_t*>::iterator;
};

std::string& get_error();
} // namespace dual

struct dual_query_t {
    dual_storage_t* storage;
    dual_filter_t filter;
    dual_meta_filter_t meta;
    bool built = false;
    dual_filter_t buildedFilter;
    dual_parameters_t parameters;
};