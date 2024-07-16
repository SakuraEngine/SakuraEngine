#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrContainers/stl_vector.hpp"

namespace sugoi
{
struct archetype_t;

struct SKR_RUNTIME_API JobScheduler
{
public:
    struct Impl;
    JobScheduler(Impl& impl);
    static JobScheduler& Get();

    bool is_main_thread(const sugoi_storage_t* storage);
    void set_main_thread(const sugoi_storage_t* storage);

    void add_storage(sugoi_storage_t* storage);
    void remove_storage(const sugoi_storage_t* storage);
    
    sugoi_entity_t add_resource();
    void remove_resource(sugoi_entity_t id);

    void collect_garbage();

    bool sync_archetype(sugoi::archetype_t* type);
    bool sync_entry(sugoi::archetype_t* type, sugoi_type_index_t entry, bool readonly);
    bool sync_query(sugoi_query_t* query);
    void sync_all_jobs();
    void sync_storage(const sugoi_storage_t* storage);
    skr::stl_vector<skr::task::event_t> sync_resources(const skr::task::event_t& counter, sugoi_resource_operation_t* resources);

    skr::task::event_t schedule_ecs_job(sugoi_query_t* query, EIndex batchSize, sugoi_system_callback_t callback, void* u, sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources = nullptr);
    skr::task::event_t schedule_job(sugoi_query_t* query, sugoi_schedule_callback_t callback, void* u, sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources = nullptr);
private:
    skr::stl_vector<skr::task::weak_event_t> updateDependencies(sugoi_query_t* query, const skr::task::event_t& counter, sugoi_resource_operation_t* resources);
    friend struct ::sugoi_context_t;
    Impl& job_impl;
};
}