#pragma once

#include "ecs/dual.h"
#include "ecs/SmallVector.h"
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>

namespace dual
{
struct query_cache_t {
    eastl::unique_ptr<char[]> data;
    bool includeDisabled;
    bool includeDead;
    dual_filter_t filter;
    llvm_vecsmall::SmallVector<dual_group_t*, 32> groups;
    using iterator = eastl::vector<dual_group_t*>::iterator;
};

eastl::string& get_error();
} // namespace dual

struct dual_query_t {
    dual_storage_t* storage;
    dual_filter_t filter;
    dual_meta_filter_t meta;
    dual_filter_t buildedFilter;
    dual_parameters_t parameters;
};