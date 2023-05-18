#include "MPShared/world_delta.h"
#include "MPShared/components.h"
#include "containers/hashmap.hpp"
#include "misc/make_zeroed.hpp"
#include "ecs/type_builder.hpp"
#include "containers/vector.hpp"
#include "ecs/set.hpp"

#include "ecs/array.hpp"
#include "misc/log.hpp"
#include "platform/time.h"
#include "ecs/entity.hpp"

BandwidthCounter::BandwidthCounter()
    : dataRecord(30)
{
    skr_init_hires_timer(&timer);
}

void BandwidthCounter::AddRecord(double bytes)
{
    dataRecord.push_back({skr_hires_timer_get_seconds(&timer, false), bytes});
}

double BandwidthCounter::GetBytePerSecond()
{
    if(dataRecord.size() > 5)
    {
        double totalBytes = 0;
        for(int i=0; i<dataRecord.size(); ++i)
        {
            totalBytes += dataRecord[i].second;
        }
        double totalTime = dataRecord.back().first - dataRecord.front().first;
        return totalBytes / totalTime;
    }
    return 0;
}

struct ComponentDeltaBuilder;
struct ComponentDeltaApplier;

struct ComponentDeltaBuilderRegistry
{
    skr::flat_hash_map<dual_type_index_t, ComponentDeltaBuilder> builders;

    static ComponentDeltaBuilderRegistry& Get()
    {
        static ComponentDeltaBuilderRegistry instance;
        return instance;
    }
};

struct ComponentDeltaApplierRegistry
{
    skr::flat_hash_map<dual_type_index_t, ComponentDeltaApplier> appliers;

    static ComponentDeltaApplierRegistry& Get()
    {
        static ComponentDeltaApplierRegistry instance;
        return instance;
    }
};

struct ComponentDeltaBuilder
{
    dual_type_index_t component;
    component_delta_build_callback_t callback;
    dual_type_index_t historyComponent = dual::kInvalidTypeIndex;
    dual_query_t* deltaQuery;

    void Initialize(dual_storage_t* storage)    
    {
        if(historyComponent != dual::kInvalidTypeIndex)
        {
            SKR_ASSERT(DUAL_IS_BUFFER(historyComponent));
            auto filter = make_zeroed<dual_filter_t>();
            dual_type_index_t types[] = { component, historyComponent, dual_id_of<CAuth>::get() };
            dual::type_builder_t all;
            all.with(types, 3);
            filter.all = all.build();
            dual_parameters_t params = {};
            dual_operation_t accesses[] = { 
                dual_operation_t{-1, 1, 0, 1}, 
                dual_operation_t{-1, 0, 0, 1},
                dual_operation_t{-1, 1, 0, 1},
            };
            params.types = types;
            params.accesses = accesses;
            params.length = 3;
            deltaQuery = dualQ_create(storage, &filter, &params);
        }
        else 
        {
            auto filter = make_zeroed<dual_filter_t>();
            dual::type_builder_t all;
            dual_type_index_t types[] = { component, dual_id_of<CAuth>::get() };
            all.with(types, 2);
            filter.all = all.build();
            dual_parameters_t params = {};
            dual_operation_t accesses[] = { 
                dual_operation_t{-1, 1, 0, 1}, 
                dual_operation_t{-1, 1, 0, 1} 
            };
            params.types = types;
            params.accesses = accesses;
            params.length = 2;
            deltaQuery = dualQ_create(storage, &filter, &params);
        }
    }

    void Release()
    {
        dualQ_release(deltaQuery);
    }

    skr::task::event_t GenerateDelta(uint32_t connectionId, uint32_t totalConnection, MPWorldDeltaViewBuilder& builder)
    {
        return callback(component, deltaQuery, {connectionId, totalConnection, historyComponent}, builder);
    }
};

struct WorldDeltaBuilder : IWorldDeltaBuilder
{
    skr::vector<ComponentDeltaBuilder> components;
    skr::vector<skr::task::event_t> dependencies;
    dual_query_t* worldDeltaQuery;
    dual_query_t* clearDirtyQuery;
    dual_query_t* deadQuery;
    dual_storage_t* storage;
    bool initialized = false;

