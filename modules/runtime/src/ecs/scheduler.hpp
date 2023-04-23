#pragma once
#include "ecs/dual.h"
#include "task/task.hpp"
#include "archetype.hpp"
#include "ecs/entities.hpp"
#include "platform/thread.h"
#include "containers/hashmap.hpp"
#include "EASTL/vector.h"

namespace dual
{
struct job_dependency_entry_t {
    eastl::vector<skr::task::weak_event_t> owned;
    eastl::vector<skr::task::weak_event_t> shared;
};

struct scheduler_t {
    dual::entity_registry_t registry;
    skr::task::counter_t allCounter;
    eastl::vector<dual::job_dependency_entry_t> allResources;
    skr::flat_hash_map<dual::archetype_t*, eastl::vector<job_dependency_entry_t>> dependencyEntries;
    SMutexObject entryMutex;
    SMutexObject resourceMutex;
    eastl::vector<dual_storage_t*> storages;
    SMutexObject storageMutex;

    scheduler_t();
    ~scheduler_t();
    static scheduler_t& get();
    bool is_main_thread(const dual_storage_t* storage);
    void set_main_thread(const dual_storage_t* storage);
    void add_storage(dual_storage_t* storage);
    void remove_storage(const dual_storage_t* storage);
    dual_entity_t add_resource();
    void remove_resource(dual_entity_t id);
    bool sync_archetype(dual::archetype_t* type);
    bool sync_entry(dual::archetype_t* type, dual_type_index_t entry, bool readonly);
    bool sync_query(dual_query_t* query);
    void sync_all();
    void gc_entries();
    void sync_storage(const dual_storage_t* storage);
    skr::task::event_t schedule_ecs_job(dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u, dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources);
    eastl::vector<skr::task::weak_event_t> update_dependencies(dual_query_t* query, const skr::task::event_t& counter, dual_resource_operation_t* resources);
    skr::task::event_t schedule_job(dual_query_t* query, dual_schedule_callback_t callback, void* u, dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources);
    eastl::vector<skr::task::event_t> sync_resources(const skr::task::event_t& counter, dual_resource_operation_t* resources);
};
} // namespace dual

/*
enum class dual_job_type
{
    simple,
    ecs
};

struct dual_job_t {
    dual::scheduler_t* scheduler;
    dual_job_type type;
    skr::task::event_t counter;
    eastl::vector<skr::task::event_t> dependencies;
    int dependencyCount;
    dual_job_t(dual::scheduler_t& scheduler);
    virtual ~dual_job_t();
};

struct dual_ecs_job_t : dual_job_t {
    using dual_job_t::dual_job_t;
    dual_group_t** groups;
    uint32_t groupCount;
    dual_type_index_t* localTypes;
    eastl::bitset<32>* readonly;
    eastl::bitset<32>* atomic;
    eastl::bitset<32>* randomAccess;
    bool hasRandomWrite;
    EIndex entityCount;
    dual_resource_operation_t resources;
    const dual_query_t* query;
    dual_system_callback_t callback;
    dual_system_lifetime_callback_t init;
    dual_system_lifetime_callback_t teardown;
    EIndex batchSize;
    void* userdata;
    void* payloads;
    void* tasks;
    ~dual_ecs_job_t();
};
*/