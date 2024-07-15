#pragma once
#include "SkrArchive/json/writer.h"

// helper functions
namespace skr::json
{
template <class T>
bool Write(skr::archive::JsonWriter* writer, const T& value);
template <class T, class = void>
struct WriteTrait;
template <typename T>
inline static constexpr bool HasWriteTrait = requires(skr::archive::JsonWriter* w, T& t) { WriteTrait<T>::Write(w, t); };
} // namespace skr::json

// primitive types
// bool
// int/uint 8/16/32/64
// float/double
namespace skr::json
{
template <>
struct SKR_STATIC_API WriteTrait<bool> {
    static bool Write(skr::archive::JsonWriter* writer, bool b);
};

// int
template <>
struct SKR_STATIC_API WriteTrait<int8_t> {
    static bool Write(skr::archive::JsonWriter* writer, int8_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int16_t> {
    static bool Write(skr::archive::JsonWriter* writer, int16_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int32_t> {
    static bool Write(skr::archive::JsonWriter* writer, int32_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<int64_t> {
    static bool Write(skr::archive::JsonWriter* writer, int64_t i);
};

// uint
template <>
struct SKR_STATIC_API WriteTrait<uint8_t> {
    static bool Write(skr::archive::JsonWriter* writer, uint8_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint16_t> {
    static bool Write(skr::archive::JsonWriter* writer, uint16_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint32_t> {
    static bool Write(skr::archive::JsonWriter* writer, uint32_t i);
};
template <>
struct SKR_STATIC_API WriteTrait<uint64_t> {
    static bool Write(skr::archive::JsonWriter* writer, uint64_t i);
};

// float
template <>
struct SKR_STATIC_API WriteTrait<float> {
    static bool Write(skr::archive::JsonWriter* writer, float f);
};
template <>
struct SKR_STATIC_API WriteTrait<double> {
    static bool Write(skr::archive::JsonWriter* writer, double d);
};

template <>
struct SKR_STATIC_API WriteTrait<skr_float2_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_float2_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float3_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_float3_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_float4_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4x4_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_float4x4_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_rotator_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_rotator_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_quaternion_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_quaternion_t& v);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_guid_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_guid_t& guid);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_md5_t> {
    static bool Write(skr::archive::JsonWriter* writer, const skr_md5_t& md5);
};
} // namespace skr::json

// help function impl
namespace skr::json
{
template <class T>
bool Write(skr::archive::JsonWriter* writer, const T& value)
{
    return WriteTrait<T>::Write(writer, value);
}
} // namespace skr::json