#pragma once
#include "SkrScene/skr_scene.configure.h"
#include "utils/types.h"
#include "ecs/dual.h"
#if !defined(__meta__) && defined(__cplusplus)
    #include "SkrScene/scene.dual.generated.hpp"
#endif

// scene hierarchy

struct sreflect sattr(
    "guid" : "EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A"
)
game_scene_t
{
    dual_storage_t* world;
};
typedef struct game_scene_t game_scene_t;

struct sreflect sattr(
    "guid" : "82CDDC11-3D94-4552-8FD4-237A053F35C0",
    "component" : 
    {
        "buffer" : 4
    }
) skr_child_t
{
    dual_entity_t entity;
};
typedef struct skr_child_t skr_child_t;

#ifdef __cplusplus
#include "ecs/array.hpp"
using skr_children_t = dual::array_component_T<skr_child_t, 4>;
#endif

struct sreflect sattr(
    "guid" : "2CAA41D2-54A4-46FB-BE43-68B545F313BF",
    "component" : true
)
skr_parent_t
{
    dual_entity_t entity;
};
typedef struct skr_parent_t skr_parent_t;

struct sreflect sattr(
    "guid" : "1CD632F6-3149-42E6-9114-647B0C803F32",
    "component" : true
)
skr_name_t
{
    char str[32];
};
typedef struct skr_name_t skr_name_t;

// transforms

struct sreflect sattr(
    "guid" : "AE2C7477-8A44-4339-BE5D-64D05D7E05B1",
    "component" : true //, "serialize" : "USD"
)
SKR_ALIGNAS(16) skr_l2w_t
{
    skr_float4x4_t matrix;
};

struct sreflect sattr(
    "guid" : "869F46D3-992A-4C18-9538-BDC48F4BED1D",
    "component" : true
)
SKR_ALIGNAS(16) skr_l2r_t
{
    skr_float4x4_t matrix;
};

struct sreflect sattr(
    "guid" : "78DD218B-87DE-4250-A7E8-A6B4553B47BF",
    "component" : true
)
skr_rotation_t
{
    skr_rotator_t euler;
};

struct sreflect sattr(
    "guid" : "A059A2A1-CC3B-43B0-88B6-ADA7822BA25D",
    "component" : true
)
skr_translation_t
{
    skr_float3_t value;
};

struct sreflect sattr(
    "guid" : "D045D755-FBD1-44C2-8BF0-C86F2D8485FF",
    "component" : true
)
skr_scale_t
{
    skr_float3_t value;
};

struct sreflect sattr(
    "guid" : "4fa24729-2c66-45a2-9417-3497ebc18771",
    "component" : true
)
skr_movement_t
{
    skr_float3_t value;
};

struct sreflect sattr(
    "guid" : "d33c74c5-2763-4ba4-b58e-dc44a627ebf4",
    "component" : true
)
skr_camera_t
{
    uint32_t viewport_width;
    uint32_t viewport_height;
};



struct skr_transform_system {
    dual_query_t* localToWorld;
    dual_query_t* localToRelative;
    dual_query_t* relativeToWorld;
};

SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_setup(dual_storage_t* world, skr_transform_system* system);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_update(skr_transform_system* query);