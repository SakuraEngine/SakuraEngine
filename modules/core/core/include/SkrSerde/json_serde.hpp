#pragma once
#include "SkrArchive/json/writer.h"
#include "SkrArchive/json/reader.h"
#include "SkrBase/template/concepts.hpp"
#include "SkrCore/log.h"

// traits
namespace skr
{
template <typename T>
struct JsonSerde;

// concept
template <typename T>
concept HasJsonRead = requires(skr::archive::JsonReader* r, T& t) { JsonSerde<T>::read(r, t); };
template <typename T>
concept HasJsonWrite = requires(skr::archive::JsonWriter* w, const T& t) { JsonSerde<T>::write(w, t); };

// helper
template <HasJsonRead T>
inline bool json_read(skr::archive::JsonReader* r, T& v) { return JsonSerde<T>::read(r, v); }
template <HasJsonWrite T>
inline bool json_write(skr::archive::JsonWriter* w, const T& v) { return JsonSerde<T>::write(w, v); }
} // namespace skr

// primitive types
//  bool
//  int/uint 8/16/32/64
//  float/double
namespace skr
{
template <>
struct JsonSerde<bool> {
    inline static bool read(skr::archive::JsonReader* r, bool& v)
    {
        return r->Bool(v).has_value();
    }
    inline static bool write(skr::archive::JsonWriter* w, const bool& v)
    {
        return w->Bool(v).has_value();
    }
};

template <>
struct JsonSerde<int8_t> {
    inline static bool read(skr::archive::JsonReader* r, int8_t& v)
    {
        int32_t _v;
        if (r->Int32(_v).has_value())
        {
            v = static_cast<int8_t>(_v);
            return true;
        }
        return false;
    }
    inline static bool write(skr::archive::JsonWriter* w, const int8_t& v)
    {
        return w->Int32(v).has_value();
    }
};
template <>
struct JsonSerde<int16_t> {
    inline static bool read(skr::archive::JsonReader* r, int16_t& v)
    {
        int32_t _v;
        if (r->Int32(_v).has_value())
        {
            v = static_cast<int16_t>(_v);
            return true;
        }
        return false;
    }
    inline static bool write(skr::archive::JsonWriter* w, const int16_t& v)
    {
        return w->Int32(v).has_value();
    }
};
template <>
struct JsonSerde<int32_t> {
    inline static bool read(skr::archive::JsonReader* r, int32_t& v)
    {
        return r->Int32(v).has_value();
    }
    inline static bool write(skr::archive::JsonWriter* w, const int32_t& v)
    {
        return w->Int32(v).has_value();
    }
};
template <>
struct JsonSerde<int64_t> {
    inline static bool read(skr::archive::JsonReader* r, int64_t& v)
    {
        return r->Int64(v).has_value();
    }
    inline static bool write(skr::archive::JsonWriter* w, const int64_t& v)
    {
        return w->Int64(v).has_value();
    }
};

template <>
struct JsonSerde<uint8_t> {
    inline static bool read(skr::archive::JsonReader* r, uint8_t& v)
    {
        uint32_t _v;
        if (r->UInt32(_v).has_value())
        {
            v = static_cast<uint8_t>(_v);
            return true;
        }
        return false;
    }
    inline static bool write(skr::archive::JsonWriter* w, const uint8_t& v)
    {
        return w->UInt32(v).has_value();
    }
};
template <>
struct JsonSerde<uint16_t> {
    inline static bool read(skr::archive::JsonReader* r, uint16_t& v)
    {
        uint32_t _v;
        if (r->UInt32(_v).has_value())
        {
            v = static_cast<uint16_t>(_v);
            return true;
        }
        return false;
    }
    inline static bool write(skr::archive::JsonWriter* w, const uint16_t& v)
    {
        return w->UInt32(v).has_value();
    }
};
template <>
struct JsonSerde<uint32_t> {
    inline static bool read(skr::archive::JsonReader* r, uint32_t& v)
    {
        return r->UInt32(v).has_value();
    }
    inline static bool write(skr::archive::JsonWriter* w, const uint32_t& v)
    {
        return w->UInt32(v).has_value();
    }
};
template <>
struct JsonSerde<uint64_t> {
    inline static bool read(skr::archive::JsonReader* r, uint64_t& v)
    {
        return r->UInt64(v).has_value();
    }
    inline static bool write(skr::archive::JsonWriter* w, const uint64_t& v)
    {
        return w->UInt64(v).has_value();
    }
};

template <>
struct JsonSerde<float> {
    inline static bool read(skr::archive::JsonReader* r, float& v)
    {
        return r->Float(v).has_value();
    }
    inline static bool write(skr::archive::JsonWriter* w, const float& v)
    {
        return w->Float(v).has_value();
    }
};
template <>
struct JsonSerde<double> {
    inline static bool read(skr::archive::JsonReader* r, double& v)
    {
        return r->Double(v).has_value();
    }
    inline static bool write(skr::archive::JsonWriter* w, const double& v)
    {
        return w->Double(v).has_value();
    }
};
} // namespace skr

