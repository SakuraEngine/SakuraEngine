#pragma once
#include "reader_fwd.h"
#include "utils/types.h"
#include "type/type_helper.hpp"

struct skr_binary_reader_t {
    template <class T>
    skr_binary_reader_t(T& user)
    {
        user_data = &user;
        vread = [](void* user, void* data, size_t size) {
            return static_cast<T*>(user)->read(data, size);
        };
    }
    int (*vread)(void* user_data, void* data, size_t size);
    void* user_data;
    int read(void* data, size_t size)
    {
        return vread(user_data, data, size);
    }
};
namespace skr
{
namespace binary
{
inline int ReadValue(skr_binary_reader_t* reader, void* data, size_t size)
{
    return reader->read(data, size);
}

template <class T>
int Read(skr_binary_reader_t* reader, T&& value);
template <class T>
int Archive(skr_binary_reader_t* reader, T&& value);
template <class T>
int Archive(skr_binary_reader_t* reader, skr_blob_arena_t& arena, T&& value);

template <>
struct RUNTIME_API ReadHelper<bool> {
    static int Read(skr_binary_reader_t* reader, bool& value);
};

template <>
struct RUNTIME_API ReadHelper<uint32_t> {
    static int Read(skr_binary_reader_t* reader, uint32_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<uint64_t> {
    static int Read(skr_binary_reader_t* reader, uint64_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<int32_t> {
    static int Read(skr_binary_reader_t* reader, int32_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<int64_t> {
    static int Read(skr_binary_reader_t* reader, int64_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<float> {
    static int Read(skr_binary_reader_t* reader, float& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<double> {
    static int Read(skr_binary_reader_t* reader, double& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<skr_float2_t> {
    static int Read(skr_binary_reader_t* reader, skr_float2_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<skr_float3_t> {
    static int Read(skr_binary_reader_t* reader, skr_float3_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<skr_rotator_t> {
    static int Read(skr_binary_reader_t* reader, skr_rotator_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<skr_float4_t> {
    static int Read(skr_binary_reader_t* reader, skr_float4_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<skr_quaternion_t> {
    static int Read(skr_binary_reader_t* reader, skr_quaternion_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<skr_float4x4_t> {
    static int Read(skr_binary_reader_t* reader, skr_float4x4_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
};

template <>
struct RUNTIME_API ReadHelper<skr_guid_t> {
    static int Read(skr_binary_reader_t* reader, skr_guid_t& guid);
};

template <>
struct RUNTIME_API ReadHelper<skr_md5_t> {
    static int Read(skr_binary_reader_t* reader, skr_md5_t& md5);
};

template <>
struct RUNTIME_API ReadHelper<skr_blob_t> {
    static int Read(skr_binary_reader_t* reader, skr_blob_t& blob);
};

template <>
struct RUNTIME_API ReadHelper<skr_blob_arena_t> {
    static int Read(skr_binary_reader_t* reader, skr_blob_arena_t& blob);
};

template <class T>
struct ReadHelper<T, std::enable_if_t<std::is_enum_v<T>>> {
    static int Read(skr_binary_reader_t* reader, T& value)
    {
        using UT = std::underlying_type_t<T>;
        return ReadHelper<UT>::Read(reader, (UT&)(value));
    }
};

template <class T>
int Read(skr_binary_reader_t* reader, T&& value)
{
    return ReadHelper<std::decay_t<T>>::Read(reader, value);
}
} // namespace binary
} // namespace skr