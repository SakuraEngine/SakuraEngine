#pragma once
#include "SkrArchive/json/writer.h"
#include "SkrArchive/json/reader.h"
#include "SkrBase/template/concepts.hpp"

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
};
} // namespace skr

// skr types, 主要指 skr base 中的类型，遵循模块依赖的规则
//  float2/float3/float4
//  float4x4/rotator/quaternion
//  guid/md5
namespace skr
{
}