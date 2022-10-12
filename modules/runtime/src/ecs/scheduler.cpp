#include "scheduler.hpp"
#include "ecs/SmallVector.h"
#include "archetype.hpp"
#include "arena.hpp"

#include "ecs/constants.hpp"
#include "ecs/dual.h"
#include "ftl/task.h"
#include "ftl/task_counter.h"
#include "ftl/task_scheduler.h"
#include "mask.hpp"
#include "phmap.h"
#include "query.hpp"
#include "storage.hpp"
#include "ecs/callback.hpp"
#include "type.hpp"
#include "set.hpp"

dual::scheduler_t::scheduler_t()
{
}

void dual::scheduler_t::initialize(ftl::TaskScheduler* inScheduler)
{
    scheduler = inScheduler;
    allCounter = std::make_shared<ftl::TaskCounter>(scheduler);
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
    SKR_ASSERT(storage->scheduler == this);
    auto mainFiber = storage->mainFiber != nullptr ? storage->mainFiber : scheduler->GetMainFiber();
    return scheduler->GetCurrentFiber() == mainFiber;
}

void dual::scheduler_t::set_main_thread(const dual_storage_t* storage)
{
    SKR_ASSERT(storage->scheduler == this);
    sync_storage(storage);
    storage->mainFiber = scheduler->GetCurrentFiber();
}

void dual::scheduler_t::sync_archetype(dual::archetype_t* type)
{
    SKR_ASSERT(is_main_thread(type->storage));
    eastl::vector<std::shared_ptr<ftl::TaskCounter>> deps;
    {
        // TODO: performance optimization
        SMutexLock entryLock(entryMutex.mMutex);
        auto pair = dependencyEntries.find(type);
        if (pair == dependencyEntries.end())
        {
            return;
        }
        auto entries = pair->second.data();
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
    }
    for (auto dep : deps)
        scheduler->WaitForCounter(dep.get(), true);
}

void dual::scheduler_t::sync_entry(dual::archetype_t* type, dual_type_index_t i)
{
    SKR_ASSERT(is_main_thread(type->storage));
    // TODO: performance optimization
    eastl::vector<std::shared_ptr<ftl::TaskCounter>> deps;
    {
        SMutexLock entryLock(entryMutex.mMutex);
        auto pair = dependencyEntries.find(type);
        if (pair == dependencyEntries.end()) 
        {
            return;
        };
        auto entries = pair->second.data();
        for (auto dep : entries[i].owned)
            deps.push_back(dep);
        for (auto p : entries[i].shared)
            deps.push_back(p);
        entries[i].shared.clear();
        entries[i].owned.clear();
    }
    for (auto dep : deps)
        scheduler->WaitForCounter(dep.get(), true);
}

void dual::scheduler_t::sync_all()
{
    SKR_ASSERT(scheduler->GetCurrentThreadIndex() == 0);
    scheduler->WaitForCounter(allCounter.get(), true);
}

void dual::scheduler_t::sync_storage(const dual_storage_t* storage)
{
    if (!storage->counter)
        return;
    scheduler->WaitForCounter(storage->counter.get(), true);
    storage->counter.reset();
    storage->scheduler = nullptr;
    storage->mainFiber = nullptr;
}

