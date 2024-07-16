#pragma once
#include "SkrOS/thread.h"
#include "SkrTask/fib_task.hpp"
#include "SkrContainers/hashmap.hpp"
#include "SkrRT/ecs/job.hpp"
#include "SkrRT/ecs/entity_registry.hpp"

#include "./../archetype.hpp"

namespace sugoi
{
struct JobDependencyEntry {
    skr::stl_vector<skr::task::weak_event_t> owned;
    skr::stl_vector<skr::task::weak_event_t> shared;
};

struct JobScheduler::Impl {
    template<typename T>
    struct Lockcable
    {
        template<typename F>
        void read(const F& func) const
        {
            mtx.lock_shared();
            func(value);
            mtx.unlock_shared();
        }

        template<typename F>
        void write(const F& func)
        {
            mtx.lock();
            func(value);
            mtx.unlock();
        }
    private:
        T value;
        mutable skr::shared_atomic_mutex mtx;
    };

    skr::task::counter_t allCounter;
    Lockcable<sugoi::EntityRegistry> registry;
    Lockcable<skr::stl_vector<sugoi::JobDependencyEntry>> allResources;
    Lockcable<skr::stl_vector<sugoi_storage_t*>> storages;
    skr::ParallelFlatHashMap<sugoi::archetype_t*, skr::stl_vector<JobDependencyEntry>> dependencyEntries;
};
} // namespace sugoi