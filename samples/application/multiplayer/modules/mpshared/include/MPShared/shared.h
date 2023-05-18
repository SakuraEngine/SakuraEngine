#pragma once
#include "MPShared/module.configure.h"
#include "ecs/dual.h"
#include "steam/steamnetworkingtypes.h"
#include "containers/vector.hpp"
#include "SkrScene/scene.h"
#include "components.h"
#include "utils/traits.hpp"
#include "platform/guid.hpp"
#include "containers/hashmap.hpp"
#include "EASTL/fixed_vector.h"
#ifndef __meta__
    #include "MPShared/shared.generated.h"
#endif

static constexpr double serverTickInterval = 1.0 / 30;
static constexpr uint32_t syncRate = 3;

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

sreflect_struct(
    "guid" : "305FB0DC-85F6-4B70-952F-BE662150E506",
    "query" : "[in]CController, [inout]CMovement, [inout]CSkill, [inout]CPlayer, [atomic]?dual::dirty_comp_t"
)
QControl
{
    GENERATED_QUERY_BODY(QControl);
};

sreflect_struct(
    "guid" : "E4D46AA5-7D4D-4864-8241-7260F7D4837F",
    "query" : "[inout]CHealth, [inout]skr_translation_comp_t, [inout]skr_rotation_comp_t, [inout]CWeapon, [atomic]?dual::dirty_comp_t"
)
QHeathCheck
{
    GENERATED_QUERY_BODY(QHeathCheck);
};

sreflect_struct(
    "guid" : "ED18D917-71A4-4D51-B805-84BA73F67AD2",
    "query" : "[in]CController, [in]skr_translation_comp_t, [in]skr_rotation_comp_t, [inout]CWeapon, [atomic]?dual::dirty_comp_t"
)
QFireBullet
{
    GENERATED_QUERY_BODY(QFireBullet);
};

sreflect_struct(
    "guid" : "CB8DA6C6-4D7E-4066-A867-A37418EAEC1E",
    "query" : "[inout]skr_translation_comp_t', [in]CSphereCollider2D, [inout]CMovement, [atomic]?dual::dirty_comp_t, [inout]CBall, [has]skr_rotation_comp_t"
)
QBallMovement
{
    GENERATED_QUERY_BODY(QBallMovement);
};

sreflect_struct(
    "guid" : "67DB0A0E-4ACB-483F-98EE-DE3D062F1381",
    "query" : "[inout]CRelevance, [in]skr_translation_comp_t"
)
QUpdateRelevance
{
    GENERATED_QUERY_BODY(QUpdateRelevance);
};

sreflect_struct(
    "guid" : "30537121-0322-4D9C-95E8-2C975EEDE1A4",
    "query" : "[inout]CBall"
)
QKillBall
{
    GENERATED_QUERY_BODY(QKillBall);
};

sreflect_struct(
    "guid" : "A4A25B43-7C4C-4A60-9BA6-0E6E5C8793A0",
    "query" : "[inout]CZombie, [inout]CHealth"
)
QKillZombie
{
    GENERATED_QUERY_BODY(QKillZombie);
};

sreflect_struct(
    "guid" : "A4A25B43-7C4C-4A60-9BA6-0E6E5C8793A0",
    "query" : "[in]skr_translation_comp_t, [inout]CMovement, [atomic]?dual::dirty_comp_t, [inout]CZombie, [has]skr_rotation_comp_t"
)
QZombieAI
{
    GENERATED_QUERY_BODY(QZombieAI);
};

sreflect_struct("guid" : "8B5C7563-EA37-4FA9-BF73-9E353EC99A03")
QCollision
{
    GENERATED_QUERY_BODY(QCollision);
} 
sattr("query" : "[in]skr_translation_comp_t, [in]CSphereCollider2D, [inout]CCollisionScene");


inline constexpr skr_guid_t GetPlayerPrefab()
{
    return skr::guid::make_guid_unsafe(u8"AC4BA94B-B2A8-484C-9AC5-BDEA9070DFEE");
}

inline constexpr skr_guid_t GetZombiePrefab()
{
    return skr::guid::make_guid_unsafe(u8"9DFEBC41-4731-4AAE-9618-8BA4CC0EF86C");
}

inline constexpr skr_guid_t GetBulletPrefab()
{
    return skr::guid::make_guid_unsafe(u8"8698AA92-F3E3-4DDA-B0B9-59D004538988");
}


sreflect_struct(
    "guid" : "E8B8B447-4BBE-4A1E-A29E-FD28F046864E",
    "component" :
    {
        "custom" : "::dual::managed_component"
    }
)
CCollisionScene
{
    struct CollisionEntity
    {  
        dual_entity_t entity;
        CSphereCollider2D collider;
    };
    skr::parallel_flat_hash_map<int, eastl::fixed_vector<CollisionEntity, 4>> cells;
};

struct MP_SHARED_API MPGameWorld
{
    MPGameWorld() = default;
    MPGameWorld(const MPGameWorld&) = delete;
    MPGameWorld& operator=(const MPGameWorld&) = delete;
    virtual ~MPGameWorld() = default;
    dual_storage_t* storage;
    QControl controlQuery;
    QHeathCheck healthCheckQuery;
    QMovement movementQuery;
    QFireBullet fireQuery;
    QBallMovement ballQuery;
    QUpdateRelevance relevanceQuery;
    QKillBall killBallQuery;
    QKillZombie killZombieQuery;
    QZombieAI zombieAIQuery;
    dual_query_t* relevanceChildQuery;
    dual_query_t* ballChildQuery;
    dual_query_t* zombieAIChildQuery;
    dual_query_t* gameStateQuery;
    skr_transform_system_t transformSystem;
    MPInputFrame input;
    MPGameModeConfig config;
    bool authoritative;
    virtual void Initialize();
    virtual void Shutdown();
    void Tick(const MPInputFrame& input);
    void SetupCollsionWorld();
    void ClearDeadBall();
    void ClearDeadZombie();
    void SpawnZombie();
    void PlayerControl();
    void ZombieAI();
    void PlayerShoot();
    void PlayerHealthCheck();
    void PlayerMovement();
    void BulletMovement();
    void RelevenceUpdate();
};
