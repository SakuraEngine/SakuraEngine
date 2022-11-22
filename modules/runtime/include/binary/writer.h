#pragma once
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include "platform/configure.h"
#include "resource/resource_handle.h"
#include "containers/variant.hpp"

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
template <class T>
using TParamType = std::conditional_t<std::is_fundamental_v<T> || std::is_enum_v<T>, T, const T&>;

template <class T>
std::enable_if_t<!std::is_enum_v<T>, int> WriteValue(skr_binary_writer_t* writer, T value)
{
    static_assert(!sizeof(T), "WriteValue not implemented for this type");
    return -1;
}

inline int WriteValue(skr_binary_writer_t* writer, const void* data, size_t size)
{
    return writer->write(data, size);
}
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, bool value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, uint32_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, uint64_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, int32_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, int64_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, float value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, double value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, skr_float2_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, skr_float3_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, skr_rotator_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, skr_float4_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, skr_quaternion_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, skr_float4x4_t value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_float2_t& value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_float3_t& value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_rotator_t& value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_float4_t& value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_quaternion_t& value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_float4x4_t& value);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const eastl::string& str);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const eastl::string_view& str);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_guid_t& guid);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_resource_handle_t& handle);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, skr_resource_handle_t handle);
template <>
RUNTIME_API int WriteValue(skr_binary_writer_t* writer, const skr_blob_t& blob);
template<class T>
std::enable_if_t<std::is_enum_v<T>, int> WriteValue(skr_binary_writer_t* writer, T value)
{
    return WriteValue(writer, static_cast<std::underlying_type_t<T>>(value));
}

template <class T>
int Write(skr_binary_writer_t* writer, const T& value);

template <class T>
struct WriteHelper {
    static int Write(skr_binary_writer_t* binary, T map)
    {
        using TType = std::remove_const_t<std::remove_reference_t<T>>;
        return WriteValue<TParamType<TType>>(binary, map);
    }
};

template <class T>
struct WriteHelper<const skr::resource::TResourceHandle<T>&> {
    static int Write(skr_binary_writer_t* binary, const skr::resource::TResourceHandle<T>& handle)
    {
        const auto& hdl = static_cast<const skr_resource_handle_t&>(handle);
        return WriteValue(binary, hdl);
    }
};

template <class V, class Allocator>
struct WriteHelper<const eastl::vector<V, Allocator>&> {
    static int Write(skr_binary_writer_t* binary, const eastl::vector<V, Allocator>& vec)
    {
        int ret = WriteValue(binary, (uint32_t)vec.size());
        if (ret != 0)
            return ret;
        for (auto& value : vec)
        {
            ret = skr::binary::Write<TParamType<V>>(binary, value);
            if (ret != 0)
                return ret;
        }
        return ret;
    }
};

template<class ...Ts>
struct WriteHelper<const skr::variant<Ts...>&>
{
    static int Write(skr_binary_writer_t* binary, const skr::variant<Ts...>& variant)
    {
        int ret = WriteValue(binary, (uint32_t)variant.index());
        if (ret != 0)
            return ret;
        std::visit([&](auto&& value) {
            ret = skr::binary::Write<decltype(value)>(binary, value);
        }, variant);
        return ret;
    }
};

template <class T>
int Write(skr_binary_writer_t* writer, const T& value)
{
    return WriteHelper<const T&>::Write(writer, value);
}

template <class T>
int Archive(skr_binary_writer_t* writer, const T& value)
{
    return WriteHelper<const T&>::Write(writer, value);
}
} // namespace skr::binary