#pragma once
#include "SkrBase/types.h"
#include "SkrSerde/blob.h"

// reader
struct SBinaryReader {
    template <class T>
    SBinaryReader(T& user)
    {
        user_data = &user;
        vread = +[](void* user, void* data, size_t size) -> bool {
            return static_cast<T*>(user)->read(data, size);
        };
        auto SupportBitPacking = SKR_VALIDATOR((auto t), t.read_bits((void*)0, (size_t)0));
        if constexpr (SupportBitPacking(SKR_TYPELIST(T)))
        {
            vread_bits = +[](void* user, void* data, size_t size) -> bool {
                return static_cast<T*>(user)->read_bits(data, size);
            };
        }
    }
    bool (*vread)(void* user_data, void* data, size_t size)      = nullptr;
    bool (*vread_bits)(void* user_data, void* data, size_t size) = nullptr;
    void* user_data                                             = nullptr;
    bool read(void* data, size_t size)
    {
        return vread(user_data, data, size);
    }
    bool read_bits(void* data, size_t size)
    {
        return vread_bits(user_data, data, size);
    }
};

// help functions
namespace skr::binary
{
inline bool ReadBytes(SBinaryReader* reader, void* data, size_t size)
{
    return reader->read(data, size);
}

template <class T, class... Args>
bool Read(SBinaryReader* reader, T&& value, Args&&... args);
template <class T, class... Args>
bool Archive(SBinaryReader* reader, T&& value, Args&&... args);
template <class T, class... Args>
bool ArchiveBlob(SBinaryReader* reader, skr_blob_arena_t& arena, T&& value, Args&&... args);
} // namespace skr::binary

// primitive types
// bool
// int/uint 8/16/32/64
// float/double
// enum
namespace skr::binary
{
template <>
struct SKR_STATIC_API ReadTrait<bool> {
    static bool Read(SBinaryReader* reader, bool& value);
};
template <>
struct SKR_STATIC_API ReadTrait<int8_t> {
    static bool Read(SBinaryReader* reader, int8_t& value);
    static bool Read(SBinaryReader* reader, int8_t& value, IntegerPackConfig<int8_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<int16_t> {
    static bool Read(SBinaryReader* reader, int16_t& value);
    static bool Read(SBinaryReader* reader, int16_t& value, IntegerPackConfig<int16_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<int32_t> {
    static bool Read(SBinaryReader* reader, int32_t& value);
    static bool Read(SBinaryReader* reader, int32_t& value, IntegerPackConfig<int32_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<int64_t> {
    static bool Read(SBinaryReader* reader, int64_t& value);
    static bool Read(SBinaryReader* reader, int64_t& value, IntegerPackConfig<int64_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<uint8_t> {
    static bool Read(SBinaryReader* reader, uint8_t& value);
    static bool Read(SBinaryReader* reader, uint8_t& value, IntegerPackConfig<uint8_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<uint16_t> {
    static bool Read(SBinaryReader* reader, uint16_t& value);
    static bool Read(SBinaryReader* reader, uint16_t& value, IntegerPackConfig<uint16_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<uint32_t> {
    static bool Read(SBinaryReader* reader, uint32_t& value);
    static bool Read(SBinaryReader* reader, uint32_t& value, IntegerPackConfig<uint32_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<uint64_t> {
    static bool Read(SBinaryReader* reader, uint64_t& value);
    static bool Read(SBinaryReader* reader, uint64_t& value, IntegerPackConfig<uint64_t>);
};
template <>
struct SKR_STATIC_API ReadTrait<float> {
    static bool Read(SBinaryReader* reader, float& value);
    static bool Read(SBinaryReader* reader, float& value, FloatingPackConfig<float>);
};
template <>
struct SKR_STATIC_API ReadTrait<double> {
    static bool Read(SBinaryReader* reader, double& value);
    static bool Read(SBinaryReader* reader, double& value, FloatingPackConfig<double>);
};
template <class T>
struct ReadTrait<T, std::enable_if_t<std::is_enum_v<T>>> {
    static bool Read(SBinaryReader* reader, T& value)
    {
        using UT = std::underlying_type_t<T>;
        return ReadTrait<UT>::Read(reader, (UT&)(value));
    }
};
} // namespace skr::binary

// skr types
namespace skr::binary
{
template <>
struct SKR_STATIC_API ReadTrait<skr_float2_t> {
    static bool Read(SBinaryReader* reader, skr_float2_t& value);
    static bool Read(SBinaryReader* reader, skr_float2_t& value, VectorPackConfig<float>);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float3_t> {
    static bool Read(SBinaryReader* reader, skr_float3_t& value);
    static bool Read(SBinaryReader* reader, skr_float3_t& value, VectorPackConfig<float>);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float4_t> {
    static bool Read(SBinaryReader* reader, skr_float4_t& value);
    static bool Read(SBinaryReader* reader, skr_float4_t& value, VectorPackConfig<float>);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_float4x4_t> {
    static bool Read(SBinaryReader* reader, skr_float4x4_t& value);
    static bool Read(SBinaryReader* reader, skr_float4x4_t& value, VectorPackConfig<float>);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_rotator_t> {
    static bool Read(SBinaryReader* reader, skr_rotator_t& value);
    static bool Read(SBinaryReader* reader, skr_rotator_t& value, VectorPackConfig<float>);
};

template <>
struct SKR_STATIC_API ReadTrait<skr_quaternion_t> {
    static bool Read(SBinaryReader* reader, skr_quaternion_t& value);
    static bool Read(SBinaryReader* reader, skr_quaternion_t& value, VectorPackConfig<float>);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_guid_t> {
    static bool Read(SBinaryReader* reader, skr_guid_t& guid);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_md5_t> {
    static bool Read(SBinaryReader* reader, skr_md5_t& md5);
};
template <>
struct SKR_STATIC_API ReadTrait<skr::IBlob*> {
    static bool Read(SBinaryReader* reader, skr::IBlob*& blob);
};
template <>
struct SKR_STATIC_API ReadTrait<skr::BlobId> {
    static bool Read(SBinaryReader* reader, BlobId& blob);
};
template <>
struct SKR_STATIC_API ReadTrait<skr_blob_arena_t> {
    static bool Read(SBinaryReader* reader, skr_blob_arena_t& blob);
};
} // namespace skr::binary

// help function impl
namespace skr::binary
{
template <class T, class... Args>
bool Read(SBinaryReader* reader, T&& value, Args&&... args)
{
    return ReadTrait<std::decay_t<T>>::Read(reader, value, std::forward<Args>(args)...);
}
} // namespace skr::binary