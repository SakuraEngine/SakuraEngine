//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#pragma once
#include "SkrBase/config.h"
#include <inttypes.h>

#ifdef __meta__
#error "this file should not be inspected by meta"
#endif

#ifdef SKR_FILE_ID
    #undef SKR_FILE_ID
#endif
#define SKR_FILE_ID FID_SkrScene_scene_h_meta

// BEGIN forward declarations
struct skr_child_comp_t;
struct skr_parent_comp_t;
struct skr_name_comp_t;
struct skr_index_comp_t;
struct skr_transform_comp_t;
struct skr_rotation_comp_t;
struct skr_translation_comp_t;
struct skr_scale_comp_t;
struct skr_movement_comp_t;
struct skr_camera_comp_t;

// END forward declarations
// BEGIN DUAL GENERATED
#include "SkrRT/ecs/sugoi.h"

#ifdef __cplusplus
template<>
struct sugoi_id_of<::skr_child_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_parent_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_name_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_index_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_transform_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_rotation_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_translation_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_scale_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_movement_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
template<>
struct sugoi_id_of<::skr_camera_comp_t>
{
    SKR_SCENE_API static sugoi_type_index_t get();
};
//SKR_SCENE_API skr::span<sugoi_type_index_t> sugoi_get_all_component_types_SkrScene();
#endif

//END DUAL GENERATED// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
}

// rttr traits
SKR_RTTR_TYPE(::skr_child_comp_t, "82CDDC11-3D94-4552-8FD4-237A053F35C0")
SKR_RTTR_TYPE(::skr_parent_comp_t, "2CAA41D2-54A4-46FB-BE43-68B545F313BF")
SKR_RTTR_TYPE(::skr_name_comp_t, "1CD632F6-3149-42E6-9114-647B0C803F32")
SKR_RTTR_TYPE(::skr_index_comp_t, "b08ec011-9b94-47f7-9926-c66d41d735e9")
SKR_RTTR_TYPE(::skr_transform_comp_t, "AE2C7477-8A44-4339-BE5D-64D05D7E05B1")
SKR_RTTR_TYPE(::skr_rotation_comp_t, "78DD218B-87DE-4250-A7E8-A6B4553B47BF")
SKR_RTTR_TYPE(::skr_translation_comp_t, "A059A2A1-CC3B-43B0-88B6-ADA7822BA25D")
SKR_RTTR_TYPE(::skr_scale_comp_t, "D045D755-FBD1-44C2-8BF0-C86F2D8485FF")
SKR_RTTR_TYPE(::skr_movement_comp_t, "4fa24729-2c66-45a2-9417-3497ebc18771")
SKR_RTTR_TYPE(::skr_camera_comp_t, "d33c74c5-2763-4ba4-b58e-dc44a627ebf4")
// END RTTR GENERATED//BEGIN BINARY GENERATED
#include "SkrBase/types.h"

#if defined(__cplusplus)
namespace skr::binary
{
template<>
struct SKR_SCENE_API ReadTrait<skr_rotation_comp_t>
{
    static int Read(skr_binary_reader_t* archive, skr_rotation_comp_t& value );
};
template<>
struct SKR_SCENE_API WriteTrait<skr_rotation_comp_t>
{
    static int Write(skr_binary_writer_t* archive, const skr_rotation_comp_t& value );
};
template<>
struct SKR_SCENE_API ReadTrait<skr_translation_comp_t>
{
    static int Read(skr_binary_reader_t* archive, skr_translation_comp_t& value );
};
template<>
struct SKR_SCENE_API WriteTrait<skr_translation_comp_t>
{
    static int Write(skr_binary_writer_t* archive, const skr_translation_comp_t& value );
};
template<>
struct SKR_SCENE_API ReadTrait<skr_scale_comp_t>
{
    static int Read(skr_binary_reader_t* archive, skr_scale_comp_t& value );
};
template<>
struct SKR_SCENE_API WriteTrait<skr_scale_comp_t>
{
    static int Write(skr_binary_writer_t* archive, const skr_scale_comp_t& value );
};
}




#endif
//END BINARY GENERATED
// BEGIN JSON GENERATED
#ifdef __cplusplus
#include "SkrBase/types.h"

namespace skr::json
{
    template <>
    struct SKR_SCENE_API ReadTrait<skr_rotation_comp_t>
    {
        static error_code Read(value_t&& json, skr_rotation_comp_t& v);
    };
    template <>
    struct SKR_SCENE_API WriteTrait<skr_rotation_comp_t>
    {
        static void Write(skr_json_writer_t* writer, const skr_rotation_comp_t& v);
        static void WriteFields(skr_json_writer_t* writer, const skr_rotation_comp_t& v);
    };
    template <>
    struct SKR_SCENE_API ReadTrait<skr_translation_comp_t>
    {
        static error_code Read(value_t&& json, skr_translation_comp_t& v);
    };
    template <>
    struct SKR_SCENE_API WriteTrait<skr_translation_comp_t>
    {
        static void Write(skr_json_writer_t* writer, const skr_translation_comp_t& v);
        static void WriteFields(skr_json_writer_t* writer, const skr_translation_comp_t& v);
    };
    template <>
    struct SKR_SCENE_API ReadTrait<skr_scale_comp_t>
    {
        static error_code Read(value_t&& json, skr_scale_comp_t& v);
    };
    template <>
    struct SKR_SCENE_API WriteTrait<skr_scale_comp_t>
    {
        static void Write(skr_json_writer_t* writer, const skr_scale_comp_t& v);
        static void WriteFields(skr_json_writer_t* writer, const skr_scale_comp_t& v);
    };
}
#endif
// END JSON GENERATED