    void Initialize(dual_storage_t* inStorage) override
    {
        if (initialized)
            return;
        storage = inStorage;
        worldDeltaQuery = dualQ_from_literal(storage, "[in]CPrefab,[inout]CAuth,[inout]dual::dirty_comp_t,[inout]CAuthTypeData");
        clearDirtyQuery = dualQ_from_literal(storage, "[inout]dual::dirty_comp_t,[has]CPrefab,[has]CAuth,[has]CAuthTypeData");
        deadQuery = dualQ_from_literal(storage, "[in]CAuth, [has]dead");
        ComponentDeltaBuilderRegistry& registry = ComponentDeltaBuilderRegistry::Get();
        for (auto& pair : registry.builders)
        {
            components.emplace_back(pair.second);
            components.back().Initialize(storage);
        }
        initialized = true;
    }

    ~WorldDeltaBuilder()
    {
        dualQ_release(worldDeltaQuery);
        dualQ_release(clearDirtyQuery);
        dualQ_release(deadQuery);
        for (auto& component : components)
        {
            component.Release();
        }
    }

    void GenerateDelta(skr::vector<MPWorldDeltaViewBuilder>& builder) override
    {
        SKR_ASSERT(initialized);
        for(auto& delta : builder)
        {
            delta.changed.clear();
            delta.created.clear();
            delta.dead.clear();
            delta.components.clear();
            delta.entities.clear();
            auto cmps = GetNetworkComponents();
            for(int i=0; i<cmps.length; ++i)
            {
                delta.components.emplace_back().type = GetNetworkComponentIndex(cmps.data[i]);
            } 
        }
        skr::vector<skr::flat_hash_map<dual_entity_t, NetEntityId>> localMaps;
        localMaps.resize(builder.size());
        auto GetNetworkEntityIndex = [&](dual_entity_t ent, uint32_t c) -> NetEntityId
        {
            auto it = localMaps[c].find(ent);
            if (it != localMaps[c].end())
                return it->second;
            NetEntityId index = builder[c].entities.size();
            builder[c].entities.emplace_back(ent);
            localMaps[c].insert(std::make_pair(ent, index));
            return index;
        };
        
        dual::type_builder_t history;
        for(auto& component : components)
        {
            if(component.historyComponent != dual::kInvalidTypeIndex)
                history.with(component.historyComponent);
        }
        if(history.empty())
            return;
        dual_type_set_t historySet = history.build();
        auto deltaType = make_zeroed<dual_delta_type_t>();
        deltaType.added.type = historySet;
        dual::array_comp_T<dual_group_t*, 16> groupToCast;
        auto checkHistoryComponents = [&](dual_group_t* group)
        {
            dual_entity_type_t type;
            dualG_get_type(group, &type);

            if(!dual::set_utils<dual_type_index_t>::all(type.type, historySet))
                groupToCast.emplace_back(group);
        };
        dualQ_get_groups(worldDeltaQuery, DUAL_LAMBDA(checkHistoryComponents));
        for(auto group : groupToCast)
        {
            dualS_cast_group_delta(storage, group, &deltaType, nullptr, nullptr);
        }
        //find changed component and deleted component
        //prepare changed component storage, record deleted component
        auto prepare = [&builder, &GetNetworkEntityIndex](dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
        {
            auto prefabs = (CPrefab*)dualV_get_owned_rw_local(view, localTypes[0]);
            auto auths = (CAuth*)dualV_get_owned_rw_local(view, localTypes[1]);
            auto relevances = (CRelevance*)dualV_get_owned_ro(view, dual_id_of<CRelevance>::get());
            auto dirtyMasks = (uint32_t*)dualV_get_owned_ro_local(view, localTypes[2]);
            auto authTypes = (dual::array_comp_T<dual_type_index_t, 8>*)dualV_get_owned_ro_local(view, localTypes[3]);
            auto ControllerId = dual_id_of<CController>::get();
            auto controller = (CController*)dualV_get_owned_ro(view, ControllerId);
            auto entities = dualV_get_entities(view);
            dual_entity_type_t type;
            dualG_get_type(dualC_get_group(view->chunk), &type);
            dual_type_index_t buffer[128];
            auto cmps = GetNetworkComponents();
            dual_type_set_t networkType = dual::set_utils<dual_type_index_t>::intersect(cmps, type.type, buffer);
            for(int j=0; j<builder.size(); ++j)
            {
                auto& delta = builder[j];
                for(int i=0; i<view->count; ++i)
                {
                    bool relevant = !relevances || relevances[i].mask[j];
                    //if not mapped and relevant, create entity
                    if(!auths[i].mappedConnection[j] && relevant) 
                    {
                        auths[i].mappedConnection[j] = true;
                        auths[i].initializedConnection[j] = false;
                        auto& data = delta.created.emplace_back();
                        auto netEntity = GetNetworkEntityIndex(entities[i], j);
                        data.entity = netEntity;
                        data.prefab = prefabs[i].prefab.get_serialized();
                        data.components.reserve(networkType.length);
                        for(int k=0; k<networkType.length; ++k)
                        {
                            if(networkType.data[k] == ControllerId)
                                if(j != controller[i].connectionId)
                                    continue;
                            data.components.push_back(GetNetworkComponentIndex(networkType.data[k]));
                            if(!DUAL_IS_TAG(networkType.data[k]))
                            {
                                for(auto z=0; z<delta.components.size(); ++z)
                                {
                                    auto t = GetNetworkComponent(delta.components[z].type);
                                    if(t == networkType.data[k])
                                    {
                                        delta.components[z].entities.emplace_back(netEntity);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    //not relevant, delete entity
                    else if(auths[i].mappedConnection[j] && !relevant)
                    {
                        auths[i].mappedConnection[j] = false;
                        delta.dead.emplace_back(GetNetworkEntityIndex(entities[i], j));
                    }
                    //relevant, check if changed
                    else if(auths[i].mappedConnection[j] && relevant)
                    {
                        auths[i].initializedConnection[j] = true;
                        dual_type_index_t buffer2[128];
                        dual_type_index_t buffer3[128];
                        auto deleted = dual::set_utils<dual_type_index_t>::substract(auths[i].mappedType, networkType, buffer2);
                        auto added = dual::set_utils<dual_type_index_t>::substract(networkType, auths[i].mappedType, buffer3);
                        
                        if(deleted.length == 0 && dirtyMasks[i] == 0 && added.length == 0)
                            continue;
                        if(deleted.length !=0 || added.length != 0)
                        {
                            MPEntityDeltaViewBuilder& changed = delta.changed.emplace_back();
                            auto netEntity = GetNetworkEntityIndex(entities[i], j);
                            changed.entity = netEntity;
                            changed.components.reserve(added.length);
                            changed.deleted.reserve(deleted.length);
                            for(int k=0; k<added.length; ++k)
                            {
                                if(added.data[k] == ControllerId)
                                    if(j != controller[i].connectionId)
                                        continue;
                                changed.components.push_back(GetNetworkComponentIndex(added.data[k]));
                                if(!DUAL_IS_TAG(added.data[k]))
                                {
                                    for(auto z=0; z<delta.components.size(); ++z)
                                    {
                                        auto t = GetNetworkComponent(delta.components[z].type);
                                        if(t == added.data[k])
                                        {
                                            delta.components[z].entities.emplace_back(netEntity);
                                            break;
                                        }
                                    }
                                }
                            }
                            for(int k=0; k<deleted.length; ++k)
                                changed.deleted.push_back(GetNetworkComponentIndex(deleted.data[k]));
                        }
                        for(int k=0; k<type.type.length; ++k)
                            if((dirtyMasks[i] & (1 << k)))
                            {
                                if(type.type.data[k] == ControllerId)
                                    if(j != controller[i].connectionId)
                                        continue;
                                if(!DUAL_IS_TAG(type.type.data[k]))
                                {
                                    for(auto z=0; z<delta.components.size(); ++z)
                                    {
                                        auto t = GetNetworkComponent(delta.components[z].type);
                                        if(t == type.type.data[k])
                                        {
                                            auto netEntity = GetNetworkEntityIndex(entities[i], j);
                                            delta.components[z].entities.emplace_back(netEntity);
                                            break;
                                        }
                                    }
                                }
                            }
                    }
                    authTypes[i].resize(networkType.length);
                    char* temp = (char*)authTypes[i].data();
                    auths[i].mappedType = dual::clone(networkType, temp);
                }
            }
        };
        dualJ_schedule_ecs(worldDeltaQuery, 0, DUAL_LAMBDA(prepare), nullptr, nullptr, nullptr, nullptr);  
        skr::task::event_t completed(nullptr);
        dual::schedule_task(deadQuery, 0, [&](dual::task_context_t ctx)
        {
            auto auths = ctx.get_owned_ro<CAuth>(0);
            auto entities = ctx.get_entities();
            for(int j=0; j<builder.size(); ++j)
            {
                auto& delta = builder[j];
                for(int i=0; i<ctx.count(); ++i)
                {
                    //if mapped, send dead message
                    if(auths[i].mappedConnection[j]) 
                    {
                        auto netEntity = GetNetworkEntityIndex(entities[i], j);
                        delta.dead.push_back(netEntity);
                    }
                }
            }
        }, &completed);
        for (auto& component : components)
        {
            uint32_t i = 0;
            for(auto& delta : builder)
            {
                dependencies.emplace_back(component.GenerateDelta(i, builder.size(), delta));
                ++i;
            }
        }
        for (auto& dependency : dependencies)
        {
            dependency.wait(true);
        }
        dependencies.clear();
        dualJ_schedule_ecs(clearDirtyQuery, 0, 
        +[](void* u, dual_query_t* query, dual_chunk_view_t* view, dual_type_index_t* localTypes, EIndex entityIndex)
        {
            auto dirties = (uint32_t*)dualV_get_owned_rw_local(view, localTypes[0]);
            for(int i=0; i<view->count; ++i)
            {
                dirties[i] = 0;
            }
            
        }, this, nullptr, nullptr, nullptr, nullptr);
        if(completed)
            completed.wait(true);
        {
            for(auto& delta : builder)
            {
                delta.maxEntity = 0;
                for(auto& entity : delta.entities)
                {
                    EIndex maxId = std::max(DUAL_ENTITY_ID(delta.maxEntity), DUAL_ENTITY_ID(entity.entity));
                    EIndex maxVersion = std::max(DUAL_ENTITY_VERSION(delta.maxEntity), DUAL_ENTITY_VERSION(entity.entity));

                    delta.maxEntity = DUAL_ENTITY(maxId, maxVersion);
                }
            }
        }
        {
            for(auto& delta : builder)
            {
                delta.components.erase(std::remove_if(delta.components.begin(), delta.components.end(), [](auto& c) { return c.data.size() == 0; }), delta.components.end());
            }
        }
    }
};



struct ComponentDeltaApplier
{
    dual_type_index_t component;
    component_delta_apply_callback_t callback;
    dual_query_t* deltaQuery;
    BandwidthCounter bandwidthCounter;

    void Initialize(dual_storage_t* storage)
    {
        auto filter = make_zeroed<dual_filter_t>();
        dual::type_builder_t all;
        all.with(component).with<CNetwork>();
        filter.all = all.build();
        dual_parameters_t params = {};
        dual_type_index_t types[] = { component, dual_id_of<CNetwork>::get() };
        dual_operation_t accesses[] = {
            dual_operation_t{0, 0, 0, 1},
            dual_operation_t{-1, 1, 0, 1},
        };
        params.types = types;
        params.accesses = accesses;
        params.length = 2;
        deltaQuery = dualQ_create(storage, &filter, &params);
    }

    void Release()
    {
        dualQ_release(deltaQuery);
    }

    skr::task::event_t ApplyDelta(const MPWorldDeltaView& delta, const entity_map_t& entityMap)
    {
        auto iter = std::find_if(delta.components.begin(),delta.components.end(), [&](const MPComponentDeltaView& comp)
        {
            return GetNetworkComponent(comp.type) == component;
        });
        if(iter == delta.components.end())
        {
            bandwidthCounter.AddRecord(0);
            return skr::task::event_t(nullptr);
        }
        bandwidthCounter.AddRecord(iter->data.size());
        return callback(component, deltaQuery, delta, entityMap);
    }
};

struct WorldDeltaApplier : IWorldDeltaApplier
{
    skr::vector<ComponentDeltaApplier> components;
    skr::vector<skr::task::event_t> dependencies;
    dual_storage_t* storage;
    SpawnPrefab_t spawnPrefab;
    DestroyEntity_t destroyEntity;
    dual_query_t* worldDeltaQuery;
    bool initialized = false;

    void Initialize(dual_storage_t* inStorage, SpawnPrefab_t inSpawnPrefab, DestroyEntity_t inDestroyPrefab) override
    {
        if (initialized)
            return;
        storage = inStorage;
        spawnPrefab = std::move(inSpawnPrefab);
        destroyEntity = std::move(inDestroyPrefab);
        ComponentDeltaApplierRegistry& registry = ComponentDeltaApplierRegistry::Get();
        for (auto& pair : registry.appliers)
        {
            components.emplace_back(pair.second);
            components.back().Initialize(storage);
        }
        worldDeltaQuery = dualQ_from_literal(storage, "[in]CNetwork");
        initialized = true;
    }

    ~WorldDeltaApplier() override
    {
        if (!initialized)
            return;
        dualQ_release(worldDeltaQuery);
        for (auto& component : components)
        {
            component.Release();
        }
    }

    void ApplyDelta(const MPWorldDeltaView& delta, entity_map_t& map) override
    {
        ZoneScopedN("WorldDeltaApplier::ApplyDelta");
        SKR_ASSERT(initialized);
        map.clear();
        {
            ZoneScopedN("Build Entity Map");
            auto callback = [&](dual_chunk_view_t* view) {
                auto networks = (CNetwork*)dualV_get_owned_ro(view, dual_id_of<CNetwork>::get());
                auto ents = dualV_get_entities(view);
                for (int i = 0; i < view->count; ++i)
                    map.emplace(networks[i].serverEntity, ents[i]);
            };
            dualQ_get_views(worldDeltaQuery, DUAL_LAMBDA(callback));
        }
        {
            ZoneScopedN("Apply Structural Changes");
            for (auto& pair : delta.changed)
            {
                auto iter = map.find(delta.entities[pair.entity]);
                SKR_ASSERT(iter != map.end());
                skr::vector<dual_type_index_t> added;
                skr::vector<dual_type_index_t> removed;
                added.reserve(pair.components.size());
                removed.reserve(pair.deleted.size());
                for (auto& comp : pair.components)
                    added.push_back(GetNetworkComponent(comp));
                for (auto& comp : pair.deleted)
                    removed.push_back(GetNetworkComponent(comp));
                dual_delta_type_t delta = make_zeroed<dual_delta_type_t>();
                delta.added.type.data = added.data();
                delta.added.type.length = added.size();
                delta.removed.type.data = removed.data();
                delta.removed.type.length = removed.size();
                {
                    dual_chunk_view_t view;
                    dualS_access(storage, iter->second, &view);
                    dualS_cast_view_delta(storage, &view, &delta, nullptr, nullptr);
                }
            }
            for(auto& eid : delta.dead)
            {
                auto entity = delta.entities[eid];
                SKR_LOG_FMT_DEBUG(u8"Entity dead recived {}:{}", dual::e_id(entity), dual::e_version(entity));

                auto iter = map.find(entity);
                SKR_ASSERT(iter != map.end());
                destroyEntity(storage, iter->second);
                map.erase(iter);
            }
            for (auto& pair : delta.created)
            {
                auto entity = delta.entities[pair.entity];
                SKR_LOG_FMT_DEBUG(u8"New entity recived {}:{}", dual::e_id(entity), dual::e_version(entity));
                skr::vector<dual_type_index_t> added;
                added.reserve(pair.components.size());
                for (auto& comp : pair.components)
                    added.push_back(GetNetworkComponent(comp));
                
                dual_entity_type_t type;
                type.type.data = added.data();
                type.type.length = added.size();
                map.emplace(entity, spawnPrefab(storage, entity, pair.prefab, &type));
            }
        }
        {
            ZoneScopedN("Apply Component Changes");
            for (auto& component : components)
            {
                auto event = component.ApplyDelta(delta, map);
                if(event)
                    dependencies.emplace_back(std::move(event));
            }
            {
                ZoneScopedN("Syncing");
                for (auto& dependency : dependencies)
                {
                    dependency.wait(true);
                }
                dependencies.clear();
            }
        }
    }

    double GetBandwidthOf(dual_type_index_t component) override
    {
        for (auto& comp : components)
        {
            if (comp.component == component)
                return comp.bandwidthCounter.GetBytePerSecond();
        }
        return 0.0;
    }
};

IWorldDeltaBuilder* CreateWorldDeltaBuilder()
{
    return SkrNew<WorldDeltaBuilder>();
}

IWorldDeltaApplier* CreateWorldDeltaApplier()
{
    return SkrNew<WorldDeltaApplier>();
}

void RegisterComponentDeltaBuilder(dual_type_index_t component, component_delta_build_callback_t inCallback, dual_type_index_t historyComponent)
{
    ComponentDeltaBuilderRegistry::Get().builders[component] = ComponentDeltaBuilder{component, inCallback, historyComponent};
}

void RegisterComponentDeltaApplier(dual_type_index_t component, component_delta_apply_callback_t inCallback)
{
    ComponentDeltaApplierRegistry::Get().appliers[component] = ComponentDeltaApplier{component, inCallback};
}
int skr::binary::WriteTrait<const packed_entity_t&>::Write(skr_binary_writer_t *writer, const packed_entity_t &value, dual_entity_t maxEntity)
{
    uint32_t id = DUAL_ENTITY_ID(value.entity);
    uint32_t version = DUAL_ENTITY_VERSION(value.entity);
    uint32_t idMax = DUAL_ENTITY_ID(maxEntity);
    uint32_t versionMax = DUAL_ENTITY_VERSION(maxEntity);

    auto ret = Archive(writer, id, IntegerSerdeConfig<uint32_t>{0, idMax});
    if (ret != 0)
        return ret;
    ret = Archive(writer, version, IntegerSerdeConfig<uint32_t>{0, versionMax});
    return ret;
}
int skr::binary::ReadTrait<packed_entity_t>::Read(skr_binary_reader_t *reader, packed_entity_t &value, dual_entity_t maxEntity)
{
    uint32_t id = 0;
    uint32_t version = 0;
    uint32_t idMax = DUAL_ENTITY_ID(maxEntity);
    uint32_t versionMax = DUAL_ENTITY_VERSION(maxEntity);
    auto ret = Archive(reader, id, IntegerSerdeConfig<uint32_t>{0, idMax});
    if (ret != 0)
        return ret;
    ret = Archive(reader, version, IntegerSerdeConfig<uint32_t>{0, versionMax});
    if (ret != 0)
        return ret;
    value.entity = DUAL_ENTITY(id, version);
    return 0;
}
#include "serde/json/writer.h"
void skr::json::WriteTrait<const packed_entity_t&>::Write(skr_json_writer_t *writer, const packed_entity_t &value)
{
    writer->StartObject();
    writer->Key(u8"id");
    writer->UInt(DUAL_ENTITY_ID(value.entity));
    writer->Key(u8"version");
    writer->UInt(DUAL_ENTITY_VERSION(value.entity));
    writer->EndObject();
}