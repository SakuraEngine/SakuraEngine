#include "SkrCore/exec_static.hpp"
#include "SkrRTTR/export/export_builder.hpp"
#include "SkrRTTR/iobject.hpp"
#include "SkrRTTR/type.hpp"
#include <SkrRTTR/generic/generic_base.hpp>
#include <SkrRTTR/generic/generic_vector.hpp>
#include <SkrRTTR/generic/generic_sparse_vector.hpp>
#include <SkrRTTR/generic/generic_optional.hpp>
#include <SkrRTTR/generic/generic_sparse_hash_set.hpp>
#include <SkrRTTR/generic/generic_sparse_hash_map.hpp>

// primitive type helper
namespace skr
{
template <typename T>
void primitive_type_loader(RTTRType* type)
{
    type->build_primitive([&](RTTRPrimitiveData* data) {
        RTTRPrimitiveBuilder<T> builder(data);
        builder.basic_info();
    });
}
static void primitive_type_loader_void(RTTRType* type)
{
    type->build_primitive([&](RTTRPrimitiveData* data) {
        data->name      = RTTRTraits<void>::get_name();
        data->type_id   = RTTRTraits<void>::get_guid();
        data->size      = 0;
        data->alignment = 0;
    });
}

template <typename T>
void minimal_type_loader_record(RTTRType* type)
{
    type->set_module(u8"SkrCore");
    type->build_record([](RTTRRecordData* data) {
        RTTRRecordBuilder<T> builder(data);
        builder.basic_info();
    });
}
} // namespace skr

SKR_EXEC_STATIC_CTOR
{
    using namespace skr;

    // register primitive types
    {
        // void
        register_type_loader(type_id_of<void>(), &primitive_type_loader_void);

#define REG_PRIMITIVE_TYPE_LOADER(__TYPE) register_type_loader(type_id_of<__TYPE>(), &primitive_type_loader<__TYPE>)
        // int types
        REG_PRIMITIVE_TYPE_LOADER(int8_t);
        REG_PRIMITIVE_TYPE_LOADER(int16_t);
        REG_PRIMITIVE_TYPE_LOADER(int32_t);
        REG_PRIMITIVE_TYPE_LOADER(int64_t);
        REG_PRIMITIVE_TYPE_LOADER(uint8_t);
        REG_PRIMITIVE_TYPE_LOADER(uint16_t);
        REG_PRIMITIVE_TYPE_LOADER(uint32_t);
        REG_PRIMITIVE_TYPE_LOADER(uint64_t);

        // bool
        REG_PRIMITIVE_TYPE_LOADER(bool);

        // float
        REG_PRIMITIVE_TYPE_LOADER(float);
        REG_PRIMITIVE_TYPE_LOADER(double);
#undef REG_PRIMITIVE_TYPE_LOADER
    }

    // register core types
    {
#define REG_MINIMAL_TYPE_LOADER(__TYPE) register_type_loader(type_id_of<__TYPE>(), &minimal_type_loader_record<__TYPE>)
        // guid & md5
        REG_MINIMAL_TYPE_LOADER(::skr::GUID);
        REG_MINIMAL_TYPE_LOADER(::skr::MD5);

        // string types
        REG_MINIMAL_TYPE_LOADER(::skr::String);
        REG_MINIMAL_TYPE_LOADER(::skr::StringView);

        // math misc types
        REG_MINIMAL_TYPE_LOADER(::skr::RotatorF);
        REG_MINIMAL_TYPE_LOADER(::skr::RotatorD);
        REG_MINIMAL_TYPE_LOADER(::skr::QuatF);
        REG_MINIMAL_TYPE_LOADER(::skr::QuatD);
        REG_MINIMAL_TYPE_LOADER(::skr::TransformF);
        REG_MINIMAL_TYPE_LOADER(::skr::TransformD);

        // math vector types
        REG_MINIMAL_TYPE_LOADER(::skr::float2);
        REG_MINIMAL_TYPE_LOADER(::skr::float3);
        REG_MINIMAL_TYPE_LOADER(::skr::float4);
        REG_MINIMAL_TYPE_LOADER(::skr::double2);
        REG_MINIMAL_TYPE_LOADER(::skr::double3);
        REG_MINIMAL_TYPE_LOADER(::skr::double4);
        REG_MINIMAL_TYPE_LOADER(::skr::int2);
        REG_MINIMAL_TYPE_LOADER(::skr::int3);
        REG_MINIMAL_TYPE_LOADER(::skr::int4);
        REG_MINIMAL_TYPE_LOADER(::skr::uint2);
        REG_MINIMAL_TYPE_LOADER(::skr::uint3);
        REG_MINIMAL_TYPE_LOADER(::skr::uint4);
        REG_MINIMAL_TYPE_LOADER(::skr::long2);
        REG_MINIMAL_TYPE_LOADER(::skr::long3);
        REG_MINIMAL_TYPE_LOADER(::skr::long4);
        REG_MINIMAL_TYPE_LOADER(::skr::ulong2);
        REG_MINIMAL_TYPE_LOADER(::skr::ulong3);
        REG_MINIMAL_TYPE_LOADER(::skr::ulong4);
        REG_MINIMAL_TYPE_LOADER(::skr::bool2);
        REG_MINIMAL_TYPE_LOADER(::skr::bool3);
        REG_MINIMAL_TYPE_LOADER(::skr::bool4);

        // matrix types
        REG_MINIMAL_TYPE_LOADER(::skr::float3x3);
        REG_MINIMAL_TYPE_LOADER(::skr::float4x4);
        REG_MINIMAL_TYPE_LOADER(::skr::double3x3);
        REG_MINIMAL_TYPE_LOADER(::skr::double4x4);
#undef REG_MINIMAL_TYPE_LOADER
    }

    // register generic
    {
        // optional
        register_generic_processor(
            kOptionalGenericId,
            +[](TypeSignatureView sig) -> RC<IGenericBase> {
                return RC<GenericOptional>::New(
                    build_generic(sig)
                );
            }
        );

        // vector
        register_generic_processor(
            kVectorGenericId,
            +[](TypeSignatureView sig) -> RC<IGenericBase> {
                return RC<GenericVector>::New(
                    build_generic(sig)
                );
            }
        );

        // sparse vector
        register_generic_processor(
            kSparseVectorGenericId,
            +[](TypeSignatureView sig) -> RC<IGenericBase> {
                return RC<GenericSparseVector>::New(
                    build_generic(sig)
                );
            }
        );

        // sparse hash set
        register_generic_processor(
            kSetGenericId,
            +[](TypeSignatureView sig) -> RC<IGenericBase> {
                return RC<GenericSparseHashSet>::New(
                    build_generic(sig)
                );
            }
        );

        // sparse hash map
        register_generic_processor(
            kMapGenericId,
            +[](TypeSignatureView sig) -> RC<IGenericBase> {
                auto key   = sig.jump_next_type_or_data();
                auto value = sig.jump_next_type_or_data();

                return RC<GenericSparseHashMap>::New(
                    build_generic(key),
                    build_generic(value)
                );
            }
        );
    }
};