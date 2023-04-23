#include "scheduler.hpp"
#include "ecs/SmallVector.h"

#include "ecs/constants.hpp"
#include "ecs/dual.h"
#include "query.hpp"
#include "storage.hpp"

#include "type.hpp"
#include "utils/log.h"

#include "tracy/Tracy.hpp"

dual::scheduler_t::scheduler_t()
{
}

dual::scheduler_t::~scheduler_t()
{
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
    return storage->currentFiber == skr::task::current_fiber();
}

void dual::scheduler_t::set_main_thread(const dual_storage_t* storage)
{
    SKR_ASSERT(storage->scheduler == this);
    storage->counter.wait(true);
    storage->currentFiber = skr::task::current_fiber();
}

void dual::scheduler_t::add_storage(dual_storage_t* storage)
{
    SMutexLock lock(storageMutex.mMutex);
    storage->scheduler = this;
    storage->currentFiber = skr::task::current_fiber();
    storages.push_back(storage);
}
void dual::scheduler_t::remove_storage(const dual_storage_t* storage)
{
    sync_storage(storage);
    SMutexLock lock(storageMutex.mMutex);
    storage->scheduler = nullptr;
    storage->currentFiber = nullptr;
    storages.erase(std::remove(storages.begin(), storages.end(), storage), storages.end());
}

bool dual::scheduler_t::sync_archetype(dual::archetype_t* type)
{
    SKR_ASSERT(is_main_thread(type->storage));
    // TODO: performance optimization
    eastl::vector<skr::task::event_t> deps;
    {
        SMutexLock entryLock(entryMutex.mMutex);
        auto pair = dependencyEntries.find(type);
        if (pair == dependencyEntries.end())
        {
            return false;
        }
        auto entries = pair->second.data();
        auto count = type->type.length;
        forloop (i, 0, count)
        {
            if (type_index_t(type->type.data[i]).is_tag())
                break;
            for (auto dep : entries[i].owned)
                if(auto ptr = dep.lock())
                    deps.push_back(ptr);
            for (auto p : entries[i].shared)
                if(auto ptr = p.lock())
                    deps.push_back(ptr);
            entries[i].shared.clear();
            entries[i].owned.clear();
        }
        dependencyEntries.erase(pair);
    }
    for (auto dep : deps)
        dep.wait(true);
    return !deps.empty();
}

bool dual::scheduler_t::sync_entry(dual::archetype_t* type, dual_type_index_t i, bool readonly)
{
    SKR_ASSERT(is_main_thread(type->storage));
    // TODO: performance optimization
    eastl::vector<skr::task::event_t> deps;
    
    {
        SMutexLock entryLock(entryMutex.mMutex);
        auto pair = dependencyEntries.find(type);
        if (pair == dependencyEntries.end()) 
            return false;
        auto entries = pair->second.data();
        for (auto dep : entries[i].owned)
            if(auto ptr = dep.lock())
                deps.push_back(ptr);
        if(!readonly)
        {
            for (auto p : entries[i].shared)
                if(auto ptr = p.lock())
                    deps.push_back(ptr);
            entries[i].shared.clear();
        }
        entries[i].owned.clear();
    }
            
    for (auto dep : deps)
        dep.wait(true);
    return !deps.empty();
}

