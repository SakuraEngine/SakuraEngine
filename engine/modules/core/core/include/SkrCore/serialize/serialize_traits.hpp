#pragma once
#include <SkrCore/serialize/fwd.hpp>
#include <SkrCore/serialize/archive.hpp>
#include <SkrBase/config.h>
#include <SkrBase/types.h>
#include <SkrBase/type_info.hpp>
#include <SkrBase/math.hpp>

// help functions
namespace skr
{
template <typename  T>
inline void serde_read(ArchiveRead& r, T& v)
{
    Serialize<T>::read(r, v);
}
template <typename T>
inline void serde_write(ArchiveWrite& w, const T& v)
{
    Serialize<T>::write(w, v);
}
template <typename T>
inline void serde_read_fields(ArchiveRead& r, T& v)
{
    Serialize<T>::read_fields(r, v);
}
template <typename T>
inline void serde_write_fields(ArchiveWrite& w, const T& v)
{
    Serialize<T>::write_fields(w, v);
}
} // namespace skr

// primitive types serde
namespace skr
{
#define SKR_SERDE_PRIMITIVE(__TYPE, __ENUM)                        \
    template <>                                                    \
    struct Serialize<__TYPE>                                       \
    {                                                              \
        inline static void read(ArchiveRead& r, __TYPE& v)         \
        {                                                          \
            r.primitive(EArchivePrimitiveType::__ENUM, &v);        \
        }                                                          \
        inline static void write(ArchiveWrite& w, const __TYPE& v) \
        {                                                          \
            w.primitive(EArchivePrimitiveType::__ENUM, &v);        \
        }                                                          \
    };

SKR_SERDE_PRIMITIVE(bool, Bool);

SKR_SERDE_PRIMITIVE(uint8_t, UInt8);
SKR_SERDE_PRIMITIVE(uint16_t, UInt16);
SKR_SERDE_PRIMITIVE(uint32_t, UInt32);
SKR_SERDE_PRIMITIVE(uint64_t, UInt64);

SKR_SERDE_PRIMITIVE(int8_t, Int8);
SKR_SERDE_PRIMITIVE(int16_t, Int16);
SKR_SERDE_PRIMITIVE(int32_t, Int32);
SKR_SERDE_PRIMITIVE(int64_t, Int64);

SKR_SERDE_PRIMITIVE(float, Float);
SKR_SERDE_PRIMITIVE(double, Double);

#undef SKR_SERDE_PRIMITIVE
} // namespace skr

// enum & array
namespace skr
{
template <concepts::Enum T>
struct Serialize<T>
{
    inline static void read(ArchiveRead& r, T& v)
    {
        if (r.enable_string_enum())
        {
            StringView enum_str;
            SKR_FAST_CHECK(r.str_view(enum_str), );
            if (!TypeInfo<T>::from_string(enum_str, v))
            {
                r.error(
                    u8"Unknown value '{}' while reading enum of type '{}'",
                    enum_str,
                    skr::type_name_of<T>()
                );
            }
        }
        else
        {
            using UT = std::underlying_type_t<T>;
            Serialize<UT>::read(r, reinterpret_cast<UT&>(v));
        }
    }
    inline static void write(ArchiveWrite& w, const T& v)
    {
        if (w.enable_string_enum())
        {
            w.str(TypeInfo<T>::to_string(v));
        }
        else
        {
            using UT = std::underlying_type_t<T>;
            Serialize<UT>::write(w, reinterpret_cast<const UT&>(v));
        }
    }
};
template <typename T, size_t N>
struct Serialize<T[N]>
{
    inline static void read(ArchiveRead& r, T (&v)[N])
    {
        if (r.is_structured())
        {
            Archive::ArrayScope arr_scope{ r };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            uint64_t arr_count, adjusted_count;
            SKR_FAST_CHECK(r.array_size_structured(arr_count), );
            SKR_FAST_CHECK(r.adjust_array_size(arr_count, N, adjusted_count), );
            for (uint64_t i = 0; i < adjusted_count; ++i)
            {
                SKR_FAST_CHECK(r.value<T>(v[i]), );
            }
        }
        else
        {
            for (size_t i = 0; i < N; ++i)
            {
                SKR_FAST_CHECK(r.value<T>(v[i]), );
            }
        }
    }
    inline static void write(ArchiveWrite& w, const T (&v)[N])
    {
        if (w.is_structured())
        {
            Archive::ArrayScope arr_scope{ w };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            for (uint64_t i = 0; i < N; ++i)
            {
                SKR_FAST_CHECK(w.value<T>(v[i]), );
            }
        }
        else
        {
            for (size_t i = 0; i < N; ++i)
            {
                SKR_FAST_CHECK(w.value<T>(v[i]), );
            }
        }
    }
};
} // namespace skr