namespace dual
{
struct hash_shared_ptr {
    template <class T>
    size_t operator()(const std::shared_ptr<T>& value) const
    {
        return std::hash<void*>{}(value.get());
    }
};
using DependencySet = skr::flat_hash_set<std::shared_ptr<ftl::TaskCounter>, hash_shared_ptr>;
void update_entry(job_dependency_entry_t& entry, std::shared_ptr<ftl::TaskCounter> job, bool readonly, bool atomic, DependencySet& dependencies)
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

std::shared_ptr<ftl::TaskCounter> dual::scheduler_t::schedule_ecs_job(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources)
{
    ZoneScopedN("SchedualECSJob");

    if (query->storage->scheduler == nullptr)
    {
        query->storage->scheduler = this;
        set_main_thread(query->storage);
    }
    SKR_ASSERT(is_main_thread(query->storage));
    SKR_ASSERT(query->parameters.length < 32);
    llvm_vecsmall::SmallVector<dual_group_t*, 64> groups;
    auto add_group = [&](dual_group_t* group) {
        groups.push_back(group);
    };
    auto& params = query->parameters;
    query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_group));
    auto groupCount = (uint32_t)groups.size();
    size_t arenaSize = 0;
    arenaSize += sizeof(dual_ecs_job_t);
    if(resources)
        arenaSize += resources->count * (sizeof(dual_entity_t) + sizeof(int)); // job.resources
    arenaSize += groupCount * sizeof(dual_group_t*);   // job.groups
    arenaSize += groupCount * sizeof(dual_type_index_t) * params.length;   // job.localTypes
    arenaSize += groupCount * sizeof(std::bitset<32>);                     // job.readonly
    arenaSize += groupCount * sizeof(std::bitset<32>);                     // job.atomic
    arenaSize += groupCount * sizeof(std::bitset<32>);                     // job.randomAccess
    fixed_arena_t arena{ arenaSize * 2 };                                      // todo: pool?
    dual_ecs_job_t* job = new (arena.allocate<dual_ecs_job_t>()) dual_ecs_job_t(*this);
    job->type = dual_job_type::ecs;
    job->query = query;
    {
        ZoneScopedN("AllocateGroupData");
        job->groups = arena.allocate<dual_group_t*>(groupCount);
        job->groupCount = groupCount;
        job->localTypes = arena.allocate<dual_type_index_t>(groupCount * params.length);
        job->readonly = arena.allocate<std::bitset<32>>(groupCount);
        job->atomic = arena.allocate<std::bitset<32>>(groupCount);
        job->randomAccess = arena.allocate<std::bitset<32>>(groupCount);
    }
    job->hasRandomWrite = false;
    job->entityCount = 0;
    job->callback = callback;
    job->userdata = u;
    job->batchSize = batchSize;
    job->init = init;
    job->teardown = teardown;
    job->scheduler = this;
    job->tasks = nullptr;
    job->payloads = nullptr;
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
        {
            SMutexLock entryLock(entryMutex.mMutex);
            auto iter = dependencyEntries.find(at);
            if (iter == dependencyEntries.end())
            {
                eastl::vector<job_dependency_entry_t> entries(at->type.length);
                iter = dependencyEntries.insert(std::make_pair(at, std::move(entries))).first;
            }

            auto entries = (*iter).second.data();
            auto& entry = entries[localType];
            update_entry(entry, job->counter, readonly, atomic, dependencies);
        }
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
        ZoneScopedN("UpdateResourceEntries");

        SMutexLock resourceLock(resourceMutex.mMutex);
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

    {
        ZoneScopedN("AllocateDependencyData");
        job->dependencies.resize(dependencies.size());
        job->dependencyCount = (int)dependencies.size();
        uint32_t dependencyIndex = 0;
        for (auto dependency : dependencies)
            job->dependencies[dependencyIndex++] = dependency;
    }

    auto body = +[](ftl::TaskScheduler*, void* data) {
        ZoneScopedN("ECSJobBody");

        auto job = (dual_ecs_job_t*)data;
        {
            ZoneScopedN("JobWaitDependencies");
            forloop (i, 0, job->dependencyCount)
                job->scheduler->scheduler->WaitForCounter(job->dependencies[i].get());
        }
        {
            ZoneScopedN("JobInitialize");
            if (job->init)
                job->init(job->userdata, job->entityCount);
        }
        auto query = job->query;
        fixed_stack_scope_t _(localStack);
        dual_meta_filter_t validatedMeta;
        {
            ZoneScopedN("JobValidateMeta");

            auto& meta = query->meta;
            auto data = (char*)localStack.allocate(data_size(meta));
            validatedMeta = clone(meta, data);
            query->storage->validate(validatedMeta.all_meta);
            query->storage->validate(validatedMeta.any_meta);
            query->storage->validate(validatedMeta.none_meta);
        }
        if (job->hasRandomWrite || job->batchSize == 0)
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

            {
                ZoneScopedN("JobTearDown0");
                if(job->teardown)
                    job->teardown(job->userdata, job->entityCount);
            }
            job->scheduler->allCounter->Decrement();
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
            {
                ZoneScopedN("JobPrepareBatch");

                batchs.reserve(job->entityCount / job->batchSize);
                tasks.reserve(batchs.capacity());
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
                batch.startTask = (intptr_t)(tasks.data() + batch.startTask);
                batch.endTask = (intptr_t)(tasks.data() + batch.endTask);
                payloads[payloadIndex++] = { batch, job };
            }
            job->tasks = tasks.data();
            tasks.reset_lose_memory();
            job->payloads = payloads;

            auto taskBody = +[](ftl::TaskScheduler* taskScheduler, void* data) {
                task_payload_t* payload = (task_payload_t*)data;
                auto job = payload->job;
                for (auto task = (task_t*)payload->batch.startTask; task != (task_t*)payload->batch.endTask; ++task)
                    job->callback(job->userdata, job->query->storage, &task->view, &job->localTypes[job->query->parameters.length * task->groupIndex], task->startIndex);
                job->scheduler->allCounter->Decrement();
                job->query->storage->counter->Decrement();
            };
            eastl::vector<ftl::Task> _tasks;
            _tasks.resize(batchs.size());

            auto TearDown = ftl::PostTask([job] {
                ZoneScopedN("JobTearDown1");
                if(job->teardown)
                    job->teardown(job->userdata, job->entityCount);
                    
                job->~dual_ecs_job_t();
                sakura_free(job);
            });
            forloop (i, 0, batchs.size())
                _tasks[i] = { taskBody, &payloads[i], TearDown };
            job->scheduler->allCounter->Add(static_cast<const uint32_t>(batchs.size()));
            job->query->storage->counter->Add(static_cast<const uint32_t>(batchs.size()));
            job->scheduler->scheduler->AddTasks((unsigned int)batchs.size(), _tasks.data(), ftl::TaskPriority::Normal, job->counter);
            job->scheduler->allCounter->Decrement();
        }
    };
    {
        ZoneScopedN("AllocateCounter");
        allCounter->Add(1);
        if (!query->storage->counter)
            query->storage->counter = eastl::make_shared<ftl::TaskCounter>(scheduler);
        query->storage->counter->Add(1);
    }
    auto counter = job->counter;
    scheduler->AddTask({ body, job }, ftl::TaskPriority::High, job->counter);
    return counter;
}

