#include "scheduler.hpp"
#include "ecs/SmallVector.h"
#include "archetype.hpp"
#include "arena.hpp"

#include "ecs/constants.hpp"
#include "ecs/dual.h"
#include "ftl/task.h"
#include "ftl/task_counter.h"
#include "mask.hpp"
#include "phmap.h"
#include "query.hpp"
#include "storage.hpp"
#include "ecs/callback.hpp"
#include "type.hpp"
#include "set.hpp"

dual::scheduler_t::scheduler_t()
    : allCounter(&scheduler)
{
    ftl::TaskSchedulerInitOptions option = {};
    option.Behavior = ftl::EmptyQueueBehavior::Sleep;
    scheduler.Init(option);
}

dual_entity_t dual::scheduler_t::add_resource()
{
    dual_entity_t result;
    registry.new_entities(&result, 1);
    return result;
}

void dual::scheduler_t::remove_resource(dual_entity_t id)
{
    registry.free_entities(&id, 1);
}

bool dual::scheduler_t::is_main_thread(const dual_storage_t* storage)
{
    // TODO: support multiple main thread (one main thread per storage)
    return scheduler.GetCurrentThreadIndex() == 0;
}

void dual::scheduler_t::sync_archetype(dual::archetype_t* type)
{
    // TODO: performance optimization
    auto pair = dependencyEntries.find(type);
    if (pair == dependencyEntries.end())
        return;
    auto entries = pair->second.data();
    eastl::vector<eastl::shared_ptr<ftl::TaskCounter>> deps;
    auto count = type->type.length;
    forloop (i, 0, count)
    {
        if (type_index_t(type->type.data[i]).is_tag())
            break;
        for (auto dep : entries[i].owned)
            deps.push_back(dep);
        for (auto p : entries[i].shared)
            deps.push_back(p);
        entries[i].shared.clear();
        entries[i].owned.clear();
    }
    dependencyEntries.erase(pair);
    for (auto dep : deps)
        scheduler.WaitForCounter(dep.get());
}

void dual::scheduler_t::sync_entry(dual::archetype_t* type, dual_type_index_t i)
{
    // TODO: performance optimization
    auto pair = dependencyEntries.find(type);
    if (pair == dependencyEntries.end()) return;
    auto entries = pair->second.data();
    eastl::vector<eastl::shared_ptr<ftl::TaskCounter>> deps;
    for (auto dep : entries[i].owned)
        deps.push_back(dep);
    for (auto p : entries[i].shared)
        deps.push_back(p);
    entries[i].shared.clear();
    entries[i].owned.clear();
    for (auto dep : deps)
        scheduler.WaitForCounter(dep.get());
}

void dual::scheduler_t::sync_all()
{
    scheduler.WaitForCounter(&allCounter);
}

void dual::scheduler_t::sync_storage(const dual_storage_t* storage)
{
    auto iter = ecsCounter.find(storage);
    if (iter == ecsCounter.end())
        return;
    scheduler.WaitForCounter(iter->second.get());
}

namespace dual
{
struct hash_shared_ptr {
    template <class T>
    size_t operator()(const eastl::shared_ptr<T>& value) const
    {
        return std::hash<void*>{}(value.get());
    }
};
using DependencySet = skr::flat_hash_set<eastl::shared_ptr<ftl::TaskCounter>, hash_shared_ptr>;
void update_entry(job_dependency_entry_t& entry, eastl::shared_ptr<ftl::TaskCounter> job, bool readonly, bool atomic, DependencySet& dependencies)
{
    if (readonly)
    {
        for (auto& dp : entry.owned)
            dependencies.insert(dp);
        entry.shared.push_back(job);
    }
    else
    {
        for (auto& dp : entry.shared)
            dependencies.insert(dp);
        if (atomic)
        {
            entry.owned.push_back(job);
        }
        else
        {
            for (auto& dp : entry.owned)
                dependencies.insert(dp);
            entry.shared.clear();
            entry.owned.clear();
            entry.owned.push_back(job);
        }
    }
}
} // namespace dual

