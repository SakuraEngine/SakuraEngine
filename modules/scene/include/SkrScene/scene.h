#pragma once
#include "SkrScene/module.configure.h"
#include "misc/types.h"
#include "ecs/dual_types.h"
#ifndef __meta__
    #include "SkrScene/scene.generated.h"
#endif

// scene hierarchy

sreflect_struct(
    "guid" : "82CDDC11-3D94-4552-8FD4-237A053F35C0",
    "component" : 
    {
        "buffer" : 4
    }
) skr_child_comp_t
{
    dual_entity_t entity;
};
typedef struct skr_child_comp_t skr_child_comp_t;

#ifdef __cplusplus
using skr_children_t = dual::array_comp_T<skr_child_comp_t, 4>;
#endif

sreflect_struct(
    "guid" : "2CAA41D2-54A4-46FB-BE43-68B545F313BF",
    "component" : true
)
skr_parent_comp_t
{
    dual_entity_t entity;
};
typedef struct skr_parent_comp_t skr_parent_comp_t;

#define SKR_SCENE_MAX_NAME_LENGTH 32

sreflect_struct(
    "guid" : "1CD632F6-3149-42E6-9114-647B0C803F32",
    "component" : true
)
skr_name_comp_t
{
    char str[SKR_SCENE_MAX_NAME_LENGTH + 1];
};
typedef struct skr_name_comp_t skr_name_comp_t;

sreflect_struct(
    "guid" : "b08ec011-9b94-47f7-9926-c66d41d735e9",
    "component" : true
)
skr_index_comp_t
{
    uint32_t value;
};
typedef struct skr_index_comp_t skr_index_comp_t;

// transforms

sreflect_struct(
    "guid" : "AE2C7477-8A44-4339-BE5D-64D05D7E05B1",
    "component" : true
)
SKR_ALIGNAS(16) skr_transform_comp_t
{
    skr_transform_t value;
};

sreflect_struct(
    "guid" : "78DD218B-87DE-4250-A7E8-A6B4553B47BF", 
    "component" : true, 
    "serialize" : ["bin", "json"],
    "rtti" : true
)
skr_rotation_comp_t
{
    skr_rotator_t euler;
};

struct sreflect sattr(
    "guid" : "A059A2A1-CC3B-43B0-88B6-ADA7822BA25D",
    "component" : true, 
    "serialize" : ["bin", "json"],
    "rtti" : true
)
skr_translation_comp_t
{
    skr_float3_t value;
};

sreflect_struct(
    "guid" : "D045D755-FBD1-44C2-8BF0-C86F2D8485FF", 
    "component" : true, 
    "serialize" : ["bin", "json"],
    "rtti" : true)
skr_scale_comp_t
{
    skr_float3_t value;
};

sreflect_struct("guid" : "4fa24729-2c66-45a2-9417-3497ebc18771", "component" : true)
skr_movement_comp_t
{
    skr_float3_t value;
};

sreflect_struct("guid" : "d33c74c5-2763-4ba4-b58e-dc44a627ebf4", "component" : true)
skr_camera_comp_t
{
    struct SRenderer* renderer;
    uint32_t viewport_id;
    uint32_t viewport_width;
    uint32_t viewport_height;
};

struct skr_transform_system_t {
    dual_query_t* relativeToWorld;
};

SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_setup(dual_storage_t* world, skr_transform_system_t* system);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_update(skr_transform_system_t* query);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_propagate_transform(dual_storage_t* world, dual_entity_t* entities, uint32_t count);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_save_scene(dual_storage_t* world, struct skr_json_writer_t* writer);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_load_scene(dual_storage_t* world, struct skr_json_reader_t* reader);

#ifdef __cplusplus
#include "lua/bind.hpp"

namespace skr::lua
{
    SKR_SCENE_API int push_name_comp(lua_State* L, const skr_name_comp_t& value);
    SKR_SCENE_API skr_name_comp_t check_name_comp(lua_State* L, int index);
    template<>
    struct BindTrait<skr_name_comp_t>
    {
        static int push(lua_State* L, const skr_name_comp_t& value)
        {
            return push_name_comp(L, value);
        }

        static skr_name_comp_t check(lua_State* L, int index)
        {
            return check_name_comp(L, index);
        }
    };
    template<>
    struct BindTrait<skr_child_comp_t>
    {
        static int push(lua_State* L, const skr_child_comp_t& value)
        {
            lua_pushinteger(L, value.entity);
            return 1;
        }

        static skr_child_comp_t check(lua_State* L, int index)
        {
            skr_child_comp_t result;
            result.entity = static_cast<dual_entity_t>(luaL_checkinteger(L, index));
            return result;
        }
    };
}

#endif