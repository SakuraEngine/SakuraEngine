#include "SkrRTTR/exec_static.hpp"
#include "SkrRTTR/iobject.hpp"
#include "SkrRTTR/type/record_type.hpp"
#include "SkrRTTR/type_loader/primitive_type_loader.hpp"

SKR_RTTR_EXEC_STATIC
{
    using namespace skr::rttr;

    // int types
    static PrimitiveTypeLoader<int8_t> int8_loader;
    register_type_loader(type_id_of<int8_t>(), &int8_loader);
    static PrimitiveTypeLoader<int8_t> uint8_loader;
    register_type_loader(type_id_of<uint8_t>(), &uint8_loader);
    static PrimitiveTypeLoader<int16_t> int16_loader;
    register_type_loader(type_id_of<int16_t>(), &int16_loader);
    static PrimitiveTypeLoader<uint16_t> uint16_loader;
    register_type_loader(type_id_of<uint16_t>(), &uint16_loader);
    static PrimitiveTypeLoader<int32_t> int32_loader;
    register_type_loader(type_id_of<int32_t>(), &int32_loader);
    static PrimitiveTypeLoader<uint32_t> uint32_loader;
    register_type_loader(type_id_of<uint32_t>(), &uint32_loader);
    static PrimitiveTypeLoader<int64_t> int64_loader;
    register_type_loader(type_id_of<int64_t>(), &int64_loader);
    static PrimitiveTypeLoader<uint64_t> uint64_loader;
    register_type_loader(type_id_of<uint64_t>(), &uint64_loader);

    // bool & void
    static PrimitiveTypeLoader<bool> bool_loader;
    register_type_loader(type_id_of<bool>(), &bool_loader);
    static PrimitiveTypeLoader<void> void_loader;
    register_type_loader(type_id_of<void>(), &void_loader);

    // float
    static PrimitiveTypeLoader<float> float_loader;
    register_type_loader(type_id_of<float>(), &float_loader);
    static PrimitiveTypeLoader<double> double_loader;
    register_type_loader(type_id_of<double>(), &double_loader);

    // IObject
    static struct IObjectLoader : public TypeLoader {
        Type* create() override
        {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::rttr::IObject>::get_name(),
                RTTRTraits<::skr::rttr::IObject>::get_guid(),
                sizeof(skr::rttr::IObject),
                alignof(skr::rttr::IObject),
                make_record_basic_method_table<skr::rttr::IObject>());
        }
        void load(Type* type) override
        {
        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } iobject_loader;
    register_type_loader(type_id_of<skr::rttr::IObject>(), &iobject_loader);
};