eastl::shared_ptr<ftl::TaskCounter> dual::scheduler_t::schedule_ecs_job(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_init_callback_t init, dual_resource_operation_t* resources)
{
    assert(query->parameters.length < 32);
    llvm_vecsmall::SmallVector<dual_group_t*, 64> groups;
    auto add_group = [&](dual_group_t* group) {
        groups.push_back(group);
    };
    auto& params = query->parameters;
    query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_group));
    auto groupCount = (uint32_t)groups.size();
    size_t arenaSize = 0;
    arenaSize += sizeof(dual_job_t);
    arenaSize += resources->count * (sizeof(dual_entity_t) + sizeof(int)); // job.resources
    arenaSize += groupCount * sizeof(dual_type_index_t) * params.length;   // job.localTypes
    arenaSize += groupCount * sizeof(std::bitset<32>);                     // job.readonly
    arenaSize += groupCount * sizeof(std::bitset<32>);                     // job.randomAccess
    fixed_arena_t arena{ arenaSize };                                      // todo: pool?
    dual_ecs_job_t* job = new (arena.allocate<dual_ecs_job_t>()) dual_ecs_job_t(*this);
    job->type = dual_job_type::ecs;
    job->groups = arena.allocate<dual_group_t*>(groupCount);
    job->groupCount = groupCount;
    job->localTypes = arena.allocate<dual_type_index_t>(groupCount * params.length);
    job->readonly = arena.allocate<std::bitset<32>>(groupCount);
    job->atomic = arena.allocate<std::bitset<32>>(groupCount);
    job->randomAccess = arena.allocate<std::bitset<32>>(groupCount);
    job->hasRandomWrite = false;
    job->entityCount = 0;
    job->callback = callback;
    job->userdata = u;
    job->batchSize = batchSize;
    job->init = init;
    arena.forget();
    std::memcpy(job->groups, groups.data(), groupCount * sizeof(dual_group_t*));
    int groupIndex = 0;
    for (auto group : groups)
    {
        job->entityCount += group->size;
        forloop (i, 0, params.length)
        {
            auto idx = group->index(params.types[i]);
            job->localTypes[groupIndex * params.length + i] = idx;
            auto& op = params.accesses[i];
            job->readonly[groupIndex].set(idx, op.readonly);
            job->randomAccess[groupIndex].set(idx, op.randomAccess == DOS_GLOBAL);
            job->atomic[groupIndex].set(idx, op.atomic);
            job->hasRandomWrite |= op.randomAccess == DOS_GLOBAL;
        }
        ++groupIndex;
    }

    DependencySet dependencies;
    skr::flat_hash_set<std::pair<dual::archetype_t*, dual_type_index_t>> syncedEntry;
    auto sync_entry = [&](const dual_group_t* group, dual_type_index_t localType, bool readonly, bool atomic) {
        if (localType == kInvalidTypeIndex)
            return;
        auto at = group->archetype;
        auto pair = std::make_pair(at, localType);
        if (syncedEntry.find(pair) != syncedEntry.end())
            return;
        syncedEntry.insert(pair);
        auto iter = dependencyEntries.find(at);
        if (iter == dependencyEntries.end())
        {
            eastl::vector<job_dependency_entry_t> entries(at->type.length);
            dependencyEntries.insert(std::make_pair(at, std::move(entries)));
        }

        auto entries = (*iter).second.data();
        auto& entry = entries[localType];
        update_entry(entry, job->counter, readonly, atomic, dependencies);
    };

    auto sync_type = [&](dual_type_index_t type, bool readonly, bool atomic) {
        for (auto& pair : query->storage->groups)
        {
            auto group = pair.second;
            auto idx = group->index(type);
            sync_entry(group, idx, readonly, atomic);
        }
    };

    if (resources)
    {
        forloop (i, 0, resources->count)
        {
            auto& entry = allResources[e_id(resources->resources[i])];
            auto readonly = resources->readonly[i];
            auto atomic = resources->atomic[i];
            update_entry(entry, job->counter, readonly, atomic, dependencies);
        }
    }

    forloop (i, 0, query->parameters.length)
    {
        if (type_index_t(params.types[i]).is_tag())
            continue;
        if (params.accesses[i].randomAccess == DOS_GLOBAL)
        {
            sync_type(params.types[i], params.accesses[i].readonly, params.accesses[i].atomic);
        }
        else
        {
            groupIndex = 0;
            for (auto group : groups)
            {
                auto localType = job->localTypes[groupIndex * params.length + i];
                if (localType == kInvalidTypeIndex)
                {
                    auto g = group->get_owner(params.types[i]);
                    if (g)
                    {
                        sync_entry(g, g->index(params.types[i]), params.accesses[i].readonly, params.accesses[i].atomic);
                    }
                }
                else
                {
                    sync_entry(group, localType, params.accesses[i].readonly, params.accesses[i].atomic);
                }
                ++groupIndex;
            }
        }
    }

    job->dependencies.resize(dependencies.size());
    job->dependencyCount = (int)dependencies.size();
    uint32_t dependencyIndex = 0;
    for (auto dependency : dependencies)
        job->dependencies[dependencyIndex++] = dependency;

    auto body = +[](ftl::TaskScheduler*, void* data) {
        auto job = (dual_ecs_job_t*)data;
        forloop (i, 0, job->dependencyCount)
            job->scheduler->scheduler.WaitForCounter(job->dependencies[i].get());
        if (job->init)
            job->init(job->userdata, job->entityCount);
        auto query = job->query;
        fixed_stack_scope_t _(localStack);
        dual_meta_filter_t validatedMeta;
        {
            auto& meta = query->meta;
            auto data = (char*)localStack.allocate(data_size(meta));
            validatedMeta = clone(meta, data);
            query->storage->validate(validatedMeta.all_meta);
            query->storage->validate(validatedMeta.any_meta);
            query->storage->validate(validatedMeta.none_meta);
        }
        if (job->hasRandomWrite)
        {
            uint32_t startIndex = 0;
            auto processView = [&](dual_chunk_view_t* view) {
                job->callback(job->userdata, job->query->storage, view, job->localTypes, startIndex);
                startIndex += view->count;
            };
            forloop (i, 0, job->groupCount)
            {
                auto group = job->groups[i];
                query->storage->query(group, query->filter, validatedMeta, DUAL_LAMBDA(processView));
            }
        }
        else
        {
            struct task_t {
                uint32_t groupIndex;
                uint32_t startIndex;
                dual_chunk_view_t view;
            };
            struct batch_t {
                intptr_t startTask;
                intptr_t endTask;
            };
            eastl::vector<batch_t> batchs;
            eastl::vector<task_t> tasks;
            batchs.reserve(job->entityCount / job->batchSize);
            tasks.reserve(batchs.capacity());
            {

                uint32_t batchRemain = job->batchSize;
                EIndex startIndex = 0;
                batch_t currBatch;
                currBatch.startTask = currBatch.endTask = 0;
                forloop (i, 0, job->groupCount)
                {
                    auto scheduleView = [&](dual_chunk_view_t* view) {
                        uint32_t allocated = 0;
                        while (allocated != view->count)
                        {
                            uint32_t subViewCount = std::min(view->count - allocated, batchRemain);
                            task_t newTask;
                            newTask.groupIndex = i;
                            newTask.startIndex = startIndex;
                            newTask.view = dual_chunk_view_t{ view->chunk, view->start + allocated, subViewCount };
                            allocated += subViewCount;
                            startIndex += subViewCount;
                            batchRemain -= subViewCount;
                            tasks.push_back(newTask);
                            if (batchRemain == 0) // batch filled
                            {
                                currBatch.endTask = tasks.size();
                                batchs.push_back(currBatch);
                                currBatch.startTask = currBatch.endTask; // new batch
                                batchRemain = job->batchSize;
                            }
                        }
                    };
                    auto group = job->groups[i];
                    query->storage->query(group, query->filter, validatedMeta, DUAL_LAMBDA(scheduleView));
                };
                if (currBatch.endTask != tasks.size())
                {
                    currBatch.endTask = tasks.size();
                    batchs.push_back(currBatch);
                }
            }
            struct task_payload_t {
                batch_t batch;
                dual_ecs_job_t* job;
            };
            task_payload_t* payloads = (task_payload_t*)::dual_malloc(sizeof(task_payload_t) * batchs.size());
            uint32_t payloadIndex = 0;
            for (auto& batch : batchs)
            {
                batch.startTask = (intptr_t)&tasks[batch.startTask];
                batch.endTask = (intptr_t)&tasks[batch.endTask];
                payloads[payloadIndex++] = { batch, job };
            }
            job->payloads = payloads;

            auto taskBody = +[](ftl::TaskScheduler* taskScheduler, void* data) {
                task_payload_t* payload = (task_payload_t*)data;
                auto job = payload->job;
                for (auto task = (task_t*)payload->batch.startTask; task != (task_t*)payload->batch.endTask; ++task)
                    job->callback(job->userdata, job->query->storage, &task->view, &job->localTypes[job->query->parameters.length * task->groupIndex], task->startIndex);
            };
            auto _tasks = (ftl::Task*)dual_malloc(sizeof(ftl::Task) * batchs.size());

            auto TearDown = +[](void* data) {
                task_payload_t* payload = (task_payload_t*)data;
                auto job = payload->job;
                if (job->counter->Done())
                {
                    job->~dual_ecs_job_t();
                    dual_free(job);
                }
            };
            forloop (i, 0, batchs.size())
                _tasks[i] = { taskBody, &payloads[i], TearDown };
            job->scheduler->scheduler.AddTasks((unsigned int)batchs.size(), _tasks, ftl::TaskPriority::Normal, job->counter.get());
            dual_free(_tasks);
        }
    };
    auto TearDown = +[](void* data) {
        dual_ecs_job_t* job = (dual_ecs_job_t*)data;
        if (job->counter->Done())
        {
            job->~dual_ecs_job_t();
            dual_free(job);
        }
    };
    scheduler.AddTask({ body, job, TearDown }, ftl::TaskPriority::High, job->counter.get());
    return job->counter;
}

