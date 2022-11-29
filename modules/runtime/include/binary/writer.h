#pragma once
#include <EASTL/vector.h>
#include "containers/span.hpp"
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
int Archive(skr_binary_writer_t* writer, const T& value);
template <class T>
int Archive(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value);

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
    static int Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const skr::string_view& str);
};

template <>
struct RUNTIME_API WriteHelper<const skr_guid_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_guid_t& guid);
};

template <>
struct RUNTIME_API WriteHelper<const skr_md5_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_md5_t& MD5);
};

template <>
struct RUNTIME_API WriteHelper<const skr_resource_handle_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_resource_handle_t& handle);
};

template <>
struct RUNTIME_API WriteHelper<const skr_blob_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_t& blob);
};

template <>
struct RUNTIME_API WriteHelper<const skr_blob_arena_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_blob_arena_t& blob);
};

template <class T>
struct WriteHelper<const TEnumAsByte<T>&> {
    static int Write(skr_binary_writer_t* writer, const TEnumAsByte<T>& value)
    {
        return skr::binary::Write(writer, value.as_byte());
    }
};

template <class T>
struct WriteHelper<const skr::span<T>&> {
    static int Write(skr_binary_writer_t* writer, const skr::span<T>& span)
    {
        for (const T& value : span) {
            if(auto result = skr::binary::Write(writer, value); result != 0) {
                return result;
            }
        }
        return 0;
    }
    static int Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const skr::span<T>& span)
    {
        auto ptr = (char*)span.data();
        auto buffer = (char*)arena.get_buffer();
        SKR_ASSERT(ptr > buffer);
        uint32_t offset = (uint32_t)(ptr - buffer);
        SKR_ASSERT(offset < arena.get_size());
        int ret = skr::binary::Write(writer, offset);
        if (ret != 0) {
            return ret;
        }
        ret = skr::binary::Write(writer, (uint32_t)span.size());
        if (ret != 0) {
            return ret;
        }
        // inner data is contained in blob arena, no need to write
        // if constexpr(is_complete_v<BlobHelper<T>>)
        // {
        //     for(int i = 0; i < span.size(); ++i)
        //     {
        //         ret = WriteHelper<const T&>::Write(writer, arena, span[i]);
        //         if (ret != 0) {
        //             return ret;
        //         }
        //     }
        // }
        return 0;
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
    static int Write(skr_binary_writer_t* archive, const eastl::vector<V, Allocator>& vec)
    {
        SKR_ARCHIVE((uint32_t)vec.size());
        for (auto& value : vec)
        {
            SKR_ARCHIVE(value);
        }
        return 0;
    }
};

template <class... Ts>
struct WriteHelper<const skr::variant<Ts...>&> {
    static int Write(skr_binary_writer_t* archive, const skr::variant<Ts...>& variant)
    {
        SKR_ARCHIVE((uint32_t)variant.index());
        int ret;
        std::visit([&](auto&& value) {
            ret = skr::binary::Write(archive, value);
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

template <class T>
int Archive(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const T& value)
{
    return WriteHelper<const T&>::Write(writer, arena, value);
}

struct VectorWriter
{
    eastl::vector<uint8_t>* buffer;
    int write(const void* data, size_t size)
    {
        buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
        return 0;
    }
};
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