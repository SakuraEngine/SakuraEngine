#pragma once
#include "serde/binary/reader_fwd.h"
#include "misc/types.h"
#include "type/type_helper.hpp"
#include "serde/binary/serde.h"

struct skr_binary_reader_t {
    template <class T>
    skr_binary_reader_t(T& user)
    {
        user_data = &user;
        vread = [](void* user, void* data, size_t size) {
            const auto err = static_cast<T*>(user)->read(data, size);
            return err;
        };
        auto SupportBitPacking = SKR_VALIDATOR((auto t), t.read_bits((void*)0, (size_t)0));
        if constexpr(SupportBitPacking(SKR_TYPELIST(T)))
        {
            vread_bits = [](void* user, void* data, size_t size) {
                const auto err = static_cast<T*>(user)->read_bits(data, size);
                return err;
            };
        }
    }
    int (*vread)(void* user_data, void* data, size_t size) = nullptr;
    int (*vread_bits)(void* user_data, void* data, size_t size) = nullptr;
    void* user_data = nullptr;
    int read(void* data, size_t size)
    {
        const auto err = vread(user_data, data, size);
        return err;
    }
    int read_bits(void* data, size_t size)
    {
        const auto err = vread_bits(user_data, data, size);
        return err;
    }
};
namespace skr
{
namespace binary
{
inline int ReadBytes(skr_binary_reader_t* reader, void* data, size_t size)
{
    const auto err =  reader->read(data, size);
    return err;
}

template <class T, class... Args>
int Read(skr_binary_reader_t* reader, T&& value, Args&&... args);
template <class T, class... Args>
int Archive(skr_binary_reader_t* reader, T&& value, Args&&... args);
template <class T, class... Args>
int ArchiveBlob(skr_binary_reader_t* reader, skr_blob_arena_t& arena, T&& value, Args&&... args);

template <>
struct RUNTIME_STATIC_API ReadTrait<bool> {
    static int Read(skr_binary_reader_t* reader, bool& value);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<uint8_t> {
    static int Read(skr_binary_reader_t* reader, uint8_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, uint8_t& value, IntegerPackConfig<uint8_t>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<uint16_t> {
    static int Read(skr_binary_reader_t* reader, uint16_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, uint16_t& value, IntegerPackConfig<uint16_t>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<uint32_t> {
    static int Read(skr_binary_reader_t* reader, uint32_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, uint32_t& value, IntegerPackConfig<uint32_t>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<uint64_t> {
    static int Read(skr_binary_reader_t* reader, uint64_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, uint64_t& value, IntegerPackConfig<uint64_t>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<int32_t> {
    static int Read(skr_binary_reader_t* reader, int32_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, int32_t& value, IntegerPackConfig<int32_t>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<int64_t> {
    static int Read(skr_binary_reader_t* reader, int64_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, int64_t& value, IntegerPackConfig<int64_t>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<float> {
    static int Read(skr_binary_reader_t* reader, float& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, float& value, FloatingPackConfig<float>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<double> {
    static int Read(skr_binary_reader_t* reader, double& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, double& value, FloatingPackConfig<double>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_float2_t> {
    static int Read(skr_binary_reader_t* reader, skr_float2_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, skr_float2_t& value, VectorPackConfig<float>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_float3_t> {
    static int Read(skr_binary_reader_t* reader, skr_float3_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, skr_float3_t& value, VectorPackConfig<float>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_rotator_t> {
    static int Read(skr_binary_reader_t* reader, skr_rotator_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, skr_rotator_t& value, VectorPackConfig<float>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_float4_t> {
    static int Read(skr_binary_reader_t* reader, skr_float4_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, skr_float4_t& value, VectorPackConfig<float>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_quaternion_t> {
    static int Read(skr_binary_reader_t* reader, skr_quaternion_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, skr_quaternion_t& value, VectorPackConfig<float>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_float4x4_t> {
    static int Read(skr_binary_reader_t* reader, skr_float4x4_t& value)
    {
        return reader->read(&value, sizeof(value));
    }
    static int Read(skr_binary_reader_t* reader, skr_float4x4_t& value, VectorPackConfig<float>);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_guid_t> {
    static int Read(skr_binary_reader_t* reader, skr_guid_t& guid);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_md5_t> {
    static int Read(skr_binary_reader_t* reader, skr_md5_t& md5);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_blob_t> {
    static int Read(skr_binary_reader_t* reader, skr_blob_t& blob);
};

template <>
struct RUNTIME_STATIC_API ReadTrait<skr_blob_arena_t> {
    static int Read(skr_binary_reader_t* reader, skr_blob_arena_t& blob);
};

template <class T>
struct ReadTrait<T, std::enable_if_t<std::is_enum_v<T>>> {
    static int Read(skr_binary_reader_t* reader, T& value)
    {
        using UT = std::underlying_type_t<T>;
        return ReadTrait<UT>::Read(reader, (UT&)(value));
    }
};

template <class T, class... Args>
int Read(skr_binary_reader_t* reader, T&& value, Args&&... args)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, value, std::forward<Args>(args)...);
}
} // namespace binary
} // namespace skr