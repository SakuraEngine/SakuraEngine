#include "SkrRT/rttr/exec_static.hpp"
#include "SkrRT/rttr/iobject.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/type_loader/array_type_loader.hpp"
#include "SkrRT/rttr/type_loader/pointer_type_loader.hpp"
#include "SkrRT/rttr/type_loader/primitive_type_loader.hpp"
#include "SkrRT/rttr/type_loader/reference_type_loader.hpp"

SKR_RTTR_EXEC_STATIC
{
    using namespace skr::rttr;

    // int types
    static PrimitiveTypeLoader<int8_t> int8_loader;
    register_type_loader(type_id<int8_t>(), &int8_loader);
    static PrimitiveTypeLoader<int8_t> uint8_loader;
    register_type_loader(type_id<uint8_t>(), &uint8_loader);
    static PrimitiveTypeLoader<int16_t> int16_loader;
    register_type_loader(type_id<int16_t>(), &int16_loader);
    static PrimitiveTypeLoader<uint16_t> uint16_loader;
    register_type_loader(type_id<uint16_t>(), &uint16_loader);
    static PrimitiveTypeLoader<int32_t> int32_loader;
    register_type_loader(type_id<int32_t>(), &int32_loader);
    static PrimitiveTypeLoader<uint32_t> uint32_loader;
    register_type_loader(type_id<uint32_t>(), &uint32_loader);
    static PrimitiveTypeLoader<int64_t> int64_loader;
    register_type_loader(type_id<int64_t>(), &int64_loader);
    static PrimitiveTypeLoader<uint64_t> uint64_loader;
    register_type_loader(type_id<uint64_t>(), &uint64_loader);

    // bool & void
    static PrimitiveTypeLoader<bool> bool_loader;
    register_type_loader(type_id<bool>(), &bool_loader);
    static PrimitiveTypeLoader<void> void_loader;
    register_type_loader(type_id<void>(), &void_loader);

    // float
    static PrimitiveTypeLoader<float> float_loader;
    register_type_loader(type_id<float>(), &float_loader);
    static PrimitiveTypeLoader<double> double_loader;
    register_type_loader(type_id<double>(), &double_loader);

    // pointer
    static PointerTypeLoader pointer_loader;
    register_generic_type_loader(kPointerGenericGUID, &pointer_loader);

    // reference
    static ReferenceTypeLoader reference_loader;
    register_generic_type_loader(kReferenceGenericGUID, &reference_loader);

    // array
    static VectorTypeLoader array_loader;
    register_generic_type_loader(kArrayGenericGUID, &array_loader);

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
    register_type_loader(type_id<skr::rttr::IObject>(), &iobject_loader);
};