#include "SkrCore/exec_static.hpp"
#include "SkrRTTR/export/record_builder.hpp"
#include "SkrRTTR/iobject.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRTTR/export/primitive_builder.hpp"

// primitive type helper
namespace skr::rttr
{
template <typename T>
void primitive_type_loader(Type* type)
{
    // init type
    type->init(ETypeCategory::Primitive);
    auto& primitive_data = type->primitive_data();

    // builder
    PrimitiveBuilder<T, RTTRBackend> builder(&primitive_data);
    builder.basic_info();
}

static void primitive_type_loader_void(Type* type)
{
    // init type
    type->init(ETypeCategory::Primitive);
    auto& primitive_data = type->primitive_data();

    // build
    primitive_data.name      = RTTRTraits<void>::get_name();
    primitive_data.type_id   = RTTRTraits<void>::get_guid();
    primitive_data.size      = 0;
    primitive_data.alignment = 0;
}
} // namespace skr::rttr

SKR_EXEC_STATIC_CTOR
{
    using namespace skr::rttr;

    // int types
    register_type_loader(type_id_of<int8_t>(), &primitive_type_loader<int8_t>);
    register_type_loader(type_id_of<int16_t>(), &primitive_type_loader<int16_t>);
    register_type_loader(type_id_of<int32_t>(), &primitive_type_loader<int32_t>);
    register_type_loader(type_id_of<int64_t>(), &primitive_type_loader<int64_t>);
    register_type_loader(type_id_of<uint8_t>(), &primitive_type_loader<uint8_t>);
    register_type_loader(type_id_of<uint16_t>(), &primitive_type_loader<uint16_t>);
    register_type_loader(type_id_of<uint32_t>(), &primitive_type_loader<uint32_t>);
    register_type_loader(type_id_of<uint64_t>(), &primitive_type_loader<uint64_t>);

    // bool & void
    register_type_loader(type_id_of<bool>(), &primitive_type_loader<bool>);
    register_type_loader(type_id_of<void>(), &primitive_type_loader_void);

    // float
    register_type_loader(type_id_of<float>(), &primitive_type_loader<float>);
    register_type_loader(type_id_of<double>(), &primitive_type_loader<double>);

    // IObject
    register_type_loader(type_id_of<IObject>(), +[](Type* type) {
        // init type
        type->init(ETypeCategory::Record);
        auto& record_data = type->record_data(); 

        // build
        RecordBuilder<IObject, RTTRBackend> builder(&record_data);
        builder.basic_info(); });
};