// enum & array
namespace skr
{
template <concepts::Enum T>
struct JsonSerde<T> {
};

template <typename T, size_t N>
struct JsonSerde<T[N]> {
    inline static bool read(skr::archive::JsonReader* r, T (&v)[N])
    {
        size_t count;
        r->StartArray(count);
        size_t i = 0;
        for (i = 0; i < count; ++i)
        {
            if (i > N)
            {
                SKR_LOG_WARN(u8"[SERDE/JSON] got too many elements (%d expected, given %d), ignoring overflowed elements", N, i);
                break;
            }
            if (!JsonSerde<T>::read(r, v[i]))
            {
                SKR_LOG_ERROR(u8"[SERDE/JSON] Archive [%d] failed: %s", i, "UNKNOWN ERROR"); // TODO: ERROR MESSAGE
                return false;
            }
        }
        r->EndArray();

        if (i < N)
        {
            SKR_LOG_WARN(u8"[SERDE/JSON] got too few elements (%d expected, given %d), using default value", N, i);
        }
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const T (&v)[N])
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false)
        for (int i = 0; i < N; ++i)
        {
            bool _x = JsonSerde<T>::write(w, v[i]);
            if (!_x) return false;
        }
        SKR_EXPECTED_CHECK(w->EndArray(), false)
        return true;
    }
};
} // namespace skr

// skr types, 主要指 skr base 中的类型，遵循模块依赖的规则
//  float2/float3/float4
//  float4x4/rotator/quaternion
//  guid/md5
namespace skr
{
template <>
struct JsonSerde<skr_float2_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_float2_t& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 2)
            return false;
        SKR_EXPECTED_CHECK(r->Float(v.x), false);
        SKR_EXPECTED_CHECK(r->Float(v.y), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_float2_t& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Float(v.x), false);
        SKR_EXPECTED_CHECK(w->Float(v.y), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
template <>
struct JsonSerde<skr_float3_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_float3_t& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 3)
            return false;
        SKR_EXPECTED_CHECK(r->Float(v.x), false);
        SKR_EXPECTED_CHECK(r->Float(v.y), false);
        SKR_EXPECTED_CHECK(r->Float(v.z), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_float3_t& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Float(v.x), false);
        SKR_EXPECTED_CHECK(w->Float(v.y), false);
        SKR_EXPECTED_CHECK(w->Float(v.z), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
template <>
struct JsonSerde<skr_float4_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_float4_t& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 4)
            return false;
        SKR_EXPECTED_CHECK(r->Float(v.x), false);
        SKR_EXPECTED_CHECK(r->Float(v.y), false);
        SKR_EXPECTED_CHECK(r->Float(v.z), false);
        SKR_EXPECTED_CHECK(r->Float(v.w), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_float4_t& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Float(v.x), false);
        SKR_EXPECTED_CHECK(w->Float(v.y), false);
        SKR_EXPECTED_CHECK(w->Float(v.z), false);
        SKR_EXPECTED_CHECK(w->Float(v.w), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
template <>
struct JsonSerde<skr_float4x4_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_float4x4_t& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 16)
            return false;
        SKR_EXPECTED_CHECK(r->Float(v.M[0][0]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[0][1]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[0][2]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[0][3]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[1][0]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[1][1]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[1][2]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[1][3]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[2][0]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[2][1]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[2][2]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[2][3]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[3][0]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[3][1]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[3][2]), false);
        SKR_EXPECTED_CHECK(r->Float(v.M[3][3]), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_float4x4_t& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[0][0]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[0][1]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[0][2]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[0][3]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[1][0]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[1][1]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[1][2]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[1][3]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[2][0]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[2][1]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[2][2]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[2][3]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[3][0]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[3][1]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[3][2]), false);
        SKR_EXPECTED_CHECK(w->Float(v.M[3][3]), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
template <>
struct JsonSerde<skr_rotator_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_rotator_t& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 3)
            return false;
        SKR_EXPECTED_CHECK(r->Float(v.pitch), false);
        SKR_EXPECTED_CHECK(r->Float(v.yaw), false);
        SKR_EXPECTED_CHECK(r->Float(v.roll), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_rotator_t& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Float(v.pitch), false);
        SKR_EXPECTED_CHECK(w->Float(v.yaw), false);
        SKR_EXPECTED_CHECK(w->Float(v.roll), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
template <>
struct JsonSerde<skr_quaternion_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_quaternion_t& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 4)
            return false;
        SKR_EXPECTED_CHECK(r->Float(v.x), false);
        SKR_EXPECTED_CHECK(r->Float(v.y), false);
        SKR_EXPECTED_CHECK(r->Float(v.z), false);
        SKR_EXPECTED_CHECK(r->Float(v.w), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_quaternion_t& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Float(v.x), false);
        SKR_EXPECTED_CHECK(w->Float(v.y), false);
        SKR_EXPECTED_CHECK(w->Float(v.z), false);
        SKR_EXPECTED_CHECK(w->Float(v.w), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
template <>
struct JsonSerde<skr_guid_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_guid_t& v)
    {
        skr::String str;
        if (r->String(str).has_value())
        {
            if (!skr::guid_from_sv(str.u8_str(), v))
                return false;
            return true;
        }
        return false;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_guid_t& v)
    {
        auto str = skr::format(u8"{}", v);
        SKR_EXPECTED_CHECK(w->String(str), false);
        return true;
    }
};
template <>
struct JsonSerde<skr_md5_t> {
    inline static bool read(skr::archive::JsonReader* r, skr_md5_t& v)
    {
        skr::String str;
        if (r->String(str).has_value())
        {
            if (!skr_parse_md5(str.u8_str(), &v))
                return false;
            return true;
        }
        return false;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr_md5_t& v)
    {
        auto str = skr::format(u8"{}", v);
        SKR_EXPECTED_CHECK(w->String(str), false);
        return true;
    }
};
} // namespace skr