eastl::shared_ptr<ftl::TaskCounter> dual::scheduler_t::schedule_job(uint32_t count, dual_for_callback_t callback, void* u, dual_resource_operation_t* resources)
{
    dual_simple_job_t* job = (dual_simple_job_t*)dual_malloc(sizeof(dual_simple_job_t));
    job = new (job) dual_simple_job_t(*this);
    job->type = dual_job_type::simple;
    DependencySet dependencies;
    if (resources)
    {
        forloop (i, 0, resources->count)
        {
            auto& entry = allResources[e_id(resources->resources[i])];
            auto readonly = resources->readonly[i];
            auto atomic = resources->atomic[i];
            update_entry(entry, job->counter, readonly, atomic, dependencies);
        }
    }

    job->dependencies.resize(dependencies.size());
    job->dependencyCount = (int)dependencies.size();
    uint32_t dependencyIndex = 0;
    for (auto dependency : dependencies)
        job->dependencies[dependencyIndex++] = dependency;

    struct task_payload_t {
        dual_simple_job_t* job;
        uint32_t index;
    };
    task_payload_t* payloads = (task_payload_t*)::dual_malloc(sizeof(task_payload_t) * count);
    forloop (i, 0, count)
        payloads[i] = { job, i };
    job->payloads = payloads;
    auto taskBody = +[](ftl::TaskScheduler* taskScheduler, void* data) {
        auto payload = (task_payload_t*)data;
        auto job = payload->job;
        forloop (i, 0, job->dependencyCount)
            job->scheduler->scheduler.WaitForCounter(job->dependencies[i].get());
        job->callback(job->userdata, payload->index);
    };
    auto TearDown = +[](void* data) {
        task_payload_t* payload = (task_payload_t*)data;
        auto job = payload->job;
        if (job->counter->Done())
        {
            job->~dual_simple_job_t();
            dual_free(job);
        }
    };
    auto tasks = (ftl::Task*)dual_malloc(sizeof(ftl::Task) * count);
    forloop (i, 0, count)
        tasks[i] = { taskBody, &payloads[i], TearDown };

    scheduler.AddTasks(count, tasks, ftl::TaskPriority::Normal, job->counter.get());
    dual_free(tasks);
    return job->counter;
}

