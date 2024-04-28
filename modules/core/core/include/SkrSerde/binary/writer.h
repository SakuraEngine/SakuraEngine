#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/types.h"
#include "SkrSerde/blob.h"

// writer
struct skr_binary_writer_t {
    template <class T>
    skr_binary_writer_t(T& user)
    {
        user_data = &user;
        vwrite    = [](void* user, const void* data, size_t size) -> int {
            return static_cast<T*>(user)->write(data, size);
        };
        auto SupportBitPacking = SKR_VALIDATOR((auto t), t.write_bits((void*)0, (size_t)0));
        if constexpr (SupportBitPacking(SKR_TYPELIST(T)))
        {
            vwrite_bits = [](void* user, const void* data, size_t size) -> int {
                return static_cast<T*>(user)->write_bits(data, size);
            };
        }
    }
    int (*vwrite)(void* user_data, const void* data, size_t size)      = nullptr;
    int (*vwrite_bits)(void* user_data, const void* data, size_t size) = nullptr;
    void* user_data                                                    = nullptr;
    int   write(const void* data, size_t size)
    {
        return vwrite(user_data, data, size);
    }
    int write_bits(const void* data, size_t size)
    {
        return vwrite_bits(user_data, data, size);
    }
};

// helper functions
namespace skr::binary
{

inline int WriteBytes(skr_binary_writer_t* writer, const void* data, size_t size)
{
    return writer->write(data, size);
}

template <class T, class... Args>
int Write(skr_binary_writer_t* writer, const T& value, Args&&... args);
template <class T, class... Args>
int Archive(skr_binary_writer_t* writer, const T& value, Args&&... args);
template <class T, class... Args>
int ArchiveBlob(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value, Args&&... args);

}; // namespace skr::binary

// primitive types
// bool
// int/uint 8/16/32/64
// float/double
// enum
namespace skr::binary
{
// bool
template <>
struct SKR_STATIC_API WriteTrait<bool> {
    static int Write(skr_binary_writer_t* writer, bool value);
};

// int
template <>
struct SKR_STATIC_API WriteTrait<int8_t> {
    static int Write(skr_binary_writer_t* writer, int8_t value);
    static int Write(skr_binary_writer_t* writer, int8_t value, IntegerPackConfig<int8_t> config);
};
template <>
struct SKR_STATIC_API WriteTrait<int16_t> {
    static int Write(skr_binary_writer_t* writer, int16_t value);
    static int Write(skr_binary_writer_t* writer, int16_t value, IntegerPackConfig<int16_t> config);
};
template <>
struct SKR_STATIC_API WriteTrait<int32_t> {
    static int Write(skr_binary_writer_t* writer, int32_t value);
    static int Write(skr_binary_writer_t* writer, int32_t value, IntegerPackConfig<int32_t> config);
};
template <>
struct SKR_STATIC_API WriteTrait<int64_t> {
    static int Write(skr_binary_writer_t* writer, int64_t value);
    static int Write(skr_binary_writer_t* writer, int64_t value, IntegerPackConfig<int64_t> config);
};

// uint
template <>
struct SKR_STATIC_API WriteTrait<uint8_t> {
    static int Write(skr_binary_writer_t* writer, uint8_t value);
    static int Write(skr_binary_writer_t* writer, uint8_t value, IntegerPackConfig<uint8_t> config);
};
template <>
struct SKR_STATIC_API WriteTrait<uint16_t> {
    static int Write(skr_binary_writer_t* writer, uint16_t value);
    static int Write(skr_binary_writer_t* writer, uint16_t value, IntegerPackConfig<uint16_t> config);
};
template <>
struct SKR_STATIC_API WriteTrait<uint32_t> {
    static int Write(skr_binary_writer_t* writer, uint32_t value);
    static int Write(skr_binary_writer_t* writer, uint32_t value, IntegerPackConfig<uint32_t> config);
};
template <>
struct SKR_STATIC_API WriteTrait<uint64_t> {
    static int Write(skr_binary_writer_t* writer, uint64_t value);
    static int Write(skr_binary_writer_t* writer, uint64_t value, IntegerPackConfig<uint64_t> config);
};

// float
template <>
struct SKR_STATIC_API WriteTrait<float> {
    static int Write(skr_binary_writer_t* writer, float value);
    static int Write(skr_binary_writer_t* writer, float value, FloatingPackConfig<float> config);
};
template <>
struct SKR_STATIC_API WriteTrait<double> {
    static int Write(skr_binary_writer_t* writer, double value);
    static int Write(skr_binary_writer_t* writer, double value, FloatingPackConfig<double> config);
};

// enum
template <class T>
struct WriteTrait<T, std::enable_if_t<std::is_enum_v<T>>> {
    inline static int Write(skr_binary_writer_t* writer, const T& value)
    {
        using UT = std::underlying_type_t<T>;
        return WriteTrait<UT>::Write(writer, static_cast<UT>(value));
    }
};
} // namespace skr::binary

// skr types
namespace skr::binary
{
template <>
struct SKR_STATIC_API WriteTrait<skr_float2_t> {
    static int Write(skr_binary_writer_t* writer, const skr_float2_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float2_t& value, VectorPackConfig<float> config);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float3_t> {
    static int Write(skr_binary_writer_t* writer, const skr_float3_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float3_t& value, VectorPackConfig<float> config);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4_t> {
    static int Write(skr_binary_writer_t* writer, const skr_float4_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float4_t& value, VectorPackConfig<float> config);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_float4x4_t> {
    static int Write(skr_binary_writer_t* writer, const skr_float4x4_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_float4x4_t& value, VectorPackConfig<float> config);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_rotator_t> {
    static int Write(skr_binary_writer_t* writer, const skr_rotator_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_rotator_t& value, VectorPackConfig<float> config);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_quaternion_t> {
    static int Write(skr_binary_writer_t* writer, const skr_quaternion_t& value);
    static int Write(skr_binary_writer_t* writer, const skr_quaternion_t& value, VectorPackConfig<float> config);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_guid_t> {
    static int Write(skr_binary_writer_t* writer, const skr_guid_t& guid);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_md5_t> {
    static int Write(skr_binary_writer_t* writer, const skr_md5_t& MD5);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::IBlob*> {
    static int Write(skr_binary_writer_t* writer, const skr::IBlob*& blob);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::BlobId> {
    static int Write(skr_binary_writer_t* writer, const skr::BlobId& blob);
};
template <>
struct SKR_STATIC_API WriteTrait<skr_blob_arena_t> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_arena_t& blob);
};
} // namespace skr::binary

// helper function impl
namespace skr::binary
{
template <class T, class... Args>
int Write(skr_binary_writer_t* writer, const T& value, Args&&... args)
{
    return WriteTrait<T>::Write(writer, value, std::forward<Args>(args)...);
}
} // namespace skr::binary