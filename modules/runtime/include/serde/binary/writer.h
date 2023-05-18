#pragma once
#include "serde/binary/writer_fwd.h"
#include "utils/types.h"
#include "type/type_helper.hpp"
#include <bitset>
#include "utils/traits.hpp"
#include "serde/binary/serde.h"

struct skr_binary_writer_t {
    template <class T>
    skr_binary_writer_t(T& user)
    {
        user_data = &user;
        vwrite = [](void* user, const void* data, size_t size) -> int {
            return static_cast<T*>(user)->write(data, size);
        };
        auto SupportBitPacking = SKR_VALIDATOR((auto t), t.write_bits((void*)0, (size_t)0));
        if constexpr(SupportBitPacking(SKR_TYPELIST(T)))
        {
            vwrite_bits = [](void* user, const void* data, size_t size) -> int {
                return static_cast<T*>(user)->write_bits(data, size);
            };
        }
    }
    int (*vwrite)(void* user_data, const void* data, size_t size) = nullptr;
    int (*vwrite_bits)(void* user_data, const void* data, size_t size) = nullptr;
    void* user_data = nullptr;
    int write(const void* data, size_t size)
    {
        return vwrite(user_data, data, size);
    }
    int write_bits(const void* data, size_t size)
    {
        return vwrite_bits(user_data, data, size);
    }
};

namespace skr::binary
{
inline int WriteBytes(skr_binary_writer_t* writer, const void* data, size_t size)
{
    return writer->write(data, size);
}

template <class T, class ...Args>
int Write(skr_binary_writer_t* writer, const T& value, Args&&... args);
template <class T, class ...Args>
int Archive(skr_binary_writer_t* writer, const T& value, Args&&... args);
template <class T, class ...Args>
int ArchiveBlob(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value, Args&&... args);

template <class T>
struct WriteTrait<const T&, std::enable_if_t<std::is_enum_v<T>>> {
    static int Write(skr_binary_writer_t* writer, const T& value)
    {
        using UT = std::underlying_type_t<T>;
        return WriteTrait<const UT&>::Write(writer, static_cast<UT>(value));
    }
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const bool&> {
    static int Write(skr_binary_writer_t* writer, bool value);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const uint8_t&> {
    static int Write(skr_binary_writer_t* writer, uint8_t value);
    static int Write(skr_binary_writer_t* writer, uint8_t value, IntegerSerdeConfig<uint8_t> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const uint16_t&> {
    static int Write(skr_binary_writer_t* writer, uint16_t value);
    static int Write(skr_binary_writer_t* writer, uint16_t value, IntegerSerdeConfig<uint16_t> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const uint32_t&> {
    static int Write(skr_binary_writer_t* writer, uint32_t value);
    static int Write(skr_binary_writer_t* writer, uint32_t value, IntegerSerdeConfig<uint32_t> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const uint64_t&> {
    static int Write(skr_binary_writer_t* writer, uint64_t value);
    static int Write(skr_binary_writer_t* writer, uint64_t value, IntegerSerdeConfig<uint64_t> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const int32_t&> {
    static int Write(skr_binary_writer_t* writer, int32_t value);
    static int Write(skr_binary_writer_t* writer, int32_t value, IntegerSerdeConfig<int32_t> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const int64_t&> {
    static int Write(skr_binary_writer_t* writer, int64_t value);
    static int Write(skr_binary_writer_t* writer, int64_t value, IntegerSerdeConfig<int64_t> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const float&> {
    static int Write(skr_binary_writer_t* writer, float value);
    static int Write(skr_binary_writer_t* writer, float value, FloatingSerdeConfig<float> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const double&> {
    static int Write(skr_binary_writer_t* writer, double value);
    static int Write(skr_binary_writer_t* writer, double value, FloatingSerdeConfig<double> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float2_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float2_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float2_t& value, VectorSerdeConfig<float> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float3_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float3_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float3_t& value, VectorSerdeConfig<float> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_rotator_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_rotator_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_rotator_t& value, VectorSerdeConfig<float> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float4_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float4_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float4_t& value, VectorSerdeConfig<float> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_quaternion_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_quaternion_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_quaternion_t& value, VectorSerdeConfig<float> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_float4x4_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float4x4_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float4x4_t& value, VectorSerdeConfig<float> config);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_guid_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_guid_t& guid);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_md5_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_md5_t& MD5);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_blob_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_t& blob);
};

template <>
struct RUNTIME_STATIC_API WriteTrait<const skr_blob_arena_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_arena_t& blob);
};

template <class T, class ...Args>
int Write(skr_binary_writer_t* writer, const T& value, Args&&... args)
{
    return WriteTrait<const T&>::Write(writer, value, std::forward<Args>(args)...);
}
} // namespace skr::binary