// guid/md5/sha256
namespace skr
{
template <>
struct Serialize<GUID>
{
    inline static void read(ArchiveRead& r, GUID& v)
    {
        if (r.enable_string_guid())
        {
            // read string
            StringView guid_str;
            SKR_FAST_CHECK(r.str_view(guid_str), );

            // parse
            auto opt_guid = GUID::DecodeAuto(guid_str.data(), guid_str.size());
            if (!opt_guid.has_value()) [[unlikely]]
            {
                r.error(
                    u8"failed to parse guid from string '{}'",
                    guid_str
                );
            }
            else
            {
                v = opt_guid.value();
            }
        }
        else
        {
            // // per component read
            // uint32_t time_low;
            // uint16_t time_mid, time_high_and_version;
            // uint8_t clock_and_nodes[8];
            // SKR_FAST_CHECK(r.value<uint32_t>(time_low), );
            // SKR_FAST_CHECK(r.value<uint16_t>(time_mid), );
            // SKR_FAST_CHECK(r.value<uint16_t>(time_high_and_version), );
            // SKR_FAST_CHECK(r.value<uint8_t>(clock_and_nodes[0]), );
            // SKR_FAST_CHECK(r.value<uint8_t>(clock_and_nodes[1]), );
            // for (size_t i = 0; i < 6; ++i)
            //     SKR_FAST_CHECK(r.value<uint8_t>(clock_and_nodes[2 + i]), );
            // v = GUID(time_low, time_mid, time_high_and_version, clock_and_nodes);
            SKR_FAST_CHECK(r.bytes(&v, sizeof(v)), );
        }
    }
    inline static void write(ArchiveWrite& w, const GUID& v)
    {
        if (w.enable_string_guid())
        {
            // encode
            char8_t encode_buffer[GUID::kMaxEncodedLength];
            auto encode_len = v.encode_length(GUID::EStyle::HexSpec);
            v.encode(encode_buffer, GUID::EStyle::HexSpec);

            // write
            w.str({ encode_buffer, encode_len });
        }
        else
        {
            // // per component write
            // w.value<uint32_t>(v.time_low());
            // w.value<uint16_t>(v.time_mid());
            // w.value<uint16_t>(v.time_high_and_version());
            // w.value<uint8_t>(v.clock_seq_and_reserved());
            // w.value<uint8_t>(v.clock_seq_low());
            // for (size_t i = 0; i < 6; ++i)
            //     w.value<uint8_t>(v.node(i));
            SKR_FAST_CHECK(w.bytes(&v, sizeof(v)), );
        }
    }
};
template <>
struct Serialize<MD5>
{
    inline static void read(ArchiveRead& r, MD5& v)
    {
        if (r.enable_string_md5())
        {
            // read string
            StringView md5_str;
            SKR_FAST_CHECK(r.str_view(md5_str), );

            // parse
            auto opt_md5 = MD5::DecodeAuto(md5_str.data(), md5_str.size());
            if (!opt_md5) [[unlikely]]
            {
                r.error(
                    u8"failed to parse md5 from string '{}'",
                    md5_str
                );
            }
            else
            {
                v = *opt_md5;
            }
        }
        else
        {
            r.bytes(&v, sizeof(v));
        }
    }
    inline static void write(ArchiveWrite& w, const MD5& v)
    {
        if (w.enable_string_md5())
        {
            // encode
            char8_t encode_buffer[MD5::kMaxEncodedLength];
            auto encode_len = v.encode_length(MD5::EStyle::Hex);
            v.encode(encode_buffer, MD5::EStyle::Hex);

            // write
            w.str({ encode_buffer, encode_len });
        }
        else
        {
            w.bytes(&v, sizeof(v));
        }
    }
};
template <>
struct Serialize<SHA256>
{
    inline static void read(ArchiveRead& r, SHA256& v)
    {
        if (r.enable_string_sha256())
        {
            // read string
            StringView sha256_str;
            SKR_FAST_CHECK(r.str_view(sha256_str), );

            // parse
            auto opt_sha256 = SHA256::DecodeAuto(sha256_str.data(), sha256_str.size());
            if (!opt_sha256) [[unlikely]]
            {
                r.error(
                    u8"failed to parse sha256 from string '{}'",
                    sha256_str
                );
            }
            else
            {
                v = *opt_sha256;
            }
        }
        else
        {
            r.bytes(&v, sizeof(v));
        }
    }
    inline static void write(ArchiveWrite& w, const SHA256& v)
    {
        if (w.enable_string_sha256())
        {
            // encode
            char8_t encode_buffer[SHA256::kMaxEncodedLength];
            auto encode_len = v.encode_length(SHA256::EStyle::Hex);
            v.encode(encode_buffer, SHA256::EStyle::Hex);

            // write
            w.str({ encode_buffer, encode_len });
        }
        else
        {
            w.bytes(&v, sizeof(v));
        }
    }
};
} // namespace skr

