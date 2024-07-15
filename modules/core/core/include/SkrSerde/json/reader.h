#pragma once
#include "SkrBase/types.h"
#include "SkrArchive/json/reader.h"
#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/vector.hpp"

// helper functions
namespace skr::json
{
template <class T>
bool Read(skr::archive::JsonReader* json, T& value);

template <class T, class = void>
struct ReadTrait;

template <typename T>
inline static constexpr bool HasReadTrait = requires(skr::archive::JsonReader* r, T& t) { ReadTrait<T>::Read(t, t); };
} // namespace skr::json

namespace skr::json
{
// primitive types
// bool
// int/uint 8/16/32/64
// float/double
template <>
struct SKR_STATIC_API ReadTrait<bool> {
    static bool Read(skr::archive::JsonReader* json, bool& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int8_t> {
    static bool Read(skr::archive::JsonReader* json, int8_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int16_t> {
    static bool Read(skr::archive::JsonReader* json, int16_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int32_t> {
    static bool Read(skr::archive::JsonReader* json, int32_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int64_t> {
    static bool Read(skr::archive::JsonReader* json, int64_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint8_t> {
    static bool Read(skr::archive::JsonReader* json, uint8_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint16_t> {
    static bool Read(skr::archive::JsonReader* json, uint16_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint32_t> {
    static bool Read(skr::archive::JsonReader* json, uint32_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<uint64_t> {
    static bool Read(skr::archive::JsonReader* json, uint64_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<float> {
    static bool Read(skr::archive::JsonReader* json, float& value);
};
template <>
struct SKR_STATIC_API ReadTrait<double> {
    static bool Read(skr::archive::JsonReader* json, double& value);
};
} // namespace skr::json

// skr type
namespace skr::json
{
template <>
struct SKR_STATIC_API ReadTrait<skr_float2_t> {
    static bool Read(skr::archive::JsonReader* json, skr_float2_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float3_t> {
    static bool Read(skr::archive::JsonReader* json, skr_float3_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float4_t> {
    static bool Read(skr::archive::JsonReader* json, skr_float4_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float4x4_t> {
    static bool Read(skr::archive::JsonReader* json, skr_float4x4_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_rotator_t> {
    static bool Read(skr::archive::JsonReader* json, skr_rotator_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_quaternion_t> {
    static bool Read(skr::archive::JsonReader* json, skr_quaternion_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_guid_t> {
    static bool Read(skr::archive::JsonReader* json, skr_guid_t& value);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_md5_t> {
    static bool Read(skr::archive::JsonReader* json, skr_md5_t& md5);
};
} // namespace skr::json

// helper functions impl
namespace skr::json
{
template <class T>
bool Read(skr::archive::JsonReader* json, T& value)
{
    return ReadTrait<T>::Read(json, value);
}
} // namespace skr::json