dual_job_t::~dual_job_t()
{
}

dual_simple_job_t::~dual_simple_job_t()
{
    ::dual_free(payloads);
}

dual_ecs_job_t::~dual_ecs_job_t()
{
    ::dual_free(payloads);
}

extern "C" {
dual_entity_t dualJ_add_resource()
{
    return dual::scheduler_t::get().add_resource();
}

void dualJ_remove_resource(dual_entity_t id)
{
    dual::scheduler_t::get().remove_resource(id);
}

struct dual_counter_t {
    eastl::shared_ptr<ftl::TaskCounter> counter;
};

void dualJ_schedule_ecs(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_init_callback_t init, dual_resource_operation_t* resources, dual_counter_t** counter)
{
    if (counter)
    {
        *counter = SkrNew<dual_counter_t>(dual::scheduler_t::get().schedule_ecs_job(query, batchSize, callback, u, init, resources));
    }
    else
    {
        dual::scheduler_t::get().schedule_ecs_job(query, batchSize, callback, u, init, resources);
    }
}

void dualJ_schedule_for(uint32_t count, dual_for_callback_t callback, void* u, dual_resource_operation_t* resources, dual_counter_t** counter)
{
    auto result = dual::scheduler_t::get().schedule_job(count, callback, u, resources);
}

void dualJ_wait_counter(dual_counter_t* counter, int pin)
{
    dual::scheduler_t::get().scheduler.WaitForCounter(counter->counter.get(), pin);
}

void dualJ_release_counter(dual_counter_t* counter)
{
    SkrDelete(counter);
}

void dualJ_wait_all()
{
    dual::scheduler_t::get().sync_all();
}

void dualJ_wait_storage(dual_storage_t* storage)
{
    dual::scheduler_t::get().sync_storage(storage);
}
}

ftl::TaskScheduler* dualX_get_scheduler()
{
    return &dual::scheduler_t::get().scheduler;
}