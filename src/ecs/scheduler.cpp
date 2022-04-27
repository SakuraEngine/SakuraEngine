#include "scheduler.hpp"
#include "SmallVector.h"
#include "archetype.hpp"
#include "arena.hpp"

#include "constants.hpp"
#include "ecs/dual.h"
#include "ftl/task.h"
#include "ftl/task_counter.h"
#include "mask.hpp"
#include "phmap.h"
#include "query.hpp"
#include "storage.hpp"
#include "callback.hpp"
#include "type.hpp"
#include "set.hpp"
#include "utils.hpp"

dual::scheduler_t::scheduler_t()
{
    schedular.Init();
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
    return schedular.GetCurrentFiberIndex() == 0;
}

void dual::scheduler_t::sync_archetype(dual::archetype_t* type)
{
    // TODO: performance optimization
    auto pair = dependencyEntries.find(type);
    if (pair == dependencyEntries.end())
        return;
    auto entries = pair->second.get();
    std::vector<dual_job_t*> deps;
    auto count = type->type.length;
    forloop(i, 0, count)
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
        wait_job(dep);
}

void dual::scheduler_t::sync_entry(dual::archetype_t* type, dual_type_index_t i)
{
    // TODO: performance optimization
    auto pair = dependencyEntries.find(type);
    if (pair == dependencyEntries.end()) return;
    auto entries = pair->second.get();
    std::vector<dual_job_t*> deps;
    for (auto dep : entries[i].owned)
        deps.push_back(dep);
    for (auto p : entries[i].shared)
        deps.push_back(p);
    entries[i].shared.clear();
    entries[i].owned.clear();
    for (auto dep : deps)
        wait_job(dep);
}

void dual::scheduler_t::sync_all()
{
    for (auto job : allJobs)
        wait_job(job);
}

void dual::scheduler_t::sync_storage(const dual_storage_t* storage)
{
    for (auto job : allJobs)
        if (job->type == dual_job_type::ecs)
        {
            auto ecsJob = (dual_ecs_job_t*)job;
            if (ecsJob->query->storage == storage)
                wait_job(job);
        }
}

void dual::scheduler_t::wait_job(const dual_job_t* job, bool pin)
{
    if (std::find(allJobs.begin(), allJobs.end(), job) == allJobs.end())
        return;
    schedular.WaitForCounter((ftl::TaskCounter*)&job->counter, pin);
}

void dual::scheduler_t::clean_jobs()
{
    allJobs.erase(std::remove_if(allJobs.begin(), allJobs.end(), [](dual_job_t* job) {
        bool done = job->counter.Done();
        if (done)
            delete job;
        return done;
    }),
        allJobs.end());
}

namespace dual
{
void update_entry(job_dependency_entry_t& entry, dual_job_t* job, bool readonly, bool atomic, phmap::flat_hash_set<dual_job_t*>& dependencies)
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

dual_ecs_job_t* dual::scheduler_t::schedule_ecs_job(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
    dual_system_init_callback_t init, dual_resource_operation_t* resources)
{
    assert(query->parameters.length < 32);
    llvm_vecsmall::SmallVector<dual_group_t*, 64> groups;
    auto add_group = [&](dual_group_t* group) {
        groups.push_back(group);
    };
    auto& params = query->parameters;
    query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_group));
    auto groupCount = groups.size();
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
        forloop(i, 0, params.length)
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

    phmap::flat_hash_set<dual_job_t*> dependencies;
    phmap::flat_hash_set<std::pair<dual::archetype_t*, dual_type_index_t>> syncedEntry;
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
            std::unique_ptr<job_dependency_entry_t[]> entries(new job_dependency_entry_t[at->type.length]);
            dependencyEntries.insert(std::make_pair(at, std::move(entries)));
        }

        auto entries = (*iter).second.get();
        auto& entry = entries[localType];
        update_entry(entry, job, readonly, atomic, dependencies);
    };

    auto sync_type = [&](dual_type_index_t type, bool readonly, bool atomic) {
        for (auto& pair : query->storage->groups)
        {
            auto group = pair.second;
            auto idx = group->index(type);
            sync_entry(group, idx, readonly, atomic);
        }
    };

    forloop(i, 0, resources->count)
    {
        auto& entry = allResources[e_id(resources->resources[i])];
        auto readonly = resources->readonly[i];
        auto atomic = resources->atomic[i];
        update_entry(entry, job, readonly, atomic, dependencies);
    }

    forloop(i, 0, query->parameters.length)
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

    job->dependencies = new dual_job_t*[dependencies.size()];
    job->dependencyCount = dependencies.size();
    uint32_t dependencyIndex = 0;
    for (auto dependency : dependencies)
        job->dependencies[dependencyIndex++] = dependency;

    auto body = +[](ftl::TaskScheduler*, void* data) {
        auto job = (dual_ecs_job_t*)data;
        forloop(i, 0, job->dependencyCount)
            job->scheduler->wait_job(job->dependencies[i]);
        job->init(job->userdata, job);
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
                job->callback(job->userdata, job, view, job->localTypes, startIndex);
                startIndex += view->count;
            };
            forloop(i, 0, job->groupCount)
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
            std::vector<batch_t> batchs;
            std::vector<task_t> tasks;
            batchs.reserve(job->entityCount / job->batchSize);
            tasks.reserve(batchs.capacity());
            {

                uint32_t batchRemain = job->batchSize;
                EIndex startIndex = 0;
                batch_t currBatch;
                currBatch.startTask = currBatch.endTask = 0;
                forloop(i, 0, job->groupCount)
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
                    job->callback(job->userdata, job, &task->view, &job->localTypes[job->query->parameters.length * task->groupIndex], task->startIndex);
            };
            auto _tasks = new ftl::Task[batchs.size()];
            forloop(i, 0, batchs.size())
                _tasks[i] = { taskBody, &payloads[i] };
            job->scheduler->schedular.AddTasks(batchs.size(), _tasks, ftl::TaskPriority::Normal, &job->counter);
            delete[] _tasks;
        }
    };
    schedular.AddTask({ body, job }, ftl::TaskPriority::High, &job->counter);
    return job;
}

