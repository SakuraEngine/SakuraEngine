#pragma once
#include "steam/steamnetworkingtypes.h"
#include "SkrBase/config.h"
#include "MPShared/shared.h"
#include "MPShared/world_delta.h"
#include "SkrScene/scene.h"
#include "SkrCore/time.h"
#include "SkrContainers/vector.hpp"

struct MP_SHARED_API MPServerWorld : MPGameWorld
{
    MPServerWorld();
    ~MPServerWorld();
    skr::Vector<HSteamNetConnection> connections;
    skr::Vector<MPWorldDeltaViewBuilder> worldDelta;
    uint64_t gameFrame;
    MPInputFrame queuedInputs[128];
    skr::Vector<std::vector<uint32_t>> playerMap;
    SHiresTimer timer;
    double lastGameTime;
    int playerId = 0;
    IWorldDeltaBuilder* worldDeltaBuilder;
    sugoi_query_t* deadQuery;

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
    skr::Vector<uint8_t> SerializeWorldDelta(const MPWorldDeltaViewBuilder& deltaBuilder);
    void AccumulateInput(uint32_t connectionId, const MPInputFrame& inputs);

    template<class T, class F>
    void GenerateDelta(sugoi_type_index_t type, sugoi_query_t* query, F&& generator);
};