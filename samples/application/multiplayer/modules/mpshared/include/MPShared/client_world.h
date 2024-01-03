
#pragma once
#include "steam/steamnetworkingtypes.h"
#include "SkrRT/platform/time.h"
#include "MPShared/shared.h"
#include "MPShared/world_delta.h"

struct Prefab
{
    sugoi_entity_type_t (*Type)();
    void (*Initialize)();
};

constexpr static size_t MaxPredictFrame = 512;
struct MP_SHARED_API MPClientWorld : MPGameWorld
{
    MPClientWorld();
    ~MPClientWorld();
    HSteamNetConnection serverConnection;
    uint64_t currentFrame;
    uint64_t predictedFrame;
    uint64_t verifiedFrame;
    uint64_t inputFrame;
    MPInputFrame predictedInputs[MaxPredictFrame];
    MPInputFrame input;
    sugoi_storage_t* snapshotStorage;
    sugoi_query_t* snapshotQuery;
    sugoi_query_t* healthQuery;
    IWorldDeltaApplier* worldDeltaApplier;

    bool firstFrame = true;
    bool predictionEnabled = true;
    double currentGameTime;
    double predictedGameTime;
    double lastTime;
    double timeScale = 1.0;
    MPWorldDelta worldDelta;
    SHiresTimer timer;
    BandwidthCounter bandwidthCounter;
    BandwidthCounter bandwidthBeforeCompressCounter;
    double compressRatio;
    
    virtual void Initialize() override;
    void SendInput();
    void ReceiveWorldDelta(const void* data, size_t dataLength);
    void ApplyWorldDelta();
    bool Update();
    void Shutdown() override;
    void Snapshot();
    void RollBack();
    void RollForward();
    void SkipFrame();
    void SetPredictionEnabled(bool enabled);
    void UpdateTimeScale(double deltaTime);
    double GetBytePerSecond();
    double GetBytePerSecondBeforeCompress();
    double GetCompressRatio();
    double GetTickInterval() { return serverTickInterval; }
    sugoi_entity_t SpawnPrefab(skr_resource_handle_t prefab, sugoi_entity_t entity, sugoi_entity_type_t extension);
    void DestroyEntity(sugoi_entity_t entity);


    float GetPlayerHealth();
};