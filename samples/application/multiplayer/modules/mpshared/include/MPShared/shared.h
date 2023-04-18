#pragma once
#include "MPShared/module.configure.h"
#include "ecs/dual.h"
#include "steam/steamnetworkingtypes.h"
#include "containers/vector.hpp"
#include "SkrScene/scene.h"
#include "components.h"
#ifndef __meta__
    #include "MPShared/shared.generated.h"
#endif

static constexpr double serverTickInterval = 1.0 / 30;

enum class MPEventType : uint32_t
{
    SyncWorld,
    Input,
};

sreflect_struct(
    "guid" : "913574AC-7DD2-47A9-82A6-08D4350FCBE7",
    "serialize" : "bin"
)
MPPlayerInputFrame
{
    skr_float2_t move;
    bool fire;
    bool skill;
};

sreflect_struct(
    "guid" : "31F78AAE-6EEA-46EB-8F17-6FA2E0516DB9",
    "serialize" : "bin"
) 
MPInputFrame
{
    uint64_t frame;
    skr::vector<MPPlayerInputFrame> inputs;
};

sreflect_struct(
    "guid" : "407B6F08-E76C-40E5-8EDC-A5AA808A0D0F",
    "query" : "[in]CMovement, [inout]skr_translation_comp_t', [inout]skr_rotation_comp_t, [atomic]?dual::dirty_comp_t"
)
QMovement
{
    GENERATED_QUERY_BODY(QMovement);
};

struct MP_SHARED_API MPGameWorld
{
    MPGameWorld() = default;
    MPGameWorld(const MPGameWorld&) = delete;
    MPGameWorld& operator=(const MPGameWorld&) = delete;
    virtual ~MPGameWorld() = default;
    dual_storage_t* storage;
    dual_query_t* controlQuery;
    dual_query_t* healthCheckQuery;
    dual_query_t* relevanceQuery;
    dual_query_t* relevanceChildQuery;
    dual_query_t* fireQuery;
    QMovement movementQuery;
    dual_query_t* ballQuery;
    dual_query_t* ballChildQuery;
    dual_query_t* killQuery;
    skr_transform_system_t transformSystem;
    MPInputFrame input;
    bool authoritative;
    virtual void Initialize();
    virtual void Shutdown();
    void Tick(const MPInputFrame& input);
};
