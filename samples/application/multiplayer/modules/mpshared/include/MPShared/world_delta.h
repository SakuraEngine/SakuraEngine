#pragma once
#include "MPShared/module.configure.h"
#include "containers/span.hpp"
#include "utils/types.h"
#include "ecs/dual.h"
#include "platform/guid.hpp"
#include "task/task.hpp"
#include "containers/hashmap.hpp"
#include "containers/vector.hpp"
#include "EASTL/functional.h"
#include "EASTL/bonus/fixed_ring_buffer.h"
#include "platform/time.h"
#ifndef __meta__
    #include "MPShared/world_delta.generated.h"
#endif

using NetEntityId = uint16_t;
using NetComponentId = uint8_t;

sreflect_struct("guid": "8F20F35F-38D6-4D2F-AB41-24E582F2378B") 
sattr("blob" : true)
sattr("debug" : true)
MPEntityCreationView
{
    NetEntityId entity;
    skr::span<NetComponentId> components;
    skr_guid_t prefab;
};

GENERATED_BLOB_BUILDER(MPEntityCreationView)

sreflect_struct("guid": "6DEDEB26-1A03-4194-95EF-0E589366C985") 
sattr("blob" : true)
sattr("debug" : true)
MPEntityDeltaView
{
    NetEntityId entity;
    skr::span<NetComponentId> components;
    skr::span<NetComponentId> deleted;
};
GENERATED_BLOB_BUILDER(MPEntityDeltaView)

sreflect_struct("guid": "B2A257E7-E153-485E-A4B5-5D0B2EC65E42")  
sattr("blob" : true)
sattr("debug" : true)
MPComponentDeltaView
{
    NetComponentId type;
    skr::span<NetEntityId> entities;
    sattr("no-text" : true)
    skr::span<uint8_t> data;
};
GENERATED_BLOB_BUILDER(MPComponentDeltaView)

sreflect_struct("guid": "0E7D9309-13EF-4EB8-9E8E-2DDE8D8F7BA0") 
sattr("blob" : true)
sattr("debug" : true)
MPWorldDeltaView
{
    skr::span<dual_entity_t> entities; 
    skr::span<MPComponentDeltaView> components;
    skr::span<MPEntityCreationView> created;
    skr::span<MPEntityDeltaView> changed;
    skr::span<NetEntityId> dead;
};
GENERATED_BLOB_BUILDER(MPWorldDeltaView)

sreflect_struct("guid": "54C41BFD-130C-4AF6-AD50-F43D5C6E8AE5")
sattr("serialize" : "bin")
MPWorldDelta
{
    uint64_t frame;
    double time;
    skr_blob_arena_t arena; 
    spush_attr("arena" : "arena")
    MPWorldDeltaView blob;
}; 

struct MPWorldDeltaBuildContext
{
    uint32_t connectionId;
    uint32_t totalConnections;
    dual_type_index_t historyComponent;
};

using entity_map_t = skr::flat_hash_map<dual_entity_t, dual_entity_t>;

using component_delta_build_callback_t = skr::task::event_t (*)(dual_type_index_t type, dual_query_t* query, MPWorldDeltaBuildContext ctx, MPWorldDeltaViewBuilder& builder);
using component_delta_apply_callback_t = skr::task::event_t (*)(dual_type_index_t type, dual_query_t* query, const MPWorldDeltaView& delta, const entity_map_t& map);

struct IWorldDeltaBuilder
{
    virtual ~IWorldDeltaBuilder() = default;
    virtual void Initialize(dual_storage_t* storage) = 0;
    virtual void GenerateDelta(skr::vector<MPWorldDeltaViewBuilder>& builder) = 0;
};

MP_SHARED_API IWorldDeltaBuilder* CreateWorldDeltaBuilder();
void RegisterComponentDeltaBuilder(dual_type_index_t component, component_delta_build_callback_t inCallback,  dual_type_index_t historyComponent = dual::kInvalidTypeIndex);

struct IWorldDeltaApplier
{
    using SpawnPrefab_t = eastl::function<dual_entity_t(dual_storage_t*, dual_entity_t entity, skr_guid_t prefab, dual_entity_type_t* type)>;
    using DestroyEntity_t = eastl::function<void(dual_storage_t*, dual_entity_t entity)>;
    virtual ~IWorldDeltaApplier() = default;
    virtual void Initialize(dual_storage_t* storage, SpawnPrefab_t spawnPrefab, DestroyEntity_t destroyPrefab) = 0;
    virtual void ApplyDelta(const MPWorldDeltaView& delta, entity_map_t& map) = 0;
    virtual double GetBandwidthOf(dual_type_index_t component) = 0;
};

MP_SHARED_API IWorldDeltaApplier* CreateWorldDeltaApplier();
void RegisterComponentDeltaApplier(dual_type_index_t component, component_delta_apply_callback_t inCallback);


struct BandwidthCounter
{
    BandwidthCounter();
    eastl::fixed_ring_buffer<std::pair<double, uint64_t>, 30> dataRecord;
    SHiresTimer timer;
    void AddRecord(uint64_t bytes);
    double GetBytePerSecond();
};