#pragma once
#include "arena.hpp"

#include "ecs/dual.h"
#include "ftl/task_counter.h"
#include "ftl/task_scheduler.h"
#include "mask.hpp"
#include "archetype.hpp"
#include <bitset>
#include <phmap.h>
#include "entities.hpp"
#include "utils/hashmap.hpp"
#include "EASTL/shared_ptr.h"
#include "EASTL/vector.h"

struct dual_ecs_job_t;
namespace dual
{
struct job_dependency_entry_t {
    eastl::vector<eastl::shared_ptr<ftl::TaskCounter>> owned;
    eastl::vector<eastl::shared_ptr<ftl::TaskCounter>> shared;
};

struct scheduler_t {
    ftl::TaskScheduler scheduler;
    dual::entity_registry_t registry;
    skr::flat_hash_map<dual_storage_t*, std::shared_ptr<ftl::TaskCounter>> ecsCounter;
    ftl::TaskCounter allCounter;
    eastl::vector<dual::job_dependency_entry_t> allResources;
    skr::flat_hash_map<dual::archetype_t*, eastl::vector<job_dependency_entry_t>> dependencyEntries;

    scheduler_t();
    static scheduler_t& get();
    bool is_main_thread(const dual_storage_t* storage);
    dual_entity_t add_resource();
    void remove_resource(dual_entity_t id);
    void sync_archetype(dual::archetype_t* type);
    void sync_entry(dual::archetype_t* type, dual_type_index_t entry);
    void sync_all();
    void sync_storage(const dual_storage_t* storage);
    eastl::shared_ptr<ftl::TaskCounter> schedule_ecs_job(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u, dual_system_init_callback_t init, dual_resource_operation_t* resources);
    eastl::shared_ptr<ftl::TaskCounter> schedule_job(uint32_t count, dual_for_callback_t callback, void* u, dual_resource_operation_t* resources);
};
} // namespace dual

enum class dual_job_type
{
    simple,
    ecs
};

struct dual_job_t {
    dual::scheduler_t* scheduler;
    dual_job_type type;
    eastl::shared_ptr<ftl::TaskCounter> counter;
    eastl::vector<eastl::shared_ptr<ftl::TaskCounter>> dependencies;
    int dependencyCount;
    dual_job_t(dual::scheduler_t& scheduler)
        : scheduler(&scheduler)
        , counter(eastl::make_shared<ftl::TaskCounter>(&scheduler.scheduler))
    {
    }
    virtual ~dual_job_t();
};

struct dual_simple_job_t : dual_job_t {
    using dual_job_t::dual_job_t;
    dual_for_callback_t callback;
    void* userdata;
    void* payloads;
    ~dual_simple_job_t();
};

struct dual_ecs_job_t : dual_job_t {
    using dual_job_t::dual_job_t;
    dual_group_t** groups;
    uint32_t groupCount;
    dual_type_index_t* localTypes;
    std::bitset<32>* readonly;
    std::bitset<32>* atomic;
    std::bitset<32>* randomAccess;
    bool hasRandomWrite;
    EIndex entityCount;
    dual_resource_operation_t resources;
    dual_query_t* query;
    dual_system_callback_t callback;
    dual_system_init_callback_t init;
    EIndex batchSize;
    void* userdata;
    void* payloads;
    ~dual_ecs_job_t();
};