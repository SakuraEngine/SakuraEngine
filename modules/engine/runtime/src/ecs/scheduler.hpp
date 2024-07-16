#pragma once
#include "SkrOS/thread.h"
#include "SkrTask/fib_task.hpp"
#include "SkrContainers/hashmap.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/entity_registry.hpp"

#include "./archetype.hpp"

namespace sugoi
{
struct JobDependencyEntry {
    skr::stl_vector<skr::task::weak_event_t> owned;
    skr::stl_vector<skr::task::weak_event_t> shared;
};

struct scheduler_t {
    sugoi::EntityRegistry registry;
    skr::task::counter_t allCounter;
    skr::stl_vector<sugoi::JobDependencyEntry> allResources;
    skr::FlatHashMap<sugoi::archetype_t*, skr::stl_vector<JobDependencyEntry>> dependencyEntries;
    SMutexObject entryMutex;
    SMutexObject resourceMutex;
    skr::stl_vector<sugoi_storage_t*> storages;
    SMutexObject storageMutex;

    scheduler_t();
    ~scheduler_t();

    static scheduler_t& get();
    bool is_main_thread(const sugoi_storage_t* storage);
    void set_main_thread(const sugoi_storage_t* storage);

    void add_storage(sugoi_storage_t* storage);
    void remove_storage(const sugoi_storage_t* storage);
    
    sugoi_entity_t add_resource();
    void remove_resource(sugoi_entity_t id);
    void gc_entries();

    bool sync_archetype(sugoi::archetype_t* type);
    bool sync_entry(sugoi::archetype_t* type, sugoi_type_index_t entry, bool readonly);
    bool sync_query(sugoi_query_t* query);
    void sync_all_jobs();
    void sync_storage(const sugoi_storage_t* storage);
    skr::stl_vector<skr::task::event_t> sync_resources(const skr::task::event_t& counter, sugoi_resource_operation_t* resources);

    skr::task::event_t schedule_ecs_job(sugoi_query_t* query, EIndex batchSize, sugoi_system_callback_t callback, void* u, sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources);
    skr::task::event_t schedule_job(sugoi_query_t* query, sugoi_schedule_callback_t callback, void* u, sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources);
    skr::stl_vector<skr::task::weak_event_t> update_dependencies(sugoi_query_t* query, const skr::task::event_t& counter, sugoi_resource_operation_t* resources);
};
} // namespace sugoi

/*
enum class sugoi_job_type
{
    simple,
    ecs
};

struct sugoi_job_t {
    sugoi::scheduler_t* scheduler;
    sugoi_job_type type;
    skr::task::event_t counter;
    skr::stl_vector<skr::task::event_t> dependencies;
    int dependencyCount;
    sugoi_job_t(sugoi::scheduler_t& scheduler);
    virtual ~sugoi_job_t();
};

struct sugoi_ecs_job_t : sugoi_job_t {
    using sugoi_job_t::sugoi_job_t;
    sugoi_group_t** groups;
    uint32_t groupCount;
    sugoi_type_index_t* localTypes;
    std::bitset<32>* readonly;
    std::bitset<32>* atomic;
    std::bitset<32>* randomAccess;
    bool hasRandomWrite;
    EIndex entityCount;
    sugoi_resource_operation_t resources;
    const sugoi_query_t* query;
    sugoi_system_callback_t callback;
    sugoi_system_lifetime_callback_t init;
    sugoi_system_lifetime_callback_t teardown;
    EIndex batchSize;
    void* userdata;
    void* payloads;
    void* tasks;
    ~sugoi_ecs_job_t();
};
*/