// math vector & matrix
namespace skr
{
template <MathVector T>
struct Serialize<T>
{
    inline static constexpr auto kDim = MathVectorTraits<T>::kDimensions;
    using CompType = typename MathVectorTraits<T>::ComponentType;

    inline static void read(ArchiveRead& r, T& v)
    {
        if (r.is_structured())
        {
            Archive::ArrayScope arr_scope{ r };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            uint64_t arr_count, adjusted_count;
            SKR_FAST_CHECK(r.array_size_structured(arr_count), );
            SKR_FAST_CHECK(r.adjust_array_size(arr_count, kDim, adjusted_count), );
            for (uint64_t i = 0; i < adjusted_count; ++i)
            {
                SKR_FAST_CHECK(r.value<CompType>(v[i]), );
            }
        }
        else
        {
            r.bytes(&v, uint64_t(sizeof(T)));
        }
    }
    inline static void write(ArchiveWrite& w, const T& v)
    {
        if (w.is_structured())
        {
            Archive::ArrayScope arr_scope{ w };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            for (uint64_t i = 0; i < kDim; ++i)
            {
                SKR_FAST_CHECK(w.value<CompType>(v[i]), );
            }
        }
        else
        {
            w.bytes(&v, uint64_t(sizeof(T)));
        }
    }
};
template <MathMatrix T>
struct Serialize<T>
{
    inline static constexpr auto kDim = MathMatrixTraits<T>::kDimensions;
    inline static constexpr auto kCount = kDim * kDim;
    using CompType = typename MathMatrixTraits<T>::ComponentType;

    inline static void read(ArchiveRead& r, T& v)
    {
        if (r.is_structured())
        {
            Archive::ArrayScope arr_scope{ r };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            CompType* as_array = reinterpret_cast<CompType*>(&v);
            uint64_t arr_count, adjusted_count;
            SKR_FAST_CHECK(r.array_size_structured(arr_count), );
            SKR_FAST_CHECK(r.adjust_array_size(arr_count, kCount, adjusted_count), );
            for (uint64_t i = 0; i < adjusted_count; ++i)
            {
                SKR_FAST_CHECK(r.value<CompType>(as_array[i]), );
            }
        }
        else
        {
            SKR_FAST_CHECK(r.bytes(&v, uint64_t(sizeof(T))), );
        }
    }
    inline static void write(ArchiveWrite& w, const T& v)
    {
        if (w.is_structured())
        {
            Archive::ArrayScope arr_scope{ w };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            const CompType* as_array = reinterpret_cast<const CompType*>(&v);
            for (uint64_t i = 0; i < kCount; ++i)
            {
                SKR_FAST_CHECK(w.value<CompType>(as_array[i]), );
            }
        }
        else
        {
            SKR_FAST_CHECK(w.bytes(&v, uint64_t(sizeof(T))), );
        }
    }
};
} // namespace skr