bool dual::scheduler_t::sync_query(dual_query_t* query)
{
    SKR_ASSERT(is_main_thread(query->storage));
    
    ZoneScopedN("SyncQuery");
    
    llvm_vecsmall::SmallVector<dual_group_t*, 64> groups;
    auto add_group = [&](dual_group_t* group) {
        groups.push_back(group);
    };
    query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_group));
    skr::flat_hash_set<std::pair<dual::archetype_t*, dual_type_index_t>> syncedEntry;

    auto sync_entry_once = [&](dual::archetype_t* type, dual_type_index_t i, bool readonly, bool atomic) -> bool
    {
        auto entry = std::make_pair(type, i);
        if (syncedEntry.find(entry) != syncedEntry.end())
            return false;
        auto result = sync_entry(type, i, readonly);
        syncedEntry.insert(entry);
        return result;
    };

    auto sync_type = [&](dual_type_index_t type, bool readonly, bool atomic) {
        bool result = false;
        for (auto& pair : query->storage->groups)
        {
            auto group = pair.second;
            auto idx = group->index(type);
            result = sync_entry_once(group->archetype, idx, readonly, atomic) || result;
        }
        return result;
    };

    bool result = false;
    {
        auto params = query->parameters;
        forloop (i, 0, params.length)
        {
            if (type_index_t(params.types[i]).is_tag())
                continue;
            if (params.accesses[i].randomAccess != DOS_SEQ)
            {
                result = sync_type(params.types[i], params.accesses[i].readonly, params.accesses[i].atomic) || result;
            }
            else
            {
                int groupIndex = 0;(void)groupIndex;
                for (auto group : groups)
                {
                    auto localType = group->index(params.types[i]);
                    if (localType == kInvalidTypeIndex)
                    {
                        auto g = group->get_owner(params.types[i]);
                        if (g)
                        {
                            result = sync_entry_once(g->archetype, g->index(params.types[i]), params.accesses[i].readonly, params.accesses[i].atomic) || result;
                        }
                    }
                    else
                    {
                        result = sync_entry_once(group->archetype, localType, params.accesses[i].readonly, params.accesses[i].atomic) || result;
                    }
                    ++groupIndex;
                }
            }
        }
    }
    
    for(auto subquery : query->subqueries)
    {
        result = sync_query(subquery) || result;
    }
    return result;
}

void dual::scheduler_t::sync_all()
{
    allCounter.wait(true);
    for(auto& pair : dependencyEntries)
    {
        for(auto& entry : pair.second)
        {
            entry.owned.clear();
            entry.shared.clear();
        }
    }
}

void dual::scheduler_t::gc_entries()
{
    for(auto& pair : dependencyEntries)
    {
        for(auto& entry : pair.second)
        {
            entry.owned.erase(std::remove_if(entry.owned.begin(), entry.owned.end(), [](skr::task::weak_event_t& e) { return e.expired(); }), entry.owned.end());
            entry.shared.erase(std::remove_if(entry.shared.begin(), entry.shared.end(), [](skr::task::weak_event_t& e) { return e.expired(); }), entry.shared.end());
        }
    }
}

void dual::scheduler_t::sync_storage(const dual_storage_t* storage)
{
    if (!storage->scheduler)
        return;
    storage->counter.wait(true);
    for(auto& pair : dependencyEntries)
    {
        if(pair.first->storage == storage)
        {
            for(auto& entry : pair.second)
            {
                entry.owned.clear();
                entry.shared.clear();
            }
        }
    }
}

namespace dual
{
struct hash_shared_ptr {
    size_t operator()(const skr::task::event_t& value) const
    {
        return value.hash();
    }
};
using DependencySet = skr::flat_hash_set<skr::task::event_t, hash_shared_ptr>;
void update_entry(job_dependency_entry_t& entry, skr::task::event_t job, bool readonly, bool atomic, DependencySet& dependencies)
{
    SKR_ASSERT(job);
    if (readonly)
    {
        for (auto& dp : entry.owned)
            if(auto ptr = dp.lock())
                dependencies.insert(ptr);
        entry.shared.push_back(job);
    }
    else
    {
        for (auto& dp : entry.shared)
            if(auto ptr = dp.lock())
                dependencies.insert(ptr);
        if (atomic)
        {
            entry.owned.push_back(job);
        }
        else
        {
            for (auto& dp : entry.owned)
                if(auto ptr = dp.lock())
                    dependencies.insert(ptr);
            entry.shared.clear();
            entry.owned.clear();
            entry.owned.push_back(job);
        }
    }
}
} // namespace dual

