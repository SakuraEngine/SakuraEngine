#pragma once
#include "SkrArchive/json/writer.h"
#include "SkrArchive/json/reader.h"
#include "SkrBase/template/concepts.hpp"
#include "SkrCore/log.h"
#include "SkrSerde/enum_serde_traits.hpp"
#include "SkrRTTR/rttr_traits.hpp"

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

#if SKR_PLAT_MACOSX
template <>
struct JsonSerde<size_t> {
    inline static bool read(skr::archive::JsonReader* r, size_t& v)
    {
        uint64_t _v;
        SKR_EXPECTED_CHECK(r->UInt64(_v), false);
        v = _v;
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const size_t& v)
    {
        uint64_t _v = v;
        SKR_EXPECTED_CHECK(w->UInt64(_v), false);
        return true;
    }
};
#endif

} // namespace skr

// enum & array
namespace skr
{
template <concepts::Enum T>
struct JsonSerde<T> {
    inline static bool read(skr::archive::JsonReader* r, T& v)
    {
        SkrZoneScopedN("JsonSerde<concepts::Enum>::read");
        skr::String enumStr;
        if (r->String(enumStr).has_value())
        {
            if (!EnumSerdeTraits<T>::from_string(enumStr.view(), v))
            {
                SKR_LOG_ERROR(u8"Unknown enumerator while reading enum %s: %s", skr::type_name_of<T>().data(), enumStr.data());
                return false;
            }
            return true;
        }
        return false;
    }
    inline static bool write(skr::archive::JsonWriter* w, const T& v)
    {
        SkrZoneScopedN("JsonSerde<concepts::Enum>::write");
        SKR_EXPECTED_CHECK(w->String(EnumSerdeTraits<T>::to_string(v)), false);
        return true;
    }
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

//  guid/md5
namespace skr
{
template <>
struct JsonSerde<GUID> {
    inline static bool read(skr::archive::JsonReader* r, skr_guid_t& v)
    {
        skr::String str;
        if (r->String(str).has_value())
        {
            if (!skr::guid_from_sv(str.c_str(), v))
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
struct JsonSerde<MD5> {
    inline static bool read(skr::archive::JsonReader* r, skr_md5_t& v)
    {
        skr::String str;
        if (r->String(str).has_value())
        {
            if (!skr_parse_md5(str.c_str(), &v))
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

// math misc
namespace skr
{
// rotator
template <>
struct JsonSerde<skr::RotatorF> {
    inline static bool read(skr::archive::JsonReader* r, skr::RotatorF& v)
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
    inline static bool write(skr::archive::JsonWriter* w, const skr::RotatorF& v)
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
struct JsonSerde<skr::RotatorD> {
    inline static bool read(skr::archive::JsonReader* r, skr::RotatorD& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 3)
            return false;
        SKR_EXPECTED_CHECK(r->Double(v.pitch), false);
        SKR_EXPECTED_CHECK(r->Double(v.yaw), false);
        SKR_EXPECTED_CHECK(r->Double(v.roll), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr::RotatorD& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Double(v.pitch), false);
        SKR_EXPECTED_CHECK(w->Double(v.yaw), false);
        SKR_EXPECTED_CHECK(w->Double(v.roll), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};

// quat
template <>
struct JsonSerde<skr::QuatF> {
    inline static bool read(skr::archive::JsonReader* r, skr::QuatF& v)
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
    inline static bool write(skr::archive::JsonWriter* w, const skr::QuatF& v)
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
struct JsonSerde<skr::QuatD> {
    inline static bool read(skr::archive::JsonReader* r, skr::QuatD& v)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 4)
            return false;
        SKR_EXPECTED_CHECK(r->Double(v.x), false);
        SKR_EXPECTED_CHECK(r->Double(v.y), false);
        SKR_EXPECTED_CHECK(r->Double(v.z), false);
        SKR_EXPECTED_CHECK(r->Double(v.w), false);
        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr::QuatD& v)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);
        SKR_EXPECTED_CHECK(w->Double(v.x), false);
        SKR_EXPECTED_CHECK(w->Double(v.y), false);
        SKR_EXPECTED_CHECK(w->Double(v.z), false);
        SKR_EXPECTED_CHECK(w->Double(v.w), false);
        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};

// transform
template <>
struct JsonSerde<skr::TransformF> {
    inline static bool read(skr::archive::JsonReader* r, skr::TransformF& t)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 4)
            return false;

        SKR_EXPECTED_CHECK(r->Float(t.rotation.x), false);
        SKR_EXPECTED_CHECK(r->Float(t.rotation.y), false);
        SKR_EXPECTED_CHECK(r->Float(t.rotation.z), false);
        SKR_EXPECTED_CHECK(r->Float(t.rotation.w), false);

        SKR_EXPECTED_CHECK(r->Float(t.position.x), false);
        SKR_EXPECTED_CHECK(r->Float(t.position.y), false);
        SKR_EXPECTED_CHECK(r->Float(t.position.z), false);

        SKR_EXPECTED_CHECK(r->Float(t.scale.x), false);
        SKR_EXPECTED_CHECK(r->Float(t.scale.y), false);
        SKR_EXPECTED_CHECK(r->Float(t.scale.z), false);

        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr::TransformF& t)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);

        SKR_EXPECTED_CHECK(w->Float(t.rotation.x), false);
        SKR_EXPECTED_CHECK(w->Float(t.rotation.y), false);
        SKR_EXPECTED_CHECK(w->Float(t.rotation.z), false);
        SKR_EXPECTED_CHECK(w->Float(t.rotation.w), false);

        SKR_EXPECTED_CHECK(w->Float(t.position.x), false);
        SKR_EXPECTED_CHECK(w->Float(t.position.y), false);
        SKR_EXPECTED_CHECK(w->Float(t.position.z), false);

        SKR_EXPECTED_CHECK(w->Float(t.scale.x), false);
        SKR_EXPECTED_CHECK(w->Float(t.scale.y), false);
        SKR_EXPECTED_CHECK(w->Float(t.scale.z), false);

        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
template <>
struct JsonSerde<skr::TransformD> {
    inline static bool read(skr::archive::JsonReader* r, skr::TransformD& t)
    {
        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != 4)
            return false;

        SKR_EXPECTED_CHECK(r->Double(t.rotation.x), false);
        SKR_EXPECTED_CHECK(r->Double(t.rotation.y), false);
        SKR_EXPECTED_CHECK(r->Double(t.rotation.z), false);
        SKR_EXPECTED_CHECK(r->Double(t.rotation.w), false);

        SKR_EXPECTED_CHECK(r->Double(t.position.x), false);
        SKR_EXPECTED_CHECK(r->Double(t.position.y), false);
        SKR_EXPECTED_CHECK(r->Double(t.position.z), false);

        SKR_EXPECTED_CHECK(r->Double(t.scale.x), false);
        SKR_EXPECTED_CHECK(r->Double(t.scale.y), false);
        SKR_EXPECTED_CHECK(r->Double(t.scale.z), false);

        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr::TransformD& t)
    {
        SKR_EXPECTED_CHECK(w->StartArray(), false);

        SKR_EXPECTED_CHECK(w->Double(t.rotation.x), false);
        SKR_EXPECTED_CHECK(w->Double(t.rotation.y), false);
        SKR_EXPECTED_CHECK(w->Double(t.rotation.z), false);
        SKR_EXPECTED_CHECK(w->Double(t.rotation.w), false);

        SKR_EXPECTED_CHECK(w->Double(t.position.x), false);
        SKR_EXPECTED_CHECK(w->Double(t.position.y), false);
        SKR_EXPECTED_CHECK(w->Double(t.position.z), false);

        SKR_EXPECTED_CHECK(w->Double(t.scale.x), false);
        SKR_EXPECTED_CHECK(w->Double(t.scale.y), false);
        SKR_EXPECTED_CHECK(w->Double(t.scale.z), false);

        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};
} // namespace skr

// math vector & matrix
namespace skr
{
template <MathVector T>
struct JsonSerde<T> {
    inline static bool read(skr::archive::JsonReader* r, T& v)
    {
        constexpr auto dim = MathVectorTraits<T>::kDimensions;
        using CompType     = typename MathVectorTraits<T>::ComponentType;
        using CompSerde    = JsonSerde<CompType>;

        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != dim)
            return false;

        for (size_t i = 0; i < dim; ++i)
        {
            if (!CompSerde::read(r, v[i])) { return false; }
        }

        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const T& v)
    {
        constexpr auto dim = MathVectorTraits<T>::kDimensions;
        using CompType     = typename MathVectorTraits<T>::ComponentType;
        using CompSerde    = JsonSerde<CompType>;

        SKR_EXPECTED_CHECK(w->StartArray(), false);

        for (size_t i = 0; i < dim; ++i)
        {
            if (!CompSerde::write(w, v[i])) { return false; }
        }

        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};

template <MathMatrix T>
struct JsonSerde<T> {
    inline static bool read(skr::archive::JsonReader* r, T& v)
    {
        constexpr auto dim = MathMatrixTraits<T>::kDimensions;
        using CompType     = typename MathMatrixTraits<T>::ComponentType;
        using CompSerde    = JsonSerde<CompType>;

        size_t count;
        SKR_EXPECTED_CHECK(r->StartArray(count), false);
        if (count != dim)
            return false;

        for (size_t row_idx = 0; row_idx < dim; ++row_idx)
        {
            for (size_t col_idx = 0; col_idx < dim; ++col_idx)
            {
                if (!CompSerde::read(r, v.rows[row_idx][col_idx])) { return false; }
            }
        }

        SKR_EXPECTED_CHECK(r->EndArray(), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const T& v)
    {
        constexpr auto dim = MathMatrixTraits<T>::kDimensions;
        using CompType     = typename MathMatrixTraits<T>::ComponentType;
        using CompSerde    = JsonSerde<CompType>;

        SKR_EXPECTED_CHECK(w->StartArray(), false);

        for (size_t row_idx = 0; row_idx < dim; ++row_idx)
        {
            for (size_t col_idx = 0; col_idx < dim; ++col_idx)
            {
                if (!CompSerde::write(w, v.rows[row_idx][col_idx])) { return false; }
            }
        }

        SKR_EXPECTED_CHECK(w->EndArray(), false);
        return true;
    }
};

} // namespace skr