dual_job_t* dual::scheduler_t::schedule_job(uint32_t count, dual_for_callback_t callback, void* u, dual_resource_operation_t* resources)
{
    dual_simple_job_t* job = new dual_simple_job_t(*this);
    job->type = dual_job_type::simple;
    phmap::flat_hash_set<dual_job_t*> dependencies;
    forloop(i, 0, resources->count)
    {
        auto& entry = allResources[e_id(resources->resources[i])];
        auto readonly = resources->readonly[i];
        auto atomic = resources->atomic[i];
        update_entry(entry, job, readonly, atomic, dependencies);
    }

    job->dependencies = new dual_job_t*[dependencies.size()];
    job->dependencyCount = dependencies.size();
    uint32_t dependencyIndex = 0;
    for (auto dependency : dependencies)
        job->dependencies[dependencyIndex++] = dependency;

    struct task_payload_t {
        dual_simple_job_t* job;
        uint32_t index;
    };
    task_payload_t* payloads = (task_payload_t*)::dual_malloc(sizeof(task_payload_t) * count);
    forloop(i, 0, count)
        payloads[i] = { job, i };
    job->payloads = payloads;
    auto taskBody = +[](ftl::TaskScheduler* taskScheduler, void* data) {
        auto payload = (task_payload_t*)data;
        forloop(i, 0, payload->job->dependencyCount)
            payload->job->scheduler->wait_job(payload->job->dependencies[i]);
        payload->job->callback(payload->job->userdata, payload->job, payload->index);
    };
    auto tasks = new ftl::Task[count];
    forloop(i, 0, count)
        tasks[i] = { taskBody, &payloads[i] };

    schedular.AddTasks(count, tasks, ftl::TaskPriority::Normal, &job->counter);
    delete[] tasks;
    return job;
}

dual_job_t::~dual_job_t()
{
    delete dependencies;
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

dual_job_t* dualJ_schedule_ecs(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
    dual_system_init_callback_t init, dual_resource_operation_t* resources)
{
    return dual::scheduler_t::get().schedule_ecs_job(query, batchSize, callback, u, init, resources);
}

dual_job_t* dualJ_schedule_for(uint32_t count, dual_for_callback_t callback, void* u, dual_resource_operation_t* resources)
{
    return dual::scheduler_t::get().schedule_job(count, callback, u, resources);
}

void dualJ_wait_job(dual_job_t* counter, int pin)
{
    dual::scheduler_t::get().wait_job(counter, pin);
}

void dualJ_wait_all()
{
    dual::scheduler_t::get().sync_all();
}

void dualJ_wait_storage(dual_storage_t* storage)
{
    dual::scheduler_t::get().sync_storage(storage);
}

void dualJ_GC()
{
    dual::scheduler_t::get().clean_jobs();
}
}