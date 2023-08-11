#include "SkrRT/misc/hash.h"
#include "SkrRT/platform/guid.hpp"
#include "SkrRT/resource/resource_handle.h"
#include "SkrRT/type/type.hpp"

namespace skr {
namespace type {

void type_register<bool>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_BOOL;
}

void type_register<int32_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_I32;
}

void type_register<int64_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_I64;
}

void type_register<uint32_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_U32;
}

void type_register<uint64_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_U64;
}

void type_register<float>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_F32;
}

void type_register<skr_float2_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_F32_2;
}

void type_register<skr_float3_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_F32_3;
}

void type_register<skr_float4_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_F32_4;
}

void type_register<skr_rotator_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_ROT;
}

void type_register<skr_quaternion_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_QUAT;
}

void type_register<skr_float4x4_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_F32_4x4;
}

void type_register<double>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_F64;
}

void type_register<skr_guid_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_GUID;
}

void type_register<skr_md5_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_MD5;
}

void type_register<skr_resource_handle_t>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_HANDLE;
}

void type_register<skr::string>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_STR;
}

void type_register<skr::string_view>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_STRV;
}

void type_register<void*>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_I64;
}

void type_register<SInterface>::instantiate_type(RecordType* type)
{
    type->type = SKR_TYPE_CATEGORY_OBJ;
}

uint64_t Hash(bool value, uint64_t base)
{
    return skr_hash(&value, 1, base);
}
uint64_t Hash(int32_t value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(int64_t value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(uint32_t value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(uint64_t value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(float value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_float2_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_float3_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_float4_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_float4x4_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_quaternion_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_rotator_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(double value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_guid_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_md5_t& value, uint64_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
uint64_t Hash(const skr_resource_handle_t& value, uint64_t base)
{
    auto guid = value.get_guid();
    return skr_hash(&guid, sizeof(guid), base);
}
uint64_t Hash(void* value, uint64_t base)
{
    return skr_hash((void*)&value, sizeof(value), base);
}
uint64_t Hash(const skr::string& value, uint64_t base)
{
    return skr_hash(value.c_str(), value.size(), base);
}
uint64_t Hash(const skr::string_view& value, uint64_t base)
{
    return skr_hash(value.raw().data(), value.size(), base);
}
} // namespace type
} // namespace skr