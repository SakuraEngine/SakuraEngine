#pragma once
#include "ecs/dual.h"

struct reflect attr(
    "guid" : "EFBA637E-E7E5-4B64-BA26-90AEEE9E3E1A"
)
game_scene_t
{
    dual_storage_t* world;
};
typedef struct game_scene_t game_scene_t;