// math misc
namespace skr
{
// rotator
template <>
struct Serialize<RotatorF>
{
    using ArrayType = float[3];
    inline static void read(ArchiveRead& r, RotatorF& v)
    {
        r.value(v.as_vector());
    }
    inline static void write(ArchiveWrite& w, const RotatorF& v)
    {
        w.value(v.as_vector());
    }
};
template <>
struct Serialize<RotatorD>
{
    inline static void read(ArchiveRead& r, RotatorD& v)
    {
        r.value(v.as_vector());
    }
    inline static void write(ArchiveWrite& w, const RotatorD& v)
    {
        w.value(v.as_vector());
    }
};

// quaternion
template <>
struct Serialize<QuatF>
{
    inline static void read(ArchiveRead& r, QuatF& v)
    {
        r.value(v.as_vector());
    }
    inline static void write(ArchiveWrite& w, const QuatF& v)
    {
        w.value(v.as_vector());
    }
};
template <>
struct Serialize<QuatD>
{
    using ArrayType = double[4];
    inline static void read(ArchiveRead& r, QuatD& v)
    {
        r.value(*reinterpret_cast<ArrayType*>(&v));
    }
    inline static void write(ArchiveWrite& w, const QuatD& v)
    {
        w.value(*reinterpret_cast<const ArrayType*>(&v));
    }
};

// transform
template <>
struct Serialize<TransformF>
{
    inline static void read(ArchiveRead& r, TransformF& v)
    {
        if (r.is_structured())
        {
            Archive::ObjectScope _obj_scope{ r };
            SKR_FAST_CHECK(r.key_value(u8"rotation", v.rotation), );
            SKR_FAST_CHECK(r.key_value(u8"position", v.position), );
            SKR_FAST_CHECK(r.key_value(u8"scale", v.scale), );
        }
        else
        {
            SKR_FAST_CHECK(r.bytes(&v, sizeof(v)), );
        }
    }
    inline static void write(ArchiveWrite& w, const TransformF& v)
    {
        if (w.is_structured())
        {
            Archive::ObjectScope _obj_scope{ w };
            SKR_FAST_CHECK(w.key_value(u8"rotation", v.rotation), );
            SKR_FAST_CHECK(w.key_value(u8"position", v.position), );
            SKR_FAST_CHECK(w.key_value(u8"scale", v.scale), );
        }
        else
        {
            SKR_FAST_CHECK(w.bytes(&v, sizeof(v)), );
        }
    }
};
template <>
struct Serialize<TransformD>
{
    inline static void read(ArchiveRead& r, TransformD& v)
    {
        if (r.is_structured())
        {
            Archive::ObjectScope _obj_scope{ r };
            SKR_FAST_CHECK(r.key_value(u8"rotation", v.rotation), );
            SKR_FAST_CHECK(r.key_value(u8"position", v.position), );
            SKR_FAST_CHECK(r.key_value(u8"scale", v.scale), );
        }
        else
        {
            SKR_FAST_CHECK(r.bytes(&v, sizeof(v)), );
        }
    }
    inline static void write(ArchiveWrite& w, const TransformD& v)
    {
        if (w.is_structured())
        {
            Archive::ObjectScope _obj_scope{ w };
            SKR_FAST_CHECK(w.key_value(u8"rotation", v.rotation), );
            SKR_FAST_CHECK(w.key_value(u8"position", v.position), );
            SKR_FAST_CHECK(w.key_value(u8"scale", v.scale), );
        }
        else
        {
            SKR_FAST_CHECK(w.bytes(&v, sizeof(v)), );
        }
    }
};
} // namespace skr