skr::task::event_t dual::scheduler_t::schedule_ecs_job(dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources)
{
    query->storage->build_queries();
    skr::task::event_t result;
    ZoneScopedN("SchedualECSJob");

    SKR_ASSERT(is_main_thread(query->storage));
    SKR_ASSERT(query->parameters.length < 32);
    llvm_vecsmall::SmallVector<dual_group_t*, 64> groups;
    auto add_group = [&](dual_group_t* group) {
        groups.push_back(group);
    };
    auto& params = query->parameters;
    query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_group));
    struct task_t {
        uint32_t groupIndex;
        uint32_t startIndex;
        dual_chunk_view_t view;
    };
    struct SharedData
    {
        dual_query_t* query;
        dual_group_t** groups;
        uint32_t groupCount;
        eastl::bitset<32>* readonly;
        dual_type_index_t* localTypes;
        eastl::bitset<32>* atomic;
        eastl::bitset<32>* randomAccess;
        bool hasRandomWrite;
        bool hasWriteChunkComponent;
        EIndex entityCount;
        dual_system_callback_t callback;
        void* userdata;
        eastl::vector<task_t> tasks;
    };
    SharedData* job = nullptr;
    eastl::shared_ptr<SharedData> sharedData;

    auto groupCount = (uint32_t)groups.size();
    {
        ZoneScopedN("AllocateSharedData");
        struct_arena_t<SharedData> arena;
        arena.record(&SharedData::groups, groupCount);
        arena.record(&SharedData::localTypes, groupCount * params.length);
        arena.record(&SharedData::readonly, groupCount);
        arena.record(&SharedData::atomic, groupCount);
        arena.record(&SharedData::randomAccess, groupCount);  
        job = arena.end();
        job->groups =       arena.get(&SharedData::groups, groupCount);
        job->localTypes =   arena.get(&SharedData::localTypes, groupCount * params.length);
        job->readonly =     arena.get(&SharedData::readonly, groupCount);
        job->atomic =       arena.get(&SharedData::atomic, groupCount);
        job->randomAccess = arena.get(&SharedData::randomAccess, groupCount);
    }
    job->groupCount = groupCount;
    job->hasRandomWrite = !query->subqueries.empty();
    job->hasWriteChunkComponent = false;
    job->entityCount = 0;
    job->callback = callback;
    job->userdata = u;
    job->query = query;
    sharedData.reset(job, [](SharedData* p)
    {
        SkrDelete(p);
    });
    std::memcpy(job->groups, groups.data(), groupCount * sizeof(dual_group_t*));
    int groupIndex = 0;
    for (auto group : groups)
    {
        job->entityCount += group->size;
        forloop (i, 0, params.length)
        {
            auto t = type_index_t(params.types[i]);
            auto idx = group->index(params.types[i]);
            job->localTypes[groupIndex * params.length + i] = idx;
            auto& op = params.accesses[i];
            if(idx != kInvalidTypeIndex)
            {
                job->readonly[groupIndex].set(idx, op.readonly);
                job->randomAccess[groupIndex].set(idx, op.randomAccess != DOS_SEQ);
                job->atomic[groupIndex].set(idx, op.atomic);
            }
            job->hasRandomWrite |= op.randomAccess != DOS_SEQ;
            job->hasWriteChunkComponent = t.is_chunk() && !op.readonly && !op.atomic;
        }
        ++groupIndex;
    }
    if(!job->entityCount)
        return {nullptr};

    auto dependencies = update_dependencies(query, result, resources);
    
    {
        ZoneScopedN("AllocateCounter");
        allCounter.add(1);
        query->storage->counter.add(1);
    }
    skr::task::schedule([dependencies = std::move(dependencies), sharedData, init, teardown, this, query, batchSize]()mutable
    {
        {
            ZoneScopedN("JobWaitDependencies");
            for(auto& dependency : dependencies)
                if(auto ptr = dependency.lock())
                    ptr.wait(false);
        }
        {
            ZoneScopedN("JobInitialize");
            if (init)
                init(sharedData->userdata, sharedData->entityCount);
        }
        SKR_DEFER({ 
            allCounter.decrement();
            query->storage->counter.decrement();
        });
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
        //TODO: expose this as a parameter
        bool scheduleSubchunkJobs = !sharedData->hasWriteChunkComponent;
        bool singleJob = sharedData->hasRandomWrite || batchSize == 0 || sharedData->entityCount <= batchSize;
        if (singleJob)
        {
            uint32_t startIndex = 0;
            forloop (i, 0, sharedData->groupCount)
            {
                auto processView = [&](dual_chunk_view_t* view) {
                    sharedData->callback(sharedData->userdata, query, view, sharedData->localTypes + i * query->parameters.length, startIndex);
                    startIndex += view->count;
                };
                auto group = sharedData->groups[i];
                query->storage->query(group, query->filter, validatedMeta, DUAL_LAMBDA(processView));
            }
        }
        else
        {
            struct batch_t {
                intptr_t startTask;
                intptr_t endTask;
            };
            eastl::vector<batch_t> batches;
            eastl::vector<task_t>& tasks = sharedData->tasks;
            {
                ZoneScopedN("JobBatching");
                batches.reserve(sharedData->entityCount / batchSize);
                tasks.reserve(batches.capacity());
                uint32_t batchRemain = batchSize;
                batch_t currBatch;
                currBatch.startTask = currBatch.endTask = 0;
                EIndex startIndex = 0;
                dual_chunk_t* currentChunk = nullptr;
                forloop (i, 0, sharedData->groupCount)
                {
                    auto scheduleView = [&](dual_chunk_view_t* view) {
                        if(!scheduleSubchunkJobs || sharedData->hasWriteChunkComponent)
                        {
                            if (batchRemain == 0 && currentChunk != view->chunk) // batch filled
                            {
                                currBatch.endTask = tasks.size();
                                batches.push_back(currBatch);
                                currBatch.startTask = currBatch.endTask; // new batch
                                batchRemain = batchSize;
                            }
                            currentChunk = view->chunk;
                            task_t newTask;
                            newTask.groupIndex = i;
                            newTask.startIndex = startIndex;
                            newTask.view = *view;
                            startIndex += view->count;
                            batchRemain -= std::min(batchRemain, view->count);
                            tasks.push_back(newTask);
                        }
                        else 
                        {
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
                                    batches.push_back(currBatch);
                                    currBatch.startTask = currBatch.endTask; // new batch
                                    batchRemain = batchSize;
                                }
                            }
                        }
                    };
                    auto group = sharedData->groups[i];
                    query->storage->query(group, query->filter, validatedMeta, DUAL_LAMBDA(scheduleView));
                };
                if (currBatch.endTask != tasks.size())
                {
                    currBatch.endTask = tasks.size();
                    batches.push_back(currBatch);
                }
            }
            
            
            skr::task::counter_t counter;
            counter.add((uint32_t)batches.size());
            for(auto batch : batches)
            {
                skr::task::schedule([batch, sharedData, counter]() mutable
                {
                    SKR_DEFER({
                        counter.decrement();
                    });
                    forloop (i, batch.startTask, batch.endTask)
                    {
                        auto task = sharedData->tasks[i];
                        sharedData->callback(sharedData->userdata, sharedData->query, &task.view, sharedData->localTypes + task.groupIndex * sharedData->query->parameters.length, task.startIndex);
                    }
                }, nullptr);
            }
            counter.wait(false);
        }

        {
            ZoneScopedN("JobTearDown0");
            if(teardown)
                teardown(sharedData->userdata, sharedData->entityCount);
        }
    }, &result);
    return result;
}

