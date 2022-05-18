#pragma once
#include "ecs/dual.h"
#if !defined(__meta__) && defined(__cplusplus)
    #include "GameRT/scene.dual.generated.hpp"
#endif

struct reflect attr(
    "guid" : "EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A"
)
game_scene_t
{
    dual_storage_t* world;
};
typedef struct game_scene_t game_scene_t;

struct reflect attr(
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

struct reflect attr(
    "guid" : "2CAA41D2-54A4-46FB-BE43-68B545F313BF",
    "component" : true
)
skr_parent_t
{
    dual_entity_t entity;
};
typedef struct skr_parent_t skr_parent_t;

struct reflect attr(
    "guid" : "1CD632F6-3149-42E6-9114-647B0C803F32",
    "component" : true
)
skr_name_t
{
    char str[32];
};
typedef struct skr_name_t skr_name_t;