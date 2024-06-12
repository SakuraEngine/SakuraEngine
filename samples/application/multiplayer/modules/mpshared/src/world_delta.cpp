#include "MPShared/world_delta.h"
#include "MPShared/components.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrRT/ecs/set.hpp"
#include "SkrSerde/json/writer.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrCore/log.hpp"
#include "SkrCore/time.h"
#include "SkrRT/ecs/entity.hpp"

BandwidthCounter::BandwidthCounter()
    : dataRecord(30)
{
    skr_init_hires_timer(&timer);
}

void BandwidthCounter::AddRecord(double bytes)
{
    dataRecord.push_back({ skr_hires_timer_get_seconds(&timer, false), bytes });
}

double BandwidthCounter::GetBytePerSecond()
{
    if (dataRecord.size() > 5)
    {
        double totalBytes = 0;
        for (int i = 0; i < dataRecord.size(); ++i)
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

struct ComponentDeltaBuilderRegistry {
    skr::FlatHashMap<sugoi_type_index_t, ComponentDeltaBuilder> builders;

    static ComponentDeltaBuilderRegistry& Get()
    {
        static ComponentDeltaBuilderRegistry instance;
        return instance;
    }
};

struct ComponentDeltaApplierRegistry {
    skr::FlatHashMap<sugoi_type_index_t, ComponentDeltaApplier> appliers;

    static ComponentDeltaApplierRegistry& Get()
    {
        static ComponentDeltaApplierRegistry instance;
        return instance;
    }
};

struct ComponentDeltaBuilder {
    sugoi_type_index_t               component;
    component_delta_build_callback_t callback;
    sugoi_type_index_t               historyComponent = sugoi::kInvalidTypeIndex;
    sugoi_query_t*                   deltaQuery;

    void Initialize(sugoi_storage_t* storage)
    {
        if (historyComponent != sugoi::kInvalidTypeIndex)
        {
            SKR_ASSERT(SUGOI_IS_BUFFER(historyComponent));
            auto                  filter  = make_zeroed<sugoi_filter_t>();
            sugoi_type_index_t    types[] = { component, historyComponent, sugoi_id_of<CAuth>::get() };
            sugoi::type_builder_t all;
            all.with(types, 3);
            filter.all                    = all.build();
            sugoi_parameters_t params     = {};
            sugoi_operation_t  accesses[] = {
                sugoi_operation_t{ -1, 1, 0, 1 },
                sugoi_operation_t{ -1, 0, 0, 1 },
                sugoi_operation_t{ -1, 1, 0, 1 },
            };
            params.types    = types;
            params.accesses = accesses;
            params.length   = 3;
            deltaQuery      = sugoiQ_create(storage, &filter, &params);
        }
        else
        {
            auto                  filter = make_zeroed<sugoi_filter_t>();
            sugoi::type_builder_t all;
            sugoi_type_index_t    types[] = { component, sugoi_id_of<CAuth>::get() };
            all.with(types, 2);
            filter.all                    = all.build();
            sugoi_parameters_t params     = {};
            sugoi_operation_t  accesses[] = {
                sugoi_operation_t{ -1, 1, 0, 1 },
                sugoi_operation_t{ -1, 1, 0, 1 }
            };
            params.types    = types;
            params.accesses = accesses;
            params.length   = 2;
            deltaQuery      = sugoiQ_create(storage, &filter, &params);
        }
    }

    void Release()
    {
        sugoiQ_release(deltaQuery);
    }

    skr::task::event_t GenerateDelta(uint32_t connectionId, uint32_t totalConnection, MPWorldDeltaViewBuilder& builder)
    {
        return callback(component, deltaQuery, { connectionId, totalConnection, historyComponent }, builder);
    }
};

struct WorldDeltaBuilder : IWorldDeltaBuilder {
    skr::Vector<ComponentDeltaBuilder> components;
    skr::Vector<skr::task::event_t>    dependencies;
    sugoi_query_t*                     worldDeltaQuery;
    sugoi_query_t*                     clearDirtyQuery;
    sugoi_query_t*                     deadQuery;
    sugoi_storage_t*                   storage;
    bool                               initialized = false;

    void Initialize(sugoi_storage_t* inStorage) override
    {
        if (initialized)
            return;
        storage                                 = inStorage;
        worldDeltaQuery                         = sugoiQ_from_literal(storage, u8"[in]CPrefab, [inout]CAuth, [inout]sugoi::dirty_comp_t, [inout]CAuthTypeData");
        clearDirtyQuery                         = sugoiQ_from_literal(storage, u8"[inout]sugoi::dirty_comp_t, [has]CPrefab, [has]CAuth, [has]CAuthTypeData");
        deadQuery                               = sugoiQ_from_literal(storage, u8"[in]CAuth, [has]dead");
        ComponentDeltaBuilderRegistry& registry = ComponentDeltaBuilderRegistry::Get();
        for (auto& pair : registry.builders)
        {
            components.add(pair.second);
            components[components.size() - 1].Initialize(storage);
        }
        initialized = true;
    }

    ~WorldDeltaBuilder()
    {
        sugoiQ_release(worldDeltaQuery);
        sugoiQ_release(clearDirtyQuery);
        sugoiQ_release(deadQuery);
        for (auto& component : components)
        {
            component.Release();
        }
    }

    void GenerateDelta(skr::Vector<MPWorldDeltaViewBuilder>& builder) override
    {
        SKR_ASSERT(initialized);
        for (auto& delta : builder)
        {
            delta.changed.clear();
            delta.created.clear();
            delta.dead.clear();
            delta.components.clear();
            delta.entities.clear();
            auto cmps = GetNetworkComponents();
            for (int i = 0; i < cmps.length; ++i)
            {
                delta.components.add_default().ref().type = GetNetworkComponentIndex(cmps.data[i]);
            }
        }
        skr::Vector<skr::FlatHashMap<sugoi_entity_t, NetEntityId>> localMaps;
        localMaps.resize_default(builder.size());
        auto GetNetworkEntityIndex = [&](sugoi_entity_t ent, uint32_t c) -> NetEntityId {
            auto it = localMaps[c].find(ent);
            if (it != localMaps[c].end())
                return it->second;
            NetEntityId index = builder[c].entities.size();
            builder[c].entities.add(ent);
            localMaps[c].insert(std::make_pair(ent, index));
            return index;
        };

        sugoi::type_builder_t history;
        for (auto& component : components)
        {
            if (component.historyComponent != sugoi::kInvalidTypeIndex)
                history.with(component.historyComponent);
        }
        if (history.empty())
            return;
        sugoi_type_set_t historySet = history.build();
        auto             deltaType  = make_zeroed<sugoi_delta_type_t>();
        deltaType.added.type        = historySet;
        sugoi::array_comp_T<sugoi_group_t*, 16> groupToCast;
        auto                                    checkHistoryComponents = [&](sugoi_group_t* group) {
            sugoi_entity_type_t type;
            sugoiG_get_type(group, &type);

            if (!sugoi::set_utils<sugoi_type_index_t>::all(type.type, historySet))
                groupToCast.emplace_back(group);
        };
        sugoiQ_get_groups(worldDeltaQuery, SUGOI_LAMBDA(checkHistoryComponents));
        for (auto group : groupToCast)
        {
            sugoiS_cast_group_delta(storage, group, &deltaType, nullptr, nullptr);
        }
        // find changed component and deleted component
        // prepare changed component storage, record deleted component
        auto prepare = [&builder, &GetNetworkEntityIndex](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
            auto                prefabs      = (CPrefab*)sugoiV_get_owned_rw_local(view, localTypes[0]);
            auto                auths        = (CAuth*)sugoiV_get_owned_rw_local(view, localTypes[1]);
            auto                relevances   = (CRelevance*)sugoiV_get_owned_ro(view, sugoi_id_of<CRelevance>::get());
            auto                dirtyMasks   = (uint32_t*)sugoiV_get_owned_ro_local(view, localTypes[2]);
            auto                authTypes    = (sugoi::array_comp_T<sugoi_type_index_t, 8>*)sugoiV_get_owned_ro_local(view, localTypes[3]);
            auto                ControllerId = sugoi_id_of<CController>::get();
            auto                controller   = (CController*)sugoiV_get_owned_ro(view, ControllerId);
            auto                entities     = sugoiV_get_entities(view);
            sugoi_entity_type_t type;
            sugoiG_get_type(sugoiC_get_group(view->chunk), &type);
            sugoi_type_index_t buffer[128];
            auto               cmps        = GetNetworkComponents();
            sugoi_type_set_t   networkType = sugoi::set_utils<sugoi_type_index_t>::intersect(cmps, type.type, buffer);
            for (int j = 0; j < builder.size(); ++j)
            {
                auto& delta = builder[j];
                for (int i = 0; i < view->count; ++i)
                {
                    bool relevant = !relevances || relevances[i].mask[j];
                    // if not mapped and relevant, create entity
                    if (!auths[i].mappedConnection[j] && relevant)
                    {
                        auths[i].mappedConnection[j]      = true;
                        auths[i].initializedConnection[j] = false;
                        auto& data                        = delta.created.add_default().ref();
                        auto  netEntity                   = GetNetworkEntityIndex(entities[i], j);
                        data.entity                       = netEntity;
                        data.prefab                       = prefabs[i].prefab.get_serialized();
                        data.components.reserve(networkType.length);
                        for (int k = 0; k < networkType.length; ++k)
                        {
                            if (networkType.data[k] == ControllerId)
                                if (j != controller[i].connectionId)
                                    continue;
                            data.components.add(GetNetworkComponentIndex(networkType.data[k]));
                            if (!SUGOI_IS_TAG(networkType.data[k]))
                            {
                                for (auto z = 0; z < delta.components.size(); ++z)
                                {
                                    auto t = GetNetworkComponent(delta.components[z].type);
                                    if (t == networkType.data[k])
                                    {
                                        delta.components[z].entities.add(netEntity);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    // not relevant, delete entity
                    else if (auths[i].mappedConnection[j] && !relevant)
                    {
                        auths[i].mappedConnection[j] = false;
                        delta.dead.add(GetNetworkEntityIndex(entities[i], j));
                    }
                    // relevant, check if changed
                    else if (auths[i].mappedConnection[j] && relevant)
                    {
                        auths[i].initializedConnection[j] = true;
                        sugoi_type_index_t buffer2[128];
                        sugoi_type_index_t buffer3[128];
                        auto               deleted = sugoi::set_utils<sugoi_type_index_t>::substract(auths[i].mappedType, networkType, buffer2);
                        auto               added   = sugoi::set_utils<sugoi_type_index_t>::substract(networkType, auths[i].mappedType, buffer3);

                        if (deleted.length == 0 && dirtyMasks[i] == 0 && added.length == 0)
                            continue;
                        if (deleted.length != 0 || added.length != 0)
                        {
                            MPEntityDeltaViewBuilder& changed   = delta.changed.add_default().ref();
                            auto                      netEntity = GetNetworkEntityIndex(entities[i], j);
                            changed.entity                      = netEntity;
                            changed.components.reserve(added.length);
                            changed.deleted.reserve(deleted.length);
                            for (int k = 0; k < added.length; ++k)
                            {
                                if (added.data[k] == ControllerId)
                                    if (j != controller[i].connectionId)
                                        continue;
                                changed.components.add(GetNetworkComponentIndex(added.data[k]));
                                if (!SUGOI_IS_TAG(added.data[k]))
                                {
                                    for (auto z = 0; z < delta.components.size(); ++z)
                                    {
                                        auto t = GetNetworkComponent(delta.components[z].type);
                                        if (t == added.data[k])
                                        {
                                            delta.components[z].entities.add(netEntity);
                                            break;
                                        }
                                    }
                                }
                            }
                            for (int k = 0; k < deleted.length; ++k)
                                changed.deleted.add(GetNetworkComponentIndex(deleted.data[k]));
                        }
                        for (int k = 0; k < type.type.length; ++k)
                            if ((dirtyMasks[i] & (1 << k)))
                            {
                                if (type.type.data[k] == ControllerId)
                                    if (j != controller[i].connectionId)
                                        continue;
                                if (!SUGOI_IS_TAG(type.type.data[k]))
                                {
                                    for (auto z = 0; z < delta.components.size(); ++z)
                                    {
                                        auto t = GetNetworkComponent(delta.components[z].type);
                                        if (t == type.type.data[k])
                                        {
                                            auto netEntity = GetNetworkEntityIndex(entities[i], j);
                                            delta.components[z].entities.add(netEntity);
                                            break;
                                        }
                                    }
                                }
                            }
                    }
                    authTypes[i].resize(networkType.length);
                    char* temp          = (char*)authTypes[i].data();
                    auths[i].mappedType = sugoi::clone(networkType, temp);
                }
            }
        };
        sugoiJ_schedule_ecs(worldDeltaQuery, 0, SUGOI_LAMBDA(prepare), nullptr, nullptr, nullptr, nullptr);
        skr::task::event_t completed(nullptr);
        sugoi::schedule_task(
        deadQuery, 0, [&](sugoi::task_context_t ctx) {
            auto auths    = ctx.get_owned_ro<CAuth>(0);
            auto entities = ctx.get_entities();
            for (int j = 0; j < builder.size(); ++j)
            {
                auto& delta = builder[j];
                for (int i = 0; i < ctx.count(); ++i)
                {
                    // if mapped, send dead message
                    if (auths[i].mappedConnection[j])
                    {
                        auto netEntity = GetNetworkEntityIndex(entities[i], j);
                        delta.dead.add(netEntity);
                    }
                }
            }
        },
        &completed);
        for (auto& component : components)
        {
            uint32_t i = 0;
            for (auto& delta : builder)
            {
                dependencies.add(component.GenerateDelta(i, builder.size(), delta));
                ++i;
            }
        }
        for (auto& dependency : dependencies)
        {
            dependency.wait(true);
        }
        dependencies.clear();
        sugoiJ_schedule_ecs(
        clearDirtyQuery, 0,
        +[](void* u, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
            auto dirties = (uint32_t*)sugoiV_get_owned_rw_local(view, localTypes[0]);
            for (int i = 0; i < view->count; ++i)
            {
                dirties[i] = 0;
            }
        },
        this, nullptr, nullptr, nullptr, nullptr);
        if (completed)
            completed.wait(true);
        {
            for (auto& delta : builder)
            {
                delta.maxEntity = 0;
                for (auto& entity : delta.entities)
                {
                    EIndex maxId      = std::max(SUGOI_ENTITY_ID(delta.maxEntity), SUGOI_ENTITY_ID(entity.entity));
                    EIndex maxVersion = std::max(SUGOI_ENTITY_VERSION(delta.maxEntity), SUGOI_ENTITY_VERSION(entity.entity));

                    delta.maxEntity = SUGOI_ENTITY(maxId, maxVersion);
                }
            }
        }
        {
            for (auto& delta : builder)
            {
                delta.components.remove_all_if([](auto& c) { return c.data.size() == 0; });
            }
        }
    }
};

struct ComponentDeltaApplier {
    sugoi_type_index_t               component;
    component_delta_apply_callback_t callback;
    sugoi_query_t*                   deltaQuery;
    BandwidthCounter                 bandwidthCounter;

    void Initialize(sugoi_storage_t* storage)
    {
        auto                  filter = make_zeroed<sugoi_filter_t>();
        sugoi::type_builder_t all;
        all.with(component).with<CNetwork>();
        filter.all                    = all.build();
        sugoi_parameters_t params     = {};
        sugoi_type_index_t types[]    = { component, sugoi_id_of<CNetwork>::get() };
        sugoi_operation_t  accesses[] = {
            sugoi_operation_t{ 0, 0, 0, 1 },
            sugoi_operation_t{ -1, 1, 0, 1 },
        };
        params.types    = types;
        params.accesses = accesses;
        params.length   = 2;
        deltaQuery      = sugoiQ_create(storage, &filter, &params);
    }

    void Release()
    {
        sugoiQ_release(deltaQuery);
    }

    skr::task::event_t ApplyDelta(const MPWorldDeltaView& delta, const entity_map_t& entityMap)
    {
        auto iter = std::find_if(delta.components.begin(), delta.components.end(), [&](const MPComponentDeltaView& comp) {
            return GetNetworkComponent(comp.type) == component;
        });
        if (iter == delta.components.end())
        {
            bandwidthCounter.AddRecord(0);
            return skr::task::event_t(nullptr);
        }
        bandwidthCounter.AddRecord(iter->data.size());
        return callback(component, deltaQuery, delta, entityMap);
    }
};

struct WorldDeltaApplier : IWorldDeltaApplier {
    skr::Vector<ComponentDeltaApplier> components;
    skr::Vector<skr::task::event_t>    dependencies;
    sugoi_storage_t*                   storage;
    SpawnPrefab_t                      spawnPrefab;
    DestroyEntity_t                    destroyEntity;
    sugoi_query_t*                     worldDeltaQuery;
    bool                               initialized = false;

    void Initialize(sugoi_storage_t* inStorage, SpawnPrefab_t inSpawnPrefab, DestroyEntity_t inDestroyPrefab) override
    {
        if (initialized)
            return;
        storage                                 = inStorage;
        spawnPrefab                             = std::move(inSpawnPrefab);
        destroyEntity                           = std::move(inDestroyPrefab);
        ComponentDeltaApplierRegistry& registry = ComponentDeltaApplierRegistry::Get();
        for (auto& pair : registry.appliers)
        {
            components.add(pair.second);
            components[components.size() - 1].Initialize(storage);
        }
        worldDeltaQuery = sugoiQ_from_literal(storage, u8"[in]CNetwork");
        initialized     = true;
    }

    ~WorldDeltaApplier() override
    {
        if (!initialized)
            return;
        sugoiQ_release(worldDeltaQuery);
        for (auto& component : components)
        {
            component.Release();
        }
    }

    void ApplyDelta(const MPWorldDeltaView& delta, entity_map_t& map) override
    {
        SkrZoneScopedN("WorldDeltaApplier::ApplyDelta");
        SKR_ASSERT(initialized);
        map.clear();
        {
            SkrZoneScopedN("Build Entity Map");
            auto callback = [&](sugoi_chunk_view_t* view) {
                auto networks = (CNetwork*)sugoiV_get_owned_ro(view, sugoi_id_of<CNetwork>::get());
                auto ents     = sugoiV_get_entities(view);
                for (int i = 0; i < view->count; ++i)
                    map.emplace(networks[i].serverEntity, ents[i]);
            };
            sugoiQ_get_views(worldDeltaQuery, SUGOI_LAMBDA(callback));
        }
        {
            SkrZoneScopedN("Apply Structural Changes");
            for (auto& pair : delta.changed)
            {
                auto iter = map.find(delta.entities[pair.entity]);
                SKR_ASSERT(iter != map.end());
                skr::Vector<sugoi_type_index_t> added;
                skr::Vector<sugoi_type_index_t> removed;
                added.reserve(pair.components.size());
                removed.reserve(pair.deleted.size());
                for (auto& comp : pair.components)
                    added.add(GetNetworkComponent(comp));
                for (auto& comp : pair.deleted)
                    removed.add(GetNetworkComponent(comp));
                sugoi_delta_type_t delta  = make_zeroed<sugoi_delta_type_t>();
                delta.added.type.data     = added.data();
                delta.added.type.length   = added.size();
                delta.removed.type.data   = removed.data();
                delta.removed.type.length = removed.size();
                {
                    sugoi_chunk_view_t view;
                    sugoiS_access(storage, iter->second, &view);
                    sugoiS_cast_view_delta(storage, &view, &delta, nullptr, nullptr);
                }
            }
            for (auto& eid : delta.dead)
            {
                auto entity = delta.entities[eid];
                SKR_LOG_FMT_DEBUG(u8"Entity dead recived {}:{}", sugoi::e_id(entity), sugoi::e_version(entity));

                auto iter = map.find(entity);
                SKR_ASSERT(iter != map.end());
                destroyEntity(storage, iter->second);
                map.erase(iter);
            }
            for (auto& pair : delta.created)
            {
                auto entity = delta.entities[pair.entity];
                SKR_LOG_FMT_DEBUG(u8"New entity recived {}:{}", sugoi::e_id(entity), sugoi::e_version(entity));
                skr::Vector<sugoi_type_index_t> added;
                added.reserve(pair.components.size());
                for (auto& comp : pair.components)
                    added.add(GetNetworkComponent(comp));

                sugoi_entity_type_t type;
                type.type.data   = added.data();
                type.type.length = added.size();
                map.emplace(entity, spawnPrefab(storage, entity, pair.prefab, &type));
            }
        }
        {
            SkrZoneScopedN("Apply Component Changes");
            for (auto& component : components)
            {
                auto event = component.ApplyDelta(delta, map);
                if (event)
                    dependencies.add(std::move(event));
            }
            {
                SkrZoneScopedN("Syncing");
                for (auto& dependency : dependencies)
                {
                    dependency.wait(true);
                }
                dependencies.clear();
            }
        }
    }

    double GetBandwidthOf(sugoi_type_index_t component) override
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

void RegisterComponentDeltaBuilder(sugoi_type_index_t component, component_delta_build_callback_t inCallback, sugoi_type_index_t historyComponent)
{
    ComponentDeltaBuilderRegistry::Get().builders[component] = ComponentDeltaBuilder{ component, inCallback, historyComponent, {} };
}

void RegisterComponentDeltaApplier(sugoi_type_index_t component, component_delta_apply_callback_t inCallback)
{
    ComponentDeltaApplierRegistry::Get().appliers[component] = ComponentDeltaApplier{ component, inCallback, {}, {} };
}
int skr::binary::WriteTrait<packed_entity_t>::Write(SBinaryWriter* writer, const packed_entity_t& value, sugoi_entity_t maxEntity)
{
    uint32_t id         = SUGOI_ENTITY_ID(value.entity);
    uint32_t version    = SUGOI_ENTITY_VERSION(value.entity);
    uint32_t idMax      = SUGOI_ENTITY_ID(maxEntity);
    uint32_t versionMax = SUGOI_ENTITY_VERSION(maxEntity);

    auto ret = Archive(writer, id, IntegerPackConfig<uint32_t>{ 0, idMax });
    if (ret != 0)
        return ret;
    ret = Archive(writer, version, IntegerPackConfig<uint32_t>{ 0, versionMax });
    return ret;
}
int skr::binary::ReadTrait<packed_entity_t>::Read(SBinaryReader* reader, packed_entity_t& value, sugoi_entity_t maxEntity)
{
    uint32_t id         = 0;
    uint32_t version    = 0;
    uint32_t idMax      = SUGOI_ENTITY_ID(maxEntity);
    uint32_t versionMax = SUGOI_ENTITY_VERSION(maxEntity);
    auto     ret        = Archive(reader, id, IntegerPackConfig<uint32_t>{ 0, idMax });
    if (ret != 0)
        return ret;
    ret = Archive(reader, version, IntegerPackConfig<uint32_t>{ 0, versionMax });
    if (ret != 0)
        return ret;
    value.entity = SUGOI_ENTITY(id, version);
    return 0;
}

void skr::json::WriteTrait<packed_entity_t>::Write(skr::json::Writer* writer, const packed_entity_t& value)
{
    writer->StartObject();
    writer->Key(u8"id");
    writer->UInt(SUGOI_ENTITY_ID(value.entity));
    writer->Key(u8"version");
    writer->UInt(SUGOI_ENTITY_VERSION(value.entity));
    writer->EndObject();
}