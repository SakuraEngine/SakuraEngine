#pragma once
#include "SkrScene/module.configure.h"
#include "utils/types.h"
#include "ecs/dual_types.h"
#ifndef __meta__
    #include "SkrScene/scene.generated.h"
#endif

// scene hierarchy

struct sreflect sattr(
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

struct sreflect sattr(
    "guid" : "2CAA41D2-54A4-46FB-BE43-68B545F313BF",
    "component" : true
)
skr_parent_comp_t
{
    dual_entity_t entity;
};
typedef struct skr_parent_comp_t skr_parent_comp_t;

#define SKR_SCENE_MAX_NAME_LENGTH 32

struct sreflect sattr(
    "guid" : "1CD632F6-3149-42E6-9114-647B0C803F32",
    "component" : true
)
skr_name_comp_t
{
    char str[SKR_SCENE_MAX_NAME_LENGTH + 1];
};
typedef struct skr_name_comp_t skr_name_comp_t;

struct sreflect sattr(
    "guid" : "b08ec011-9b94-47f7-9926-c66d41d735e9",
    "component" : true
)
skr_index_comp_t
{
    uint32_t value;
};
typedef struct skr_index_comp_t skr_index_comp_t;

// transforms

struct sreflect sattr(
    "guid" : "AE2C7477-8A44-4339-BE5D-64D05D7E05B1",
    "component" : true //, "serialize" : "USD"
)
SKR_ALIGNAS(16) skr_l2w_comp_t
{
    skr_float4x4_t matrix;
};

struct sreflect sattr(
    "guid" : "869F46D3-992A-4C18-9538-BDC48F4BED1D",
    "component" : true
)
SKR_ALIGNAS(16) skr_l2r_comp_t
{
    skr_float4x4_t matrix;
};

struct sreflect sattr(
    "guid" : "78DD218B-87DE-4250-A7E8-A6B4553B47BF",
    "component" : true
)
skr_rotation_comp_t
{
    skr_rotator_t euler;
};

struct sreflect sattr(
    "guid" : "A059A2A1-CC3B-43B0-88B6-ADA7822BA25D",
    "component" : true
)
skr_translation_comp_t
{
    skr_float3_t value;
};

struct sreflect sattr(
    "guid" : "D045D755-FBD1-44C2-8BF0-C86F2D8485FF",
    "component" : true
)
skr_scale_comp_t
{
    skr_float3_t value;
};

struct sreflect sattr(
    "guid" : "4fa24729-2c66-45a2-9417-3497ebc18771",
    "component" : true
)
skr_movement_comp_t
{
    skr_float3_t value;
};

struct sreflect sattr(
    "guid" : "d33c74c5-2763-4ba4-b58e-dc44a627ebf4",
    "component" : true
)
skr_camera_comp_t
{
    uint32_t viewport_width;
    uint32_t viewport_height;
};

struct skr_transform_system_t {
    dual_query_t* localToWorld;
    dual_query_t* localToRelative;
    dual_query_t* relativeToWorld;
};

SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_setup(dual_storage_t* world, skr_transform_system_t* system);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_update(skr_transform_system_t* query);

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
            result.entity = luaL_checkinteger(L, index);
            return result;
        }
    };
}

#endif