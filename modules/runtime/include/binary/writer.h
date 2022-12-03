#pragma once
#include "writer_fwd.h"
#include "utils/types.h"
#include "type/type_helper.hpp"

struct skr_binary_writer_t {
    template <class T>
    skr_binary_writer_t(T& user)
    {
        user_data = &user;
        vwrite = [](void* user, const void* data, size_t size) -> int {
            return static_cast<T*>(user)->write(data, size);
        };
    }
    int (*vwrite)(void* user_data, const void* data, size_t size);
    void* user_data;
    int write(const void* data, size_t size)
    {
        return vwrite(user_data, data, size);
    }
};

namespace skr::binary
{
inline int WriteValue(skr_binary_writer_t* writer, const void* data, size_t size)
{
    return writer->write(data, size);
}

template <class T>
int Write(skr_binary_writer_t* writer, const T& value);
template <class T>
int Archive(skr_binary_writer_t* writer, const T& value);
template <class T>
int Archive(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value);

template <class T>
struct WriteTrait<const T&, std::enable_if_t<std::is_enum_v<T>>> {
    static int Write(skr_binary_writer_t* writer, const T& value)
    {
        using UT = std::underlying_type_t<T>;
        return WriteTrait<const UT&>::Write(writer, static_cast<UT>(value));
    }
};

template <>
struct RUNTIME_API WriteTrait<const bool&> {
    static int Write(skr_binary_writer_t* writer, bool value);
};

template <>
struct RUNTIME_API WriteTrait<const uint32_t&> {
    static int Write(skr_binary_writer_t* writer, uint32_t value);
};

template <>
struct RUNTIME_API WriteTrait<const uint64_t&> {
    static int Write(skr_binary_writer_t* writer, uint64_t value);
};

template <>
struct RUNTIME_API WriteTrait<const int32_t&> {
    static int Write(skr_binary_writer_t* writer, int32_t value);
};

template <>
struct RUNTIME_API WriteTrait<const int64_t&> {
    static int Write(skr_binary_writer_t* writer, int64_t value);
};

template <>
struct RUNTIME_API WriteTrait<const float&> {
    static int Write(skr_binary_writer_t* writer, float value);
};

template <>
struct RUNTIME_API WriteTrait<const double&> {
    static int Write(skr_binary_writer_t* writer, double value);
};

template <>
struct RUNTIME_API WriteTrait<const skr_float2_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float2_t& value);
};

template <>
struct RUNTIME_API WriteTrait<const skr_float3_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float3_t& value);
};

template <>
struct RUNTIME_API WriteTrait<const skr_rotator_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_rotator_t& value);
};

template <>
struct RUNTIME_API WriteTrait<const skr_float4_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float4_t& value);
};

template <>
struct RUNTIME_API WriteTrait<const skr_quaternion_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_quaternion_t& value);
};

template <>
struct RUNTIME_API WriteTrait<const skr_float4x4_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float4x4_t& value);
};

template <>
struct RUNTIME_API WriteTrait<const skr_guid_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_guid_t& guid);
};

template <>
struct RUNTIME_API WriteTrait<const skr_md5_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_md5_t& MD5);
};

template <>
struct RUNTIME_API WriteTrait<const skr_blob_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_t& blob);
};

template <>
struct RUNTIME_API WriteTrait<const skr_blob_arena_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_arena_t& blob);
};

template <class T>
int Write(skr_binary_writer_t* writer, const T& value)
{
    return WriteTrait<const T&>::Write(writer, value);
}
} // namespace skr::binary