skr::task::event_t dual::scheduler_t::schedule_job(dual_query_t* query, dual_schedule_callback_t callback, void* u, dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources)
{
    skr::task::event_t result;
    auto deps = update_dependencies(query, result, resources);
    {
        ZoneScopedN("AllocateCounter");
        allCounter.add(1);
        query->storage->counter.add(1);
    }
    skr::task::schedule([deps = std::move(deps), this, query, callback, u, init, teardown]()
    {
        for(auto dep : deps)
            if(auto d = dep.lock())
                d.wait(false);
        SKR_DEFER({ 
            allCounter.decrement();
            query->storage->counter.decrement();
        });
        if(init)
            init(u, 0);
        callback(u, query);
        if(teardown)
            teardown(u, 0);
    }, &result);
    return result;
}

eastl::vector<skr::task::weak_event_t> dual::scheduler_t::update_dependencies(dual_query_t* query, const skr::task::event_t& counter, dual_resource_operation_t* resources)
{
    ZoneScopedN("SchedualCustomJob");

    llvm_vecsmall::SmallVector<dual_group_t*, 64> groups;
    auto add_group = [&](dual_group_t* group) {
        groups.push_back(group);
    };
    query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_group));
    auto& params = query->parameters;
    DependencySet dependencies;

    if (resources)
    {
        ZoneScopedN("UpdateResourcesEntries");
        SMutexLock resourceLock(resourceMutex.mMutex);
        forloop (i, 0, resources->count)
        {
            auto& entry = allResources[e_id(resources->resources[i])];
            auto readonly = resources->readonly[i];
            auto atomic = resources->atomic[i];
            update_entry(entry, counter, readonly, atomic, dependencies);
        }
    }

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

    {
        ZoneScopedN("UpdateArchetypeEntries");
        forloop (i, 0, params.length)
        {
            if (type_index_t(params.types[i]).is_tag())
                continue;
            if (params.accesses[i].randomAccess != DOS_SEQ)
            {
                sync_type(params.types[i], params.accesses[i].readonly, params.accesses[i].atomic);
            }
            else
            {
                int groupIndex = 0;(void)groupIndex;
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
        for(auto subquery : query->subqueries)
        {
            llvm_vecsmall::SmallVector<dual_group_t*, 64> subgroups;
            auto add_subgroup = [&](dual_group_t* group) {
                subgroups.push_back(group);
            };
            query->storage->query_groups(query->filter, query->meta, DUAL_LAMBDA(add_subgroup));
            forloop (i, 0, subquery->parameters.length)
            {
                if (type_index_t(subquery->parameters.types[i]).is_tag())
                    continue;
                if (subquery->parameters.accesses[i].randomAccess != DOS_SEQ)
                {
                    sync_type(subquery->parameters.types[i], subquery->parameters.accesses[i].readonly, subquery->parameters.accesses[i].atomic);
                }
                else
                {
                    int subgroupIndex = 0;(void)subgroupIndex;
                    for (auto subgroup : subgroups)
                    {
                        auto localType = subgroup->index(subquery->parameters.types[i]);
                        if (localType == kInvalidTypeIndex)
                        {
                            auto g = subgroup->get_owner(subquery->parameters.types[i]);
                            if (g)
                            {
                                sync_entry(g, g->index(subquery->parameters.types[i]), subquery->parameters.accesses[i].readonly, subquery->parameters.accesses[i].atomic);
                            }
                        }
                        else
                        {
                            sync_entry(subgroup, localType, subquery->parameters.accesses[i].readonly, subquery->parameters.accesses[i].atomic);
                        }
                        ++subgroupIndex;
                    }
                }
            }
        }
    }

    eastl::vector<skr::task::weak_event_t> result;
    for(auto& counter : dependencies)
        result.push_back(counter);
    return result;
}

eastl::vector<skr::task::event_t> dual::scheduler_t::sync_resources(const skr::task::event_t& counter, dual_resource_operation_t* resources)
{
    DependencySet dependencies;
    {
        SMutexLock entryLock(entryMutex.mMutex);
        forloop (i, 0, resources->count)
        {
            auto& entry = allResources[e_id(resources->resources[i])];
            auto readonly = resources->readonly[i];
            auto atomic = resources->atomic[i];
            update_entry(entry, counter, readonly, atomic, dependencies);
        }
    }
    eastl::vector<skr::task::event_t> result;

    result.resize(dependencies.size());
    uint32_t dependencyIndex = 0;
    for (auto dependency : dependencies)
        if(auto ptr = dependency)
            result[dependencyIndex++] = ptr;

    return result;
}

dual_entity_t dualJ_add_resource()
{
    return dual::scheduler_t::get().add_resource();
}

void dualJ_remove_resource(dual_entity_t id)
{
    dual::scheduler_t::get().remove_resource(id);
}

bool dualJ_schedule_ecs(dual_query_t* query, EIndex batchSize, dual_system_callback_t callback, void* u,
dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources, skr::task::event_t* counter)
{
    ZoneScopedN("dualJ::schedule_ecs");
    
    auto c = dual::scheduler_t::get().schedule_ecs_job(query, batchSize, callback, u, init, teardown, resources);
    if (counter)
    {
        *counter = c;
    }
    return !!c;
}

void dualJ_schedule_custom(dual_query_t* query, dual_schedule_callback_t callback, void* u,
dual_system_lifetime_callback_t init, dual_system_lifetime_callback_t teardown, dual_resource_operation_t* resources, skr::task::event_t* counter)
{
    ZoneScopedN("dualJ::schedule_custom");
    auto c = dual::scheduler_t::get().schedule_job(query, callback, u, init, teardown, resources);
    if (counter)
    {
        *counter = c;
    }
}

void dualJ_wait_all()
{
    dual::scheduler_t::get().sync_all();
}

void dualJ_gc()
{
    dual::scheduler_t::get().gc_entries();
}

void dualJ_wait_storage(dual_storage_t* storage)
{
    dual::scheduler_t::get().sync_storage(storage);
}

void dualJ_bind_storage(dual_storage_t* storage)
{
    dual::scheduler_t::get().add_storage(storage);
}

void dualJ_unbind_storage(dual_storage_t* storage)
{
    dual::scheduler_t::get().remove_storage(storage);
}