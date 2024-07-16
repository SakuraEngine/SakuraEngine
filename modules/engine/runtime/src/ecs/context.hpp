#pragma once
#include "pool.hpp"
#include "impl/job.hpp"
#include "impl/type_registry.hpp"

#include "SkrContainers/string.hpp"

struct sugoi_context_t {
    sugoi_context_t();
    sugoi::pool_t normalPool;
    sugoi::pool_t largePool;
    sugoi::pool_t smallPool;

    sugoi::TypeRegistry::Impl typeRegistryImpl;
    sugoi::TypeRegistry typeRegistry;

    sugoi::JobScheduler::Impl schedulerImpl;
    sugoi::JobScheduler scheduler;

    skr::String error;
};