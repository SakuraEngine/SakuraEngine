//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// BEGIN header includes
#include "D:/workspace/project/SakuraEngine/modules/engine/scene/include/SkrScene/scene.h"
#include "D:/workspace/project/SakuraEngine/modules/engine/scene/include/SkrScene/resources/scene_resource.h"
// END header includes

// BEGIN push diagnostic
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicitly-unsigned-literal"
#endif
// END push diagnostic

// BEGIN DUAL GENERATED
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrLua/sugoi_bind.hpp"
#include "SkrRT/ecs/serde.hpp"

static struct RegisterComponentskr_child_comp_tHelper
{
    RegisterComponentskr_child_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_child_comp_t";
        
        desc.size = sizeof(sugoi::array_comp_T<skr_child_comp_t, 4>);
    
        desc.entityFieldsCount = 1;
        static intptr_t entityFields[] = {0};
        desc.entityFields = (intptr_t)entityFields;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0x82CDDC11, 0x3D94, 0x4552, {0x8F, 0xD4, 0x23, 0x7A, 0x05, 0x3F, 0x35, 0xC0}};
        desc.guidStr = u8"82CDDC11-3D94-4552-8FD4-237A053F35C0";
        desc.flags = 0;
        desc.elementSize = sizeof(skr_child_comp_t);
        desc.alignment = alignof(sugoi::array_comp_T<skr_child_comp_t, 4>);

        sugoi::SetLuaBindCallback<skr_child_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_child_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_child_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_child_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_child_comp_t>::get()
{
    auto result = _RegisterComponentskr_child_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_parent_comp_tHelper
{
    RegisterComponentskr_parent_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_parent_comp_t";
        
        desc.size = std::is_empty_v<skr_parent_comp_t> ? 0 : sizeof(skr_parent_comp_t);
    
        desc.entityFieldsCount = 1;
        static intptr_t entityFields[] = {0};
        desc.entityFields = (intptr_t)entityFields;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0x2CAA41D2, 0x54A4, 0x46FB, {0xBE, 0x43, 0x68, 0xB5, 0x45, 0xF3, 0x13, 0xBF}};
        desc.guidStr = u8"2CAA41D2-54A4-46FB-BE43-68B545F313BF";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_parent_comp_t);

        sugoi::SetLuaBindCallback<skr_parent_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_parent_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_parent_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_parent_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_parent_comp_t>::get()
{
    auto result = _RegisterComponentskr_parent_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_name_comp_tHelper
{
    RegisterComponentskr_name_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_name_comp_t";
        
        desc.size = std::is_empty_v<skr_name_comp_t> ? 0 : sizeof(skr_name_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0x1CD632F6, 0x3149, 0x42E6, {0x91, 0x14, 0x64, 0x7B, 0x0C, 0x80, 0x3F, 0x32}};
        desc.guidStr = u8"1CD632F6-3149-42E6-9114-647B0C803F32";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_name_comp_t);

        sugoi::SetLuaBindCallback<skr_name_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_name_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_name_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_name_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_name_comp_t>::get()
{
    auto result = _RegisterComponentskr_name_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_index_comp_tHelper
{
    RegisterComponentskr_index_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_index_comp_t";
        
        desc.size = std::is_empty_v<skr_index_comp_t> ? 0 : sizeof(skr_index_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0xb08ec011, 0x9b94, 0x47f7, {0x99, 0x26, 0xc6, 0x6d, 0x41, 0xd7, 0x35, 0xe9}};
        desc.guidStr = u8"b08ec011-9b94-47f7-9926-c66d41d735e9";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_index_comp_t);

        sugoi::SetLuaBindCallback<skr_index_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_index_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_index_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_index_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_index_comp_t>::get()
{
    auto result = _RegisterComponentskr_index_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_transform_comp_tHelper
{
    RegisterComponentskr_transform_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_transform_comp_t";
        
        desc.size = std::is_empty_v<skr_transform_comp_t> ? 0 : sizeof(skr_transform_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0xAE2C7477, 0x8A44, 0x4339, {0xBE, 0x5D, 0x64, 0xD0, 0x5D, 0x7E, 0x05, 0xB1}};
        desc.guidStr = u8"AE2C7477-8A44-4339-BE5D-64D05D7E05B1";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_transform_comp_t);

        sugoi::SetLuaBindCallback<skr_transform_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_transform_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_transform_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_transform_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_transform_comp_t>::get()
{
    auto result = _RegisterComponentskr_transform_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_rotation_comp_tHelper
{
    RegisterComponentskr_rotation_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_rotation_comp_t";
        
        desc.size = std::is_empty_v<skr_rotation_comp_t> ? 0 : sizeof(skr_rotation_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0x78DD218B, 0x87DE, 0x4250, {0xA7, 0xE8, 0xA6, 0xB4, 0x55, 0x3B, 0x47, 0xBF}};
        desc.guidStr = u8"78DD218B-87DE-4250-A7E8-A6B4553B47BF";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_rotation_comp_t);

        sugoi::SetLuaBindCallback<skr_rotation_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_rotation_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_rotation_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_rotation_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_rotation_comp_t>::get()
{
    auto result = _RegisterComponentskr_rotation_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_translation_comp_tHelper
{
    RegisterComponentskr_translation_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_translation_comp_t";
        
        desc.size = std::is_empty_v<skr_translation_comp_t> ? 0 : sizeof(skr_translation_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0xA059A2A1, 0xCC3B, 0x43B0, {0x88, 0xB6, 0xAD, 0xA7, 0x82, 0x2B, 0xA2, 0x5D}};
        desc.guidStr = u8"A059A2A1-CC3B-43B0-88B6-ADA7822BA25D";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_translation_comp_t);

        sugoi::SetLuaBindCallback<skr_translation_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_translation_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_translation_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_translation_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_translation_comp_t>::get()
{
    auto result = _RegisterComponentskr_translation_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_scale_comp_tHelper
{
    RegisterComponentskr_scale_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_scale_comp_t";
        
        desc.size = std::is_empty_v<skr_scale_comp_t> ? 0 : sizeof(skr_scale_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0xD045D755, 0xFBD1, 0x44C2, {0x8B, 0xF0, 0xC8, 0x6F, 0x2D, 0x84, 0x85, 0xFF}};
        desc.guidStr = u8"D045D755-FBD1-44C2-8BF0-C86F2D8485FF";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_scale_comp_t);

        sugoi::SetLuaBindCallback<skr_scale_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_scale_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_scale_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_scale_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_scale_comp_t>::get()
{
    auto result = _RegisterComponentskr_scale_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_movement_comp_tHelper
{
    RegisterComponentskr_movement_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_movement_comp_t";
        
        desc.size = std::is_empty_v<skr_movement_comp_t> ? 0 : sizeof(skr_movement_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0x4fa24729, 0x2c66, 0x45a2, {0x94, 0x17, 0x34, 0x97, 0xeb, 0xc1, 0x87, 0x71}};
        desc.guidStr = u8"4fa24729-2c66-45a2-9417-3497ebc18771";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_movement_comp_t);

        sugoi::SetLuaBindCallback<skr_movement_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_movement_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_movement_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_movement_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_movement_comp_t>::get()
{
    auto result = _RegisterComponentskr_movement_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
static struct RegisterComponentskr_camera_comp_tHelper
{
    RegisterComponentskr_camera_comp_tHelper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"skr_camera_comp_t";
        
        desc.size = std::is_empty_v<skr_camera_comp_t> ? 0 : sizeof(skr_camera_comp_t);
    
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
        desc.guid = {0xd33c74c5, 0x2763, 0x4ba4, {0xb5, 0x8e, 0xdc, 0x44, 0xa6, 0x27, 0xeb, 0xf4}};
        desc.guidStr = u8"d33c74c5-2763-4ba4-b58e-dc44a627ebf4";
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(skr_camera_comp_t);

        sugoi::SetLuaBindCallback<skr_camera_comp_t>(desc);
        sugoi::SetSerdeCallback<skr_camera_comp_t>(desc);
    
    
        ::sugoi::check_managed(desc, skr::type_t<skr_camera_comp_t>{});
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponentskr_camera_comp_tHelper;

sugoi_type_index_t sugoi_id_of<::skr_camera_comp_t>::get()
{
    auto result = _RegisterComponentskr_camera_comp_tHelper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}

skr::span<sugoi_type_index_t> sugoi_get_all_component_types_SkrScene()
{
    static sugoi_type_index_t result[10] {
        sugoi_id_of<::skr_child_comp_t>::get(),
        sugoi_id_of<::skr_parent_comp_t>::get(),
        sugoi_id_of<::skr_name_comp_t>::get(),
        sugoi_id_of<::skr_index_comp_t>::get(),
        sugoi_id_of<::skr_transform_comp_t>::get(),
        sugoi_id_of<::skr_rotation_comp_t>::get(),
        sugoi_id_of<::skr_translation_comp_t>::get(),
        sugoi_id_of<::skr_scale_comp_t>::get(),
        sugoi_id_of<::skr_movement_comp_t>::get(),
        sugoi_id_of<::skr_camera_comp_t>::get(),
    };
    return {result};
}

//END DUAL GENERATED// BEGIN RTTR GENERATED
#include "SkrRT/rttr/type_loader/type_loader.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/exec_static.hpp"
#include "SkrRT/rttr/type_loader/enum_type_from_traits_loader.hpp"
#include "SkrContainers/tuple.hpp"

namespace skr::rttr 
{
}

SKR_RTTR_EXEC_STATIC
{
    using namespace ::skr::rttr;

    static struct InternalTypeLoader_skr_child_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_child_comp_t>::get_name(),
                RTTRTraits<::skr_child_comp_t>::get_guid(),
                sizeof(skr_child_comp_t),
                alignof(skr_child_comp_t),
                make_record_basic_method_table<skr_child_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_child_comp_t;
    register_type_loader(RTTRTraits<skr_child_comp_t>::get_guid(), &LOADER__skr_child_comp_t);
    static struct InternalTypeLoader_skr_parent_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_parent_comp_t>::get_name(),
                RTTRTraits<::skr_parent_comp_t>::get_guid(),
                sizeof(skr_parent_comp_t),
                alignof(skr_parent_comp_t),
                make_record_basic_method_table<skr_parent_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_parent_comp_t;
    register_type_loader(RTTRTraits<skr_parent_comp_t>::get_guid(), &LOADER__skr_parent_comp_t);
    static struct InternalTypeLoader_skr_name_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_name_comp_t>::get_name(),
                RTTRTraits<::skr_name_comp_t>::get_guid(),
                sizeof(skr_name_comp_t),
                alignof(skr_name_comp_t),
                make_record_basic_method_table<skr_name_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_name_comp_t;
    register_type_loader(RTTRTraits<skr_name_comp_t>::get_guid(), &LOADER__skr_name_comp_t);
    static struct InternalTypeLoader_skr_index_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_index_comp_t>::get_name(),
                RTTRTraits<::skr_index_comp_t>::get_guid(),
                sizeof(skr_index_comp_t),
                alignof(skr_index_comp_t),
                make_record_basic_method_table<skr_index_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_index_comp_t;
    register_type_loader(RTTRTraits<skr_index_comp_t>::get_guid(), &LOADER__skr_index_comp_t);
    static struct InternalTypeLoader_skr_transform_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_transform_comp_t>::get_name(),
                RTTRTraits<::skr_transform_comp_t>::get_guid(),
                sizeof(skr_transform_comp_t),
                alignof(skr_transform_comp_t),
                make_record_basic_method_table<skr_transform_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_transform_comp_t;
    register_type_loader(RTTRTraits<skr_transform_comp_t>::get_guid(), &LOADER__skr_transform_comp_t);
    static struct InternalTypeLoader_skr_rotation_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_rotation_comp_t>::get_name(),
                RTTRTraits<::skr_rotation_comp_t>::get_guid(),
                sizeof(skr_rotation_comp_t),
                alignof(skr_rotation_comp_t),
                make_record_basic_method_table<skr_rotation_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_rotation_comp_t;
    register_type_loader(RTTRTraits<skr_rotation_comp_t>::get_guid(), &LOADER__skr_rotation_comp_t);
    static struct InternalTypeLoader_skr_translation_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_translation_comp_t>::get_name(),
                RTTRTraits<::skr_translation_comp_t>::get_guid(),
                sizeof(skr_translation_comp_t),
                alignof(skr_translation_comp_t),
                make_record_basic_method_table<skr_translation_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_translation_comp_t;
    register_type_loader(RTTRTraits<skr_translation_comp_t>::get_guid(), &LOADER__skr_translation_comp_t);
    static struct InternalTypeLoader_skr_scale_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_scale_comp_t>::get_name(),
                RTTRTraits<::skr_scale_comp_t>::get_guid(),
                sizeof(skr_scale_comp_t),
                alignof(skr_scale_comp_t),
                make_record_basic_method_table<skr_scale_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_scale_comp_t;
    register_type_loader(RTTRTraits<skr_scale_comp_t>::get_guid(), &LOADER__skr_scale_comp_t);
    static struct InternalTypeLoader_skr_movement_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_movement_comp_t>::get_name(),
                RTTRTraits<::skr_movement_comp_t>::get_guid(),
                sizeof(skr_movement_comp_t),
                alignof(skr_movement_comp_t),
                make_record_basic_method_table<skr_movement_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_movement_comp_t;
    register_type_loader(RTTRTraits<skr_movement_comp_t>::get_guid(), &LOADER__skr_movement_comp_t);
    static struct InternalTypeLoader_skr_camera_comp_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_camera_comp_t>::get_name(),
                RTTRTraits<::skr_camera_comp_t>::get_guid(),
                sizeof(skr_camera_comp_t),
                alignof(skr_camera_comp_t),
                make_record_basic_method_table<skr_camera_comp_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_camera_comp_t;
    register_type_loader(RTTRTraits<skr_camera_comp_t>::get_guid(), &LOADER__skr_camera_comp_t);
    static struct InternalTypeLoader_skr_scene_resource_t : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr_scene_resource_t>::get_name(),
                RTTRTraits<::skr_scene_resource_t>::get_guid(),
                sizeof(skr_scene_resource_t),
                alignof(skr_scene_resource_t),
                make_record_basic_method_table<skr_scene_resource_t>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_scene_resource_t;
    register_type_loader(RTTRTraits<skr_scene_resource_t>::get_guid(), &LOADER__skr_scene_resource_t);

};
// END RTTR GENERATED
// BEGIN BINARY GENERATED
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/log.h"
#include "SkrRT/serde/binary/reader.h"
#include "SkrRT/serde/binary/writer.h"
#include "SkrRT/serde/binary/blob.h"
#include "SkrProfile/profile.h"

[[maybe_unused]] static const char8_t* BinaryArrayBinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s[%d]: %d";
[[maybe_unused]] static const char8_t* BinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s: %d";
[[maybe_unused]] static const char8_t* BinaryBaseArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s's base %s: %d";



namespace skr::binary {



template<class S>
int __Archive(S* archive, skr_rotation_comp_t& record)
{
    constexpr bool isWriter = std::is_same_v<S, skr_binary_writer_t>;
    const char* action = isWriter ? "Write" : "Read";
    int ret = 0;
    
    
    ret = Archive(archive, record.euler);

    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "skr_rotation_comp_t", "euler", ret);
        return ret;
    }
    return ret;
}

int ReadTrait<skr_rotation_comp_t>::Read(skr_binary_reader_t* archive, skr_rotation_comp_t& record)
{
    SkrZoneScopedN("binary::ReadTrait<skr_rotation_comp_t>::Read");
    return __Archive(archive, record);
}
int WriteTrait<skr_rotation_comp_t>::Write(skr_binary_writer_t* archive, const skr_rotation_comp_t& record)
{
    SkrZoneScopedN("binary::WriteTrait<skr_rotation_comp_t>::Write");
    return __Archive(archive, (skr_rotation_comp_t&)record);
} 


template<class S>
int __Archive(S* archive, skr_translation_comp_t& record)
{
    constexpr bool isWriter = std::is_same_v<S, skr_binary_writer_t>;
    const char* action = isWriter ? "Write" : "Read";
    int ret = 0;
    
    
    ret = Archive(archive, record.value);

    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "skr_translation_comp_t", "value", ret);
        return ret;
    }
    return ret;
}

int ReadTrait<skr_translation_comp_t>::Read(skr_binary_reader_t* archive, skr_translation_comp_t& record)
{
    SkrZoneScopedN("binary::ReadTrait<skr_translation_comp_t>::Read");
    return __Archive(archive, record);
}
int WriteTrait<skr_translation_comp_t>::Write(skr_binary_writer_t* archive, const skr_translation_comp_t& record)
{
    SkrZoneScopedN("binary::WriteTrait<skr_translation_comp_t>::Write");
    return __Archive(archive, (skr_translation_comp_t&)record);
} 


template<class S>
int __Archive(S* archive, skr_scale_comp_t& record)
{
    constexpr bool isWriter = std::is_same_v<S, skr_binary_writer_t>;
    const char* action = isWriter ? "Write" : "Read";
    int ret = 0;
    
    
    ret = Archive(archive, record.value);

    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "skr_scale_comp_t", "value", ret);
        return ret;
    }
    return ret;
}

int ReadTrait<skr_scale_comp_t>::Read(skr_binary_reader_t* archive, skr_scale_comp_t& record)
{
    SkrZoneScopedN("binary::ReadTrait<skr_scale_comp_t>::Read");
    return __Archive(archive, record);
}
int WriteTrait<skr_scale_comp_t>::Write(skr_binary_writer_t* archive, const skr_scale_comp_t& record)
{
    SkrZoneScopedN("binary::WriteTrait<skr_scale_comp_t>::Write");
    return __Archive(archive, (skr_scale_comp_t&)record);
} 
}
//END BINARY GENERATED// BEGIN JSON IMPLEMENTATION
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/log.h"
#include "SkrRT/serde/json/reader.h"
#include "SkrRT/serde/json/writer.h"
#include "SkrProfile/profile.h"
[[maybe_unused]] static const char8_t* JsonArrayJsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s[%d] failed: %s";
[[maybe_unused]] static const char8_t* JsonArrayFieldArchiveWarnFormat = u8"[SERDE/JSON] %s.%s got too many elements (%d expected, given %d), ignoring overflowed elements";
[[maybe_unused]] static const char8_t* JsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s failed: %s";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundErrorFormat = u8"[SERDE/JSON] %s.%s not found";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundFormat = u8"[SERDE/JSON] %s.%s not found, using default value";

[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughErrorFormat = u8"[SERDE/JSON] %s.%s has too few elements (%d expected, given %d)";
[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughWarnFormat = u8"[SERDE/JSON] %s.%s got too few elements (%d expected, given %d), using default value";
[[maybe_unused]] static const char8_t* JsonBaseArchiveFailedFormat = u8"[SERDE/JSON] Archive %s base %s failed: %d";

namespace skr::json {

error_code ReadTrait<skr_rotation_comp_t>::Read(value_t&& json, skr_rotation_comp_t& record)
{
    SkrZoneScopedN("json::ReadTrait<skr_rotation_comp_t>::Read");
    {
        auto field = json["euler"];
        if (field.error() == simdjson::NO_SUCH_FIELD)
        {
            SKR_LOG_WARN(JsonFieldNotFoundFormat, "skr_rotation_comp_t", "euler");
        }
        else if (field.error() != simdjson::SUCCESS)
        {
            SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "skr_rotation_comp_t", "euler", error_message((error_code)field.error()));
            return (error_code)field.error();
        }
        else
        {
            error_code result = skr::json::Read(std::move(field).value_unsafe(), (skr_rotator_t&)record.euler);
            if(result != error_code::SUCCESS)
            {
                SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "skr_rotation_comp_t", "euler", error_message((error_code)field.error()));
                return result;
            }
        }
    }
    return error_code::SUCCESS;
} 
void WriteTrait<skr_rotation_comp_t>::WriteFields(skr_json_writer_t* writer, const skr_rotation_comp_t& record)
{
    writer->Key(u8"euler", 5);
    skr::json::Write<skr_rotator_t>(writer, record.euler);
} 
void WriteTrait<skr_rotation_comp_t>::Write(skr_json_writer_t* writer, const skr_rotation_comp_t& record)
{
    SkrZoneScopedN("json::WriteTrait<skr_rotation_comp_t>::Write");
    writer->StartObject();
    WriteFields(writer, record);
    writer->EndObject();
} 
error_code ReadTrait<skr_translation_comp_t>::Read(value_t&& json, skr_translation_comp_t& record)
{
    SkrZoneScopedN("json::ReadTrait<skr_translation_comp_t>::Read");
    {
        auto field = json["value"];
        if (field.error() == simdjson::NO_SUCH_FIELD)
        {
            SKR_LOG_WARN(JsonFieldNotFoundFormat, "skr_translation_comp_t", "value");
        }
        else if (field.error() != simdjson::SUCCESS)
        {
            SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "skr_translation_comp_t", "value", error_message((error_code)field.error()));
            return (error_code)field.error();
        }
        else
        {
            error_code result = skr::json::Read(std::move(field).value_unsafe(), (skr_float3_t&)record.value);
            if(result != error_code::SUCCESS)
            {
                SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "skr_translation_comp_t", "value", error_message((error_code)field.error()));
                return result;
            }
        }
    }
    return error_code::SUCCESS;
} 
void WriteTrait<skr_translation_comp_t>::WriteFields(skr_json_writer_t* writer, const skr_translation_comp_t& record)
{
    writer->Key(u8"value", 5);
    skr::json::Write<skr_float3_t>(writer, record.value);
} 
void WriteTrait<skr_translation_comp_t>::Write(skr_json_writer_t* writer, const skr_translation_comp_t& record)
{
    SkrZoneScopedN("json::WriteTrait<skr_translation_comp_t>::Write");
    writer->StartObject();
    WriteFields(writer, record);
    writer->EndObject();
} 
error_code ReadTrait<skr_scale_comp_t>::Read(value_t&& json, skr_scale_comp_t& record)
{
    SkrZoneScopedN("json::ReadTrait<skr_scale_comp_t>::Read");
    {
        auto field = json["value"];
        if (field.error() == simdjson::NO_SUCH_FIELD)
        {
            SKR_LOG_WARN(JsonFieldNotFoundFormat, "skr_scale_comp_t", "value");
        }
        else if (field.error() != simdjson::SUCCESS)
        {
            SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "skr_scale_comp_t", "value", error_message((error_code)field.error()));
            return (error_code)field.error();
        }
        else
        {
            error_code result = skr::json::Read(std::move(field).value_unsafe(), (skr_float3_t&)record.value);
            if(result != error_code::SUCCESS)
            {
                SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "skr_scale_comp_t", "value", error_message((error_code)field.error()));
                return result;
            }
        }
    }
    return error_code::SUCCESS;
} 
void WriteTrait<skr_scale_comp_t>::WriteFields(skr_json_writer_t* writer, const skr_scale_comp_t& record)
{
    writer->Key(u8"value", 5);
    skr::json::Write<skr_float3_t>(writer, record.value);
} 
void WriteTrait<skr_scale_comp_t>::Write(skr_json_writer_t* writer, const skr_scale_comp_t& record)
{
    SkrZoneScopedN("json::WriteTrait<skr_scale_comp_t>::Write");
    writer->StartObject();
    WriteFields(writer, record);
    writer->EndObject();
} 
}
// END JSON IMPLEMENTATION
// BEGIN pop diagnostic
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
// END pop diagnostic
