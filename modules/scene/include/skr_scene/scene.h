#pragma once
#include "SkrScene/skr_scene.configure.h"
#include "ecs/dual.h"
#if !defined(__meta__) && defined(__cplusplus)
    #include "SkrScene/scene.dual.generated.hpp"
#endif

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