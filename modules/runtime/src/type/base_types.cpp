#include "utils/hash.h"
#include "type/type.hpp"

namespace skr {
namespace type {

const skr_type_t* type_of<bool>::get()
{
    static BoolType type;
    return &type;
}

const skr_type_t* type_of<int32_t>::get()
{
    static Int32Type type;
    return &type;
}

const skr_type_t* type_of<int64_t>::get()
{
    static Int64Type type;
    return &type;
}

const skr_type_t* type_of<uint32_t>::get()
{
    static UInt32Type type;
    return &type;
}

const skr_type_t* type_of<uint64_t>::get()
{
    static UInt64Type type;
    return &type;
}

const skr_type_t* type_of<float>::get()
{
    static Float32Type type;
    return &type;
}
const skr_type_t* type_of<skr_float2_t>::get()
{
    static Float32x2Type type;
    return &type;
}
const skr_type_t* type_of<skr_float3_t>::get()
{
    static Float32x3Type type;
    return &type;
}
const skr_type_t* type_of<skr_float4_t>::get()
{
    static Float32x4Type type;
    return &type;
}
const skr_type_t* type_of<skr_rotator_t>::get()
{
    static RotType type;
    return &type;
}
const skr_type_t* type_of<skr_quaternion_t>::get()
{
    static QuaternionType type;
    return &type;
}
const skr_type_t* type_of<skr_float4x4_t>::get()
{
    static Float32x4x4Type type;
    return &type;
}
const skr_type_t* type_of<double>::get()
{
    static Float64Type type;
    return &type;
}

const skr_type_t* type_of<skr_guid_t>::get()
{
    static GUIDType type;
    return &type;
}

const skr_type_t* type_of<skr_md5_t>::get()
{
    static MD5Type type;
    return &type;
}

const skr_type_t* type_of<skr_resource_handle_t>::get()
{
    static HandleType type{nullptr};
    return &type;
}

const skr_type_t* type_of<skr::string>::get()
{
    static StringType type;
    return &type;
}

const skr_type_t* type_of<skr::string_view>::get()
{
    static StringViewType type;
    return &type;
}

size_t Hash(bool value, size_t base)
{
    return skr_hash(&value, 1, base);
}
size_t Hash(int32_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(int64_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(uint32_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(uint64_t value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(float value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_float2_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_float3_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_float4_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_float4x4_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_quaternion_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_rotator_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(double value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_guid_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_md5_t& value, size_t base)
{
    return skr_hash(&value, sizeof(value), base);
}
size_t Hash(const skr_resource_handle_t& value, size_t base)
{
    auto guid = value.get_guid();
    return skr_hash(&guid, sizeof(guid), base);
}
size_t Hash(void* value, size_t base)
{
    return skr_hash((void*)&value, sizeof(value), base);
}
size_t Hash(const skr::string& value, size_t base)
{
    return skr_hash(value.c_str(), value.size(), base);
}
size_t Hash(const skr::string_view& value, size_t base)
{
    return skr_hash(value.c_str(), value.size(), base);
}
} // namespace type
} // namespace skr