eastl::vector<std::shared_ptr<ftl::TaskCounter>> dual::scheduler_t::schedule_custom_job(const dual_query_t* query, const std::shared_ptr<ftl::TaskCounter>& counter, dual_resource_operation_t* resources)
{
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
        {
            SMutexLock entryLock(entryMutex.mMutex);
            auto iter = dependencyEntries.find(at);
            if (iter == dependencyEntries.end())
            {
                eastl::vector<job_dependency_entry_t> entries(at->type.length);
                iter = dependencyEntries.insert(std::make_pair(at, std::move(entries))).first;
            }

            auto entries = (*iter).second.data();
            auto& entry = entries[localType];
            update_entry(entry, counter, readonly, atomic, dependencies);
        }
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
        SMutexLock resourceLock(resourceMutex.mMutex);
        forloop (i, 0, resources->count)
        {
            auto& entry = allResources[e_id(resources->resources[i])];
            auto readonly = resources->readonly[i];
            auto atomic = resources->atomic[i];
            update_entry(entry, counter, readonly, atomic, dependencies);
        }
    }

    llvm_vecsmall::SmallVector<dual_group_t*, 64> groups;
    auto add_group = [&](dual_group_t* group) {
        groups.push_back(group);
    };
    auto& params = query->parameters;
    query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_group));
    forloop (i, 0, params.length)
    {
        if (type_index_t(params.types[i]).is_tag())
            continue;
        if (params.accesses[i].randomAccess == DOS_GLOBAL)
        {
            sync_type(params.types[i], params.accesses[i].readonly, params.accesses[i].atomic);
        }
        else
        {
            int groupIndex = 0;
            for (auto group : groups)
            {
                auto localType = group->index(params.types[i]);
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

    eastl::vector<std::shared_ptr<ftl::TaskCounter>> result;
    for(auto& counter : dependencies)
        result.push_back(std::move(counter));
    return result;
}

eastl::vector<std::shared_ptr<ftl::TaskCounter>> dual::scheduler_t::sync_resources(const std::shared_ptr<ftl::TaskCounter>& counter, dual_resource_operation_t* resources)
{
    DependencySet dependencies;
    {
        SMutexLock resourceLock(resourceMutex.mMutex);
        forloop (i, 0, resources->count)
        {
            auto& entry = allResources[e_id(resources->resources[i])];
            auto readonly = resources->readonly[i];
            auto atomic = resources->atomic[i];
            update_entry(entry, counter, readonly, atomic, dependencies);
        }
    }
    
    eastl::vector<std::shared_ptr<ftl::TaskCounter>> result;
    result.resize(dependencies.size());
    uint32_t dependencyIndex = 0;
    for (auto dependency : dependencies)
        result[dependencyIndex++] = dependency;

    return result;
}

dual_job_t::~dual_job_t()
{
}

dual_job_t::dual_job_t(dual::scheduler_t& scheduler)
    : scheduler(&scheduler)
    , counter(std::make_shared<ftl::TaskCounter>(scheduler.scheduler))
{
}

dual_ecs_job_t::~dual_ecs_job_t()
{
    if(payloads)
        dual_free(payloads);
    if(tasks)
        dual_free_aligned(tasks, 1); // tasks moved from eastl::vector uses aligned allocation
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
    std::shared_ptr<ftl::TaskCounter> counter;
};

void dualJ_schedule_ecs(const dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources, dual_counter_t** counter)
{
    if (counter)
    {
        *counter = SkrNew<dual_counter_t>(dual::scheduler_t::get().schedule_ecs_job(query, batchSize, callback, u, init, teardown, resources));
    }
    else
    {
        dual::scheduler_t::get().schedule_ecs_job(query, batchSize, callback, u, init, teardown, resources);
    }
}

void dualJ_schedule_custom(const dual_query_t* query, dual_counter_t* counter, dual_schedule_callback_t callback, void* u, dual_resource_operation_t* resources)
{
    auto results = dual::scheduler_t::get().schedule_custom_job(query, counter->counter, resources);
    for(auto& result : results)
    {
        callback(u, SkrNew<dual_counter_t>(std::move(result)));
    }
}

void dualJ_wait_counter(dual_counter_t* counter, int pin)
{
    dual::scheduler_t::get().scheduler->WaitForCounter(counter->counter.get(), pin);
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

void dualJ_initialize(dual_scheduler_t* scheduler)
{
    dual::scheduler_t::get().initialize((ftl::TaskScheduler*)scheduler);
}

dual_scheduler_t* dualJ_get_scheduler()
{
    return (dual_scheduler_t*)dual::scheduler_t::get().scheduler;
}