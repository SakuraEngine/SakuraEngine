#include "SkrProfile/profile.h"
#include "SkrContainers/sptr.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/type_index.hpp"

#include "./impl/query.hpp"
#include "./impl/storage.hpp"
#include "./scheduler.hpp"

#include <bitset>

#ifndef forloop
#define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif

sugoi::scheduler_t::scheduler_t()
{

}

sugoi::scheduler_t::~scheduler_t()
{

}

sugoi_entity_t sugoi::scheduler_t::add_resource()
{
    sugoi_entity_t result;
    registry.new_entities(&result, 1);
    return result;
}

void sugoi::scheduler_t::remove_resource(sugoi_entity_t id)
{
    registry.free_entities(&id, 1);
}

bool sugoi::scheduler_t::is_main_thread(const sugoi_storage_t* storage)
{
    SKR_ASSERT(storage->pimpl->scheduler == this);
    return storage->pimpl->currentFiber == skr::task::current_fiber();
}

void sugoi::scheduler_t::set_main_thread(const sugoi_storage_t* storage)
{
    SKR_ASSERT(storage->pimpl->scheduler == this);
    storage->pimpl->storage_counter.wait(true);
    storage->pimpl->currentFiber = skr::task::current_fiber();
}

void sugoi::scheduler_t::add_storage(sugoi_storage_t* storage)
{
    SMutexLock lock(storageMutex.mMutex);
    storage->pimpl->scheduler = this;
    storage->pimpl->currentFiber = skr::task::current_fiber();
    storages.push_back(storage);
}
void sugoi::scheduler_t::remove_storage(const sugoi_storage_t* storage)
{
    sync_storage(storage);
    SMutexLock lock(storageMutex.mMutex);
    storage->pimpl->scheduler = nullptr;
    storage->pimpl->currentFiber = nullptr;
    storages.erase(std::remove(storages.begin(), storages.end(), storage), storages.end());
}

bool sugoi::scheduler_t::sync_archetype(sugoi::archetype_t* type)
{
    SKR_ASSERT(is_main_thread(type->storage));
    // TODO: performance optimization
    skr::stl_vector<skr::task::event_t> deps;
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

bool sugoi::scheduler_t::sync_entry(sugoi::archetype_t* type, sugoi_type_index_t i, bool readonly)
{
    SKR_ASSERT(is_main_thread(type->storage));
    // TODO: performance optimization
    skr::stl_vector<skr::task::event_t> deps;
    
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

bool sugoi::scheduler_t::sync_query(sugoi_query_t* q)
{
    SKR_ASSERT(is_main_thread(q->pimpl->storage));
    
    SkrZoneScopedN("SyncQuery");
    
    skr::InlineVector<sugoi_group_t*, 64> groups;
    auto add_group = [&](sugoi_group_t* group) {
        groups.push_back(group);
    };
    q->pimpl->storage->filter_groups(q->pimpl->filter, q->pimpl->meta, SUGOI_LAMBDA(add_group));
    skr::FlatHashSet<std::pair<sugoi::archetype_t*, sugoi_type_index_t>> syncedEntry;

    auto sync_entry_once = [&](sugoi::archetype_t* type, sugoi_type_index_t i, bool readonly, bool atomic) -> bool
    {
        if(i == kInvalidTypeIndex)
            return false;
        auto entry = std::make_pair(type, i);
        if (syncedEntry.find(entry) != syncedEntry.end())
            return false;
        auto result = sync_entry(type, i, readonly);
        syncedEntry.insert(entry);
        return result;
    };

    auto sync_type = [&](sugoi_type_index_t type, bool readonly, bool atomic) {
        bool result = false;
        q->pimpl->storage->pimpl->groups.read_versioned([&](auto& groups) {
            for (auto& [_, group] : groups)
            {
                const auto idx = group->archetype->index(type);
                result = sync_entry_once(group->archetype, idx, readonly, atomic) || result;
            }
        }, 
        [&](){
            return q->pimpl->storage->pimpl->storage_timestamp;
        });
        return result;
    };

    bool result = false;
    {
        auto params = q->pimpl->parameters;
        forloop (i, 0, params.length)
        {
            if (type_index_t(params.types[i]).is_tag())
                continue;
            if (params.accesses[i].randomAccess != SOS_SEQ)
            {
                result = sync_type(params.types[i], params.accesses[i].readonly, params.accesses[i].atomic) || result;
            }
            else
            {
                int groupIndex = 0;(void)groupIndex;
                for (auto group : groups)
                {
                    auto localType = group->archetype->index(params.types[i]);
                    if (localType == kInvalidTypeIndex)
                    {
                        if (auto g = group->get_owner(params.types[i]))
                        {
                            result = sync_entry_once(g->archetype, g->archetype->index(params.types[i]), params.accesses[i].readonly, params.accesses[i].atomic) || result;
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
    
    for(auto subquery : q->pimpl->subqueries)
    {
        result = sync_query(subquery) || result;
    }
    return result;
}

void sugoi::scheduler_t::sync_all()
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

void sugoi::scheduler_t::gc_entries()
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

void sugoi::scheduler_t::sync_storage(const sugoi_storage_t* storage)
{
    if (!storage->pimpl->scheduler)
        return;
    storage->pimpl->storage_counter.wait(true);
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

namespace sugoi
{
struct hash_shared_ptr {
    size_t operator()(const skr::task::event_t& value) const
    {
        return value.hash();
    }
};
using DependencySet = skr::FlatHashSet<skr::task::event_t, hash_shared_ptr>;
void update_entry(JobDependencyEntry& entry, skr::task::event_t job, bool readonly, bool atomic, DependencySet& dependencies)
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
} // namespace sugoi

skr::task::event_t sugoi::scheduler_t::schedule_ecs_job(sugoi_query_t* q, EIndex batchSize, sugoi_system_callback_t callback, void* u,
sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources)
{
    skr::task::event_t result;
    SkrZoneScopedN("SchedualECSJob");

    SKR_ASSERT(is_main_thread(q->pimpl->storage));
    SKR_ASSERT(q->pimpl->parameters.length < 32);
    skr::InlineVector<sugoi_group_t*, 64> groups;
    auto add_group = [&](sugoi_group_t* group) {
        groups.push_back(group);
    };
    auto& params = q->pimpl->parameters;
    q->pimpl->storage->filter_groups(q->pimpl->filter, q->pimpl->meta, SUGOI_LAMBDA(add_group));
    struct task_t {
        uint32_t groupIndex;
        uint32_t startIndex;
        sugoi_chunk_view_t view;
    };
    struct SharedData
    {
        sugoi_query_t* query;
        sugoi_group_t** groups;
        uint32_t groupCount;
        std::bitset<32>* readonly;
        sugoi_type_index_t* localTypes;
        std::bitset<32>* atomic;
        std::bitset<32>* randomAccess;
        bool hasRandomWrite;
        bool hasWriteChunkComponent;
        EIndex entityCount;
        sugoi_system_callback_t callback;
        void* userdata;
        skr::stl_vector<task_t> tasks;
    };
    SharedData* job = nullptr;
    skr::SPtr<SharedData> sharedData;

    auto groupCount = (uint32_t)groups.size();
    {
        SkrZoneScopedN("AllocateSharedData");
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
    job->hasRandomWrite = !q->pimpl->subqueries.empty();
    job->hasWriteChunkComponent = false;
    job->entityCount = 0;
    job->callback = callback;
    job->userdata = u;
    job->query = q;
    sharedData.reset(job, [](SharedData* p)
    {
        SkrDelete(p);
    });
    std::memcpy(job->groups, groups.data(), groupCount * sizeof(sugoi_group_t*));
    int groupIndex = 0;
    for (auto group : groups)
    {
        job->entityCount += group->size;
        forloop (i, 0, params.length)
        {
            const auto t = type_index_t(params.types[i]);
            const auto idx = group->index(params.types[i]);
            job->localTypes[groupIndex * params.length + i] = idx;
            const auto& op = params.accesses[i];
            if(idx != kInvalidTypeIndex)
            {
                job->readonly[groupIndex].set(idx, op.readonly);
                job->randomAccess[groupIndex].set(idx, op.randomAccess != SOS_SEQ);
                job->atomic[groupIndex].set(idx, op.atomic);
            }
            job->hasRandomWrite |= (op.randomAccess != SOS_SEQ);
            job->hasWriteChunkComponent = t.is_chunk() && !op.readonly && !op.atomic;
        }
        ++groupIndex;
    }
    if(!job->entityCount)
        return {nullptr};

    auto dependencies = update_dependencies(q, result, resources);
    {
        SkrZoneScopedN("AllocateCounter");
        allCounter.add(1);
        q->pimpl->storage->pimpl->storage_counter.add(1);
    }
    skr::task::schedule([dependencies = std::move(dependencies), sharedData, init, teardown, this, q, batchSize]()mutable
    {
        {
            SkrZoneScopedN("JobWaitDependencies");
            for(auto& dependency : dependencies)
                if(auto ptr = dependency.lock())
                    ptr.wait(false);
        }
        {
            SkrZoneScopedN("JobInitialize");
            if (init)
                init(sharedData->userdata, sharedData->entityCount);
        }
        SKR_DEFER({ 
            allCounter.decrement();
            q->pimpl->storage->pimpl->storage_counter.decrement();
        });
        fixed_stack_scope_t _(localStack);
        sugoi_meta_filter_t validatedMeta;
        {
            SkrZoneScopedN("JobValidateMeta");

            auto& meta = q->pimpl->meta;
            auto data = (char*)localStack.allocate(data_size(meta));
            validatedMeta = clone(meta, data);
            q->pimpl->storage->validate(validatedMeta.all_meta);
            q->pimpl->storage->validate(validatedMeta.none_meta);
        }
        //TODO: expose this as a parameter
        bool scheduleSubchunkJobs = !sharedData->hasWriteChunkComponent;
        bool singleJob = sharedData->hasRandomWrite || batchSize == 0 || sharedData->entityCount <= batchSize;
        if (singleJob)
        {
            uint32_t startIndex = 0;
            forloop (i, 0, sharedData->groupCount)
            {
                auto processView = [&](sugoi_chunk_view_t* view) {
                    sharedData->callback(sharedData->userdata, q, view, sharedData->localTypes + i * q->pimpl->parameters.length, startIndex);
                    startIndex += view->count;
                };
                auto group = sharedData->groups[i];
                q->pimpl->storage->filter_in_single_group(&q->pimpl->parameters, group, q->pimpl->filter, validatedMeta, q->pimpl->customFilter, q->pimpl->customFilterUserData, SUGOI_LAMBDA(processView));
            }
        }
        else
        {
            struct batch_t {
                intptr_t startTask;
                intptr_t endTask;
            };
            skr::stl_vector<batch_t> batches;
            skr::stl_vector<task_t>& tasks = sharedData->tasks;
            {
                SkrZoneScopedN("JobBatching");
                batches.reserve(sharedData->entityCount / batchSize);
                tasks.reserve(batches.capacity());
                uint32_t batchRemain = batchSize;
                batch_t currBatch;
                currBatch.startTask = currBatch.endTask = 0;
                EIndex startIndex = 0;
                sugoi_chunk_t* currentChunk = nullptr;
                forloop (i, 0, sharedData->groupCount)
                {
                    auto scheduleView = [&](sugoi_chunk_view_t* view) {
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
                                newTask.view = sugoi_chunk_view_t{ view->chunk, view->start + allocated, subViewCount };
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
                    q->pimpl->storage->filter_in_single_group(
                        &q->pimpl->parameters, group, q->pimpl->filter, validatedMeta, 
                        q->pimpl->customFilter, q->pimpl->customFilterUserData, SUGOI_LAMBDA(scheduleView));
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
                        sharedData->callback(sharedData->userdata, sharedData->query, &task.view, sharedData->localTypes + task.groupIndex * sharedData->query->pimpl->parameters.length, task.startIndex);
                    }
                }, nullptr);
            }
            counter.wait(false);
        }

        {
            SkrZoneScopedN("JobTearDown0");
            if(teardown)
                teardown(sharedData->userdata, sharedData->entityCount);
        }
    }, &result);
    return result;
}

skr::task::event_t sugoi::scheduler_t::schedule_job(sugoi_query_t* q, sugoi_schedule_callback_t callback, void* u, sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources)
{
    skr::task::event_t result;
    auto deps = update_dependencies(q, result, resources);
    {
        SkrZoneScopedN("AllocateCounter");
        allCounter.add(1);
        q->pimpl->storage->pimpl->storage_counter.add(1);
    }
    skr::task::schedule([deps = std::move(deps), this, q, callback, u, init, teardown]()
    {
        for(auto dep : deps)
            if(auto d = dep.lock())
                d.wait(false);
        SKR_DEFER({ 
            allCounter.decrement();
            q->pimpl->storage->pimpl->storage_counter.decrement();
        });
        if(init)
            init(u, 0);
        callback(u, q);
        if(teardown)
            teardown(u, 0);
    }, &result);
    return result;
}

skr::stl_vector<skr::task::weak_event_t> sugoi::scheduler_t::update_dependencies(sugoi_query_t* q, const skr::task::event_t& counter, sugoi_resource_operation_t* resources)
{
    SkrZoneScopedN("schedualCustomJob");

    skr::InlineVector<sugoi_group_t*, 64> groups;
    auto add_group = [&](sugoi_group_t* group) {
        groups.push_back(group);
    };
    q->pimpl->storage->filter_groups(q->pimpl->filter, q->pimpl->meta, SUGOI_LAMBDA(add_group));
    auto& params = q->pimpl->parameters;
    DependencySet dependencies;

    if (resources)
    {
        SkrZoneScopedN("UpdateResourcesEntries");
        SMutexLock resourceLock(resourceMutex.mMutex);
        forloop (i, 0, resources->count)
        {
            auto& entry = allResources[e_id(resources->resources[i])];
            auto readonly = resources->readonly[i];
            auto atomic = resources->atomic[i];
            update_entry(entry, counter, readonly, atomic, dependencies);
        }
    }

    skr::FlatHashSet<std::pair<sugoi::archetype_t*, sugoi_type_index_t>> syncedEntry;
    auto sync_entry = [&](const sugoi_group_t* group, sugoi_type_index_t localType, bool readonly, bool atomic) {
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
                skr::stl_vector<JobDependencyEntry> entries(at->type.length);
                iter = dependencyEntries.insert(std::make_pair(at, std::move(entries))).first;
            }

            auto entries = (*iter).second.data();
            auto& entry = entries[localType];
            update_entry(entry, counter, readonly, atomic, dependencies);
        }
    };

    auto sync_type = [&](sugoi_type_index_t type, bool readonly, bool atomic) {
        q->pimpl->storage->pimpl->groups.read_versioned([&](auto& groups) {
            for (auto& pair : groups)
            {
                auto group = pair.second;
                auto idx = group->index(type);
                sync_entry(group, idx, readonly, atomic);
            }
        }, 
        [&](){
            return q->pimpl->storage->pimpl->storage_timestamp;
        });
    };

    {
        SkrZoneScopedN("UpdateArchetypeEntries");
        forloop (i, 0, params.length)
        {
            if (type_index_t(params.types[i]).is_tag())
                continue;
            if (params.accesses[i].randomAccess != SOS_SEQ)
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
        // sync subqueries
        for(auto subquery : q->pimpl->subqueries)
        {
            skr::InlineVector<sugoi_group_t*, 64> subgroups;
            auto add_subgroup = [&](sugoi_group_t* group) {
                subgroups.push_back(group);
            };
            q->pimpl->storage->filter_groups(q->pimpl->filter, q->pimpl->meta, SUGOI_LAMBDA(add_subgroup));
            forloop (i, 0, subquery->pimpl->parameters.length)
            {
                if (type_index_t(subquery->pimpl->parameters.types[i]).is_tag())
                    continue;
                if (subquery->pimpl->parameters.accesses[i].randomAccess != SOS_SEQ)
                {
                    sync_type(subquery->pimpl->parameters.types[i], subquery->pimpl->parameters.accesses[i].readonly, subquery->pimpl->parameters.accesses[i].atomic);
                }
                else
                {
                    int subgroupIndex = 0;(void)subgroupIndex;
                    for (auto subgroup : subgroups)
                    {
                        auto localType = subgroup->index(subquery->pimpl->parameters.types[i]);
                        if (localType == kInvalidTypeIndex)
                        {
                            if (auto g = subgroup->get_owner(subquery->pimpl->parameters.types[i]))
                            {
                                sync_entry(g, g->index(subquery->pimpl->parameters.types[i]), subquery->pimpl->parameters.accesses[i].readonly, subquery->pimpl->parameters.accesses[i].atomic);
                            }
                        }
                        else
                        {
                            sync_entry(subgroup, localType, subquery->pimpl->parameters.accesses[i].readonly, subquery->pimpl->parameters.accesses[i].atomic);
                        }
                        ++subgroupIndex;
                    }
                }
            }
        }
    }

    skr::stl_vector<skr::task::weak_event_t> result;
    for(auto& counter : dependencies)
        result.push_back(counter);
    return result;
}

skr::stl_vector<skr::task::event_t> sugoi::scheduler_t::sync_resources(const skr::task::event_t& counter, sugoi_resource_operation_t* resources)
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
    skr::stl_vector<skr::task::event_t> result;

    result.resize(dependencies.size());
    uint32_t dependencyIndex = 0;
    for (auto dependency : dependencies)
        if(auto ptr = dependency)
            result[dependencyIndex++] = ptr;

    return result;
}

sugoi_entity_t sugoiJ_add_resource()
{
    return sugoi::scheduler_t::get().add_resource();
}

void sugoiJ_remove_resource(sugoi_entity_t id)
{
    sugoi::scheduler_t::get().remove_resource(id);
}

bool sugoiJ_schedule_ecs(sugoi_query_t* query, EIndex batchSize, sugoi_system_callback_t callback, void* u,
sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources, skr::task::event_t* counter)
{
    SkrZoneScopedN("sugoiJ::schedule_ecs");
    
    auto c = sugoi::scheduler_t::get().schedule_ecs_job(query, batchSize, callback, u, init, teardown, resources);
    if (counter)
    {
        *counter = c;
    }
    return !!c;
}

void sugoiJ_schedule_custom(sugoi_query_t* query, sugoi_schedule_callback_t callback, void* u,
sugoi_system_lifetime_callback_t init, sugoi_system_lifetime_callback_t teardown, sugoi_resource_operation_t* resources, skr::task::event_t* counter)
{
    SkrZoneScopedN("sugoiJ::schedule_custom");
    auto c = sugoi::scheduler_t::get().schedule_job(query, callback, u, init, teardown, resources);
    if (counter)
    {
        *counter = c;
    }
}

void sugoiJ_wait_all()
{
    sugoi::scheduler_t::get().sync_all();
}

void sugoiJ_gc()
{
    sugoi::scheduler_t::get().gc_entries();
}

void sugoiJ_wait_storage(sugoi_storage_t* storage)
{
    sugoi::scheduler_t::get().sync_storage(storage);
}

void sugoiJ_bind_storage(sugoi_storage_t* storage)
{
    sugoi::scheduler_t::get().add_storage(storage);
}

void sugoiJ_unbind_storage(sugoi_storage_t* storage)
{
    sugoi::scheduler_t::get().remove_storage(storage);
}