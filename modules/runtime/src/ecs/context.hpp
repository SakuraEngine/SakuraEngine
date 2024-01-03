#pragma once
#include "pool.hpp"
#include "scheduler.hpp"
#include "type_registry.hpp"

#include "SkrRT/containers/string.hpp"

struct sugoi_context_t {
    sugoi_context_t();
    sugoi::pool_t normalPool;
    sugoi::pool_t largePool;
    sugoi::pool_t smallPool;
    sugoi::type_registry_t typeRegistry;
    sugoi::scheduler_t scheduler;
    skr::String error;
};