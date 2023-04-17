#pragma once
#include "MPShared/module.configure.h"
#include "ecs/dual.h"
#include "containers/bitset.h"
#include "resource/resource_handle.h"
#ifndef __meta__
#include "MPShared/components.generated.h"
#endif

sreflect_struct(
    "guid" : "611902BB-0918-48F4-AE50-A4376C8F9A56",
    "serialize" : "bin",
    "component" : true
) 
CNetwork
{
    dual_entity_t serverEntity;
};

sreflect_struct(
    "guid" : "727FDAD5-6882-4360-8A16-77F07CAEF2AE",
    "serialize" : "bin",
    "component" : true
)
CPrefab
{
    skr_resource_handle_t prefab;
};

sreflect_struct(
    "guid" : "56F23FB1-E3FC-4A17-83B8-502A21EA2E2A",
    "serialize" : "bin",
    "component": true
)
CPlayer
{
    double sightRadius;
};

sreflect_struct(
    "guid" : "2907B89B-A3D7-4750-A2B6-CD326483BF6C",
    "serialize" : "bin",
    "component" : true
)
CGhost
{
    dual_entity_t mappedEntity;
};

sreflect_struct(
    "guid" : "5A9B099D-4A7F-4053-97A0-A0B1AF400339",
    "component" : 
    {
        "pin" : true
    }
)
CAuth
{
    skr::bitset<128> mappedConnection;
    skr::bitset<128> initializedConnection;
    dual_type_set_t mappedType; //data store in CAuthType buffer component
};

sreflect_struct(
    "guid" : "4027DE2C-A026-42AB-8BD2-06B29293DAB6",
    "component" : 
    {
        "buffer" : 8
    }
)
CAuthTypeData
{
    dual_type_index_t type;
};

sreflect_struct(
    "guid" : "BCBFD4DC-96F9-498E-B1FC-61A39CE8689B",
    "serialize" : "bin",
    "component" : true
)
CRelevance
{
    skr::bitset<128> mask;
};

sreflect_struct(
    "guid" : "34BCF135-691A-41CD-B531-8A258AB35B39",
    "serialize" : "bin",
    "component" : true
)
//接收输入的组件
CController
{
    int connectionId;
    int playerId;
    int serverPlayerId;
    int localPlayerId;
};

sreflect_struct(
    "guid" : "CA09FC13-3299-40D8-8F23-D038DA4403F0",
    "serialize" : "bin",
    "component" : true
)
CMovement
{
    float speed;
    float baseSpeed;
    skr_float2_t velocity;
};

sreflect_struct(
    "guid" : "8A148CAD-46BD-4895-9115-2CF05D2CD64B",
    "serialize" : "bin",
    "component" : true
)
CWeapon
{
    float fireRate;
    float fireTimer;
    //skr_resource_handle_t projectilePrefab;
};

sreflect_struct(
    "guid" : "C3A2C8CB-E1C7-405D-B6F9-006D8D2B46EC",
    "serialize" : "bin",
    "component" : true
)
CSkill
{
    float cooldown;
    float cooldownTimer;
    float duration;
    float durationTimer;
    float speedMultiplier;
}; 

sreflect_struct(
    "guid" : "36383FB4-8941-435F-B5FF-1F11438104EF",
    "serialize" : "bin",
    "component" : true
)
CHealth 
{
    float maxHealth;
    float health;
};

sreflect_struct(
    "guid" : "8B23F272-E6D5-4556-BD41-CF642F4BFC99",
    "serialize" : "bin",
    "component" : true
)
CSnapshot
{
    dual_entity_t owner;
};

sreflect_struct(
    "guid" : "7F90EF5D-2174-405A-8A38-9A42713F31A0",
    "serialize" : "bin",
    "component" : true
)
CSphereCollider2D
{
    float radius;
};

sreflect_struct(
    "guid" : "824C95F6-C8D6-49B0-AB2D-2493802D2DE3",
    "serialize" : "bin",
    "component" : true
)
CBall
{
    float lifeTime;
    int playerId;
};

MP_SHARED_API dual_type_set_t GetNetworkComponents();
MP_SHARED_API dual_type_index_t GetNetworkComponent(uint8_t index);
MP_SHARED_API uint8_t GetNetworkComponentIndex(dual_type_index_t type);
MP_SHARED_API void InitializeNetworkComponents();