#pragma once
#include "SkrBase/config.h"
#include "SkrRT/ecs/sugoi_types.h"
#ifndef __meta__
    #include "SkrScene/scene.generated.h" // IWYU pragma: export
#endif

#ifndef SKR_SCENE_MAX_NAME_LENGTH
#define SKR_SCENE_MAX_NAME_LENGTH 32
#endif

struct SRenderer;

namespace skr
{
// scene hierarchy

sreflect_struct(
    "guid" : "1CD632F6-3149-42E6-9114-647B0C803F32",
    "ecs::comp" : true
)
NameComponent {
    char str[SKR_SCENE_MAX_NAME_LENGTH + 1];
};

sreflect_struct(
    "guid" : "b08ec011-9b94-47f7-9926-c66d41d735e9",
    "ecs::comp" : true
)
IndexComponent {
    uint32_t value;
};

// transforms

sreflect_struct(
    "guid" : "AE2C7477-8A44-4339-BE5D-64D05D7E05B1",
    "ecs::comp" : true
)
SKR_ALIGNAS(16) 
TransformComponent {
    skr_transform_t value;
};

sreflect_struct(
    "guid" : "78DD218B-87DE-4250-A7E8-A6B4553B47BF", 
    "serialize" : ["bin", "json"],
    "ecs::comp" : true
)
RotationComponent {
    skr_rotator_t euler;
};

sreflect_struct(
    "guid" : "A059A2A1-CC3B-43B0-88B6-ADA7822BA25D",
    "serialize" : ["bin", "json"],
    "ecs::comp" : true
)
TranslationComponent {
    skr_float3_t value;
};

sreflect_struct(
    "guid" : "D045D755-FBD1-44C2-8BF0-C86F2D8485FF", 
    "serialize" : ["bin", "json"],
    "ecs::comp" : true
)
ScaleComponent {
    skr_float3_t value;
};

sreflect_struct(
    "guid" : "4fa24729-2c66-45a2-9417-3497ebc18771", 
    "ecs::comp" : true
)
MovementComponent {
    skr_float3_t value;
};

sreflect_struct(
    "guid" : "d33c74c5-2763-4ba4-b58e-dc44a627ebf4", 
    "ecs::comp" : true
)
CameraComponent {
    struct SRenderer* renderer;
    uint32_t          viewport_id;
    uint32_t          viewport_width;
    uint32_t          viewport_height;
};

sreflect_struct(
    "guid": "82CDDC11-3D94-4552-8FD4-237A053F35C0",
    "ecs::comp::array": 4
) ChildrenComponent {
    sugoi_entity_t entity;
};

#ifdef __cplusplus
using ChildrenArray = sugoi::ArrayComponent<ChildrenComponent, 4>;
#endif

sreflect_struct(
    "guid" : "2CAA41D2-54A4-46FB-BE43-68B545F313BF",
    "ecs::comp" : true
)
ParentComponent {
    sugoi_entity_t entity;
};

} // namespace skr

#ifndef SKR_SCENE_COMPONENTS
#define SKR_SCENE_COMPONENTS skr::ParentComponent, skr::ChildrenComponent, skr::TranslationComponent, skr::RotationComponent, skr::ScaleComponent, skr::TransformComponent
#endif

namespace skr
{
struct SKR_SCENE_API TransformSystem 
{
public:
    static TransformSystem* Create(sugoi_storage_t* world) SKR_NOEXCEPT;
    static void Destroy(TransformSystem* system) SKR_NOEXCEPT;

    void update() SKR_NOEXCEPT;
    sugoi_entity_t root_mark() const SKR_NOEXCEPT;
    void set_parallel_entry(sugoi_entity_t entity) SKR_NOEXCEPT;

private:
    TransformSystem() SKR_NOEXCEPT = default;
    ~TransformSystem() SKR_NOEXCEPT = default;
    struct Impl;
    Impl* impl;

};
} // namespace skr

SKR_SCENE_EXTERN_C SKR_SCENE_API skr::TransformSystem* skr_transform_system_create(sugoi_storage_t* world);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_system_destroy(skr::TransformSystem* system);

SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_system_set_parallel_entry(skr::TransformSystem* system, sugoi_entity_t entity);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_transform_system_update(skr::TransformSystem* system);

SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_propagate_transform(sugoi_storage_t* world, sugoi_entity_t* entities, uint32_t count);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_save_scene(sugoi_storage_t* world, struct skr::archive::JsonWriter* writer);
SKR_SCENE_EXTERN_C SKR_SCENE_API void skr_load_scene(sugoi_storage_t* world, struct skr::archive::JsonReader* reader);

#ifdef __cplusplus
// FIXME. lua support
//     #include "SkrLua/bind.hpp"
// namespace skr::lua
// {
// SKR_SCENE_API int             push_name_comp(lua_State* L, const skr_name_comp_t& value);
// SKR_SCENE_API skr_name_comp_t check_name_comp(lua_State* L, int index);
// template <>
// struct BindTrait<skr_name_comp_t> {
//     static int push(lua_State* L, const skr_name_comp_t& value)
//     {
//         return push_name_comp(L, value);
//     }

//     static skr_name_comp_t check(lua_State* L, int index)
//     {
//         return check_name_comp(L, index);
//     }
// };
// template <>
// struct BindTrait<skr::ChildrenComponent> {
//     static int push(lua_State* L, const skr::ChildrenComponent& value)
//     {
//         lua_pushinteger(L, value.entity);
//         return 1;
//     }

//     static skr::ChildrenComponent check(lua_State* L, int index)
//     {
//         skr::ChildrenComponent result;
//         result.entity = static_cast<sugoi_entity_t>(luaL_checkinteger(L, index));
//         return result;
//     }
// };
// } // namespace skr::lua

inline static SKR_CONSTEXPR bool operator==(skr::ScaleComponent l, skr::ScaleComponent r)
{
    return (l.value == r.value);
}
inline static SKR_CONSTEXPR bool operator!=(skr::ScaleComponent l, skr::ScaleComponent r)
{
    return (l.value != r.value);
}

inline static SKR_CONSTEXPR bool operator==(skr::TranslationComponent l, skr::TranslationComponent r)
{
    return (l.value == r.value);
}
inline static SKR_CONSTEXPR bool operator!=(skr::TranslationComponent l, skr::TranslationComponent r)
{
    return (l.value != r.value);
}

inline static SKR_CONSTEXPR bool operator==(skr::RotationComponent l, skr::RotationComponent r)
{
    return (l.euler == r.euler);
}
inline static SKR_CONSTEXPR bool operator!=(skr::RotationComponent l, skr::RotationComponent r)
{
    return (l.euler != r.euler);
}

#endif