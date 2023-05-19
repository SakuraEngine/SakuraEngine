#pragma once
#include "MPShared/module.configure.h"
#include "MPShared/shared.h"
#include "MPShared/world_delta.h"
#include "SkrScene/scene.h"
#include "containers/hashmap.hpp"
#include "async/fib_task.hpp"
#include "platform/time.h"
#include "containers/vector.hpp"
#include "ecs/entities.hpp"

struct MP_SHARED_API MPServerWorld : MPGameWorld
{
    MPServerWorld();
    ~MPServerWorld();
    skr::vector<HSteamNetConnection> connections;
    skr::vector<MPWorldDeltaViewBuilder> worldDelta;
    uint64_t gameFrame;
    MPInputFrame queuedInputs[128];
    skr::vector<std::vector<uint32_t>> playerMap;
    SHiresTimer timer;
    double lastGameTime;
    int playerId = 0;
    IWorldDeltaBuilder* worldDeltaBuilder;
    dual_query_t* deadQuery;

    void AddConnection(HSteamNetConnection connection);
    void RemoveConnection(HSteamNetConnection connection);
    int GetPlayerConnection(int player, int* localId = nullptr);
    void SpawnGameModeEntity();
    void SpawnPlayerEntity(int player, int connectionId, int localPlayerId);
    void Initialize() override;
    void InitializeScene();
    void Update();
    void Shutdown() override;
    void GenerateWorldDelta();
    void SendWorldDelta();
    void LogNetworkStatics();
    skr::vector<uint8_t> SerializeWorldDelta(const MPWorldDeltaViewBuilder& deltaBuilder);
    void AccumulateInput(uint32_t connectionId, const MPInputFrame& inputs);

    template<class T, class F>
    void GenerateDelta(dual_type_index_t type, dual_query_t* query, F&& generator);
};