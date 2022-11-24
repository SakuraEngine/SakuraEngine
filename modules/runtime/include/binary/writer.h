#pragma once
#include <EASTL/vector.h>
#include "writer_fwd.h"
#include "resource/resource_handle.h"
#include "containers/variant.hpp"
#include "containers/string.hpp"
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
struct WriteHelper<const T&, std::enable_if_t<std::is_enum_v<T>>> {
    static int Write(skr_binary_writer_t* writer, const T& value)
    {
        using UT = std::underlying_type_t<T>;
        return WriteHelper<const UT&>::Write(writer, static_cast<UT>(value));
    }
};

template <>
struct RUNTIME_API WriteHelper<const bool&> {
    static int Write(skr_binary_writer_t* writer, bool value);
};

template <>
struct RUNTIME_API WriteHelper<const uint32_t&> {
    static int Write(skr_binary_writer_t* writer, uint32_t value);
};

template <>
struct RUNTIME_API WriteHelper<const uint64_t&> {
    static int Write(skr_binary_writer_t* writer, uint64_t value);
};

template <>
struct RUNTIME_API WriteHelper<const int32_t&> {
    static int Write(skr_binary_writer_t* writer, int32_t value);
};

template <>
struct RUNTIME_API WriteHelper<const int64_t&> {
    static int Write(skr_binary_writer_t* writer, int64_t value);
};

template <>
struct RUNTIME_API WriteHelper<const float&> {
    static int Write(skr_binary_writer_t* writer, float value);
};

template <>
struct RUNTIME_API WriteHelper<const double&> {
    static int Write(skr_binary_writer_t* writer, double value);
};

template <>
struct RUNTIME_API WriteHelper<const skr_float2_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float2_t& value);
};

template <>
struct RUNTIME_API WriteHelper<const skr_float3_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float3_t& value);
};

template <>
struct RUNTIME_API WriteHelper<const skr_rotator_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_rotator_t& value);
};

template <>
struct RUNTIME_API WriteHelper<const skr_float4_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float4_t& value);
};

template <>
struct RUNTIME_API WriteHelper<const skr_quaternion_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_quaternion_t& value);
};

template <>
struct RUNTIME_API WriteHelper<const skr_float4x4_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_float4x4_t& value);
};

template <>
struct RUNTIME_API WriteHelper<const skr::string&> {
    static int Write(skr_binary_writer_t* writer, const skr::string& str);
};

template <>
struct RUNTIME_API WriteHelper<const skr::string_view&> {
    static int Write(skr_binary_writer_t* writer, const skr::string_view& str);
};

template <>
struct RUNTIME_API WriteHelper<const skr_guid_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_guid_t& guid);
};

template <>
struct RUNTIME_API WriteHelper<const skr_resource_handle_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_resource_handle_t& handle);
};

template <>
struct RUNTIME_API WriteHelper<const skr_blob_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_t& blob);
};

template <class T>
struct WriteHelper<const TEnumAsByte<T>&> {
    static int Write(skr_binary_writer_t* writer, const TEnumAsByte<T>& value)
    {
        return skr::binary::Write(writer, value.as_byte());
    }
};

template <class T>
struct WriteHelper<const skr::resource::TResourceHandle<T>&> {
    static int Write(skr_binary_writer_t* binary, const skr::resource::TResourceHandle<T>& handle)
    {
        const auto& hdl = static_cast<const skr_resource_handle_t&>(handle);
        return skr::binary::Write(binary, hdl);
    }
};

template <class V, class Allocator>
struct WriteHelper<const eastl::vector<V, Allocator>&> {
    static int Write(skr_binary_writer_t* binary, const eastl::vector<V, Allocator>& vec)
    {
        int ret = skr::binary::Write(binary, (uint32_t)vec.size());
        if (ret != 0)
            return ret;
        for (auto& value : vec)
        {
            ret = skr::binary::Write(binary, value);
            if (ret != 0)
                return ret;
        }
        return ret;
    }
};

template <class... Ts>
struct WriteHelper<const skr::variant<Ts...>&> {
    static int Write(skr_binary_writer_t* binary, const skr::variant<Ts...>& variant)
    {
        int ret = skr::binary::Write(binary, (uint32_t)variant.index());
        if (ret != 0)
            return ret;
        std::visit([&](auto&& value) {
            ret = skr::binary::Write(binary, value);
        },
        variant);
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

namespace skr
{
template <class V, class Allocator>
struct SerdeCompleteChecker<binary::WriteHelper<const eastl::vector<V, Allocator>&>>
    : std::bool_constant<is_complete_serde_v<json::WriteHelper<V>>> {
};

template <class... Ts>
struct SerdeCompleteChecker<binary::WriteHelper<const skr::variant<Ts...>&>>
    : std::bool_constant<(is_complete_serde_v<json::WriteHelper<Ts>> && ...)> {
};

} // namespace skr