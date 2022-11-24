#pragma once
#include <EASTL/vector.h>
#include "reader_fwd.h"
#include "resource/resource_handle.h"
#include "containers/variant.hpp"
#include "containers/string.hpp"
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
int Read(skr_binary_reader_t* reader, T& value);

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
struct RUNTIME_API ReadHelper<skr::string> {
    static int Read(skr_binary_reader_t* reader, skr::string& str);
};

template <>
struct RUNTIME_API ReadHelper<skr_guid_t> {
    static int Read(skr_binary_reader_t* reader, skr_guid_t& guid);
};

template <>
struct RUNTIME_API ReadHelper<skr_resource_handle_t> {
    static int Read(skr_binary_reader_t* reader, skr_resource_handle_t& handle);
};

template <>
struct RUNTIME_API ReadHelper<skr_blob_t> {
    static int Read(skr_binary_reader_t* reader, skr_blob_t& blob);
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
struct ReadHelper<TEnumAsByte<T>>
{
    static int Read(skr_binary_reader_t* reader, TEnumAsByte<T>& value)
    {
        return skr::binary::Read(reader, value.as_byte());
    }
};

template <class T>
struct ReadHelper<skr::resource::TResourceHandle<T>> {
    static int Read(skr_binary_reader_t* reader, skr::resource::TResourceHandle<T>& handle)
    {
        skr_guid_t guid;
        int ret = skr::binary::Read(reader, guid);
        if (ret != 0)
            return ret;
        handle.set_guid(guid);
        return ret;
    }
};

template <class V, class Allocator>
struct ReadHelper<eastl::vector<V, Allocator>> {
    static int Read(skr_binary_reader_t* json, eastl::vector<V, Allocator>& vec)
    {
        eastl::vector<V, Allocator> temp;
        uint32_t size;
        int ret = skr::binary::Read(json, size);
        if (ret != 0)
            return ret;

        temp.reserve(size);
        for (uint32_t i = 0; i < size; ++i)
        {
            V value;
            ret = skr::binary::Read(json, value);
            if (ret != 0)
                return ret;
            temp.push_back(std::move(value));
        }
        vec = std::move(temp);
        return 0;
    }
};

template<class ...Ts>
struct ReadHelper<skr::variant<Ts...>>
{
    template<size_t I, class T>
    static int ReadByIndex(skr_binary_reader_t* reader, skr::variant<Ts...>& value, size_t index)
    {
        if (index == I)
        {
            T t;
            int ret = skr::binary::Read(reader, t);
            if (ret != 0)
                return ret;
            value = std::move(t);
            return 0;
        }
        return -1;
    }

    template<size_t ...Is>
    static int ReadByIndexHelper(skr_binary_reader_t* reader, skr::variant<Ts...>& value, size_t index, std::index_sequence<Is...>)
    {
        int result;
        (void)(((result = ReadByIndex<Is, Ts>(reader, value, index)) != 0) && ...);
        return result;
    }

    static int Read(skr_binary_reader_t* reader, skr::variant<Ts...>& value)
    {
        uint32_t index;
        int ret = skr::binary::Read(reader, index);
        if (ret != 0)
            return ret;
        if (index >= sizeof...(Ts))
            return -1;
        return ReadByIndexHelper(reader, value, index, std::make_index_sequence<sizeof...(Ts)>());
    }
};

template <class T>
int Read(skr_binary_reader_t* reader, T& value)
{
    return ReadHelper<std::decay_t<T>>::Read(reader, value);
}

template <class T>
int Archive(skr_binary_reader_t* writer, T& value)
{
    return ReadHelper<std::decay_t<T>>::Read(writer, value);
}
} // namespace binary

template <class V, class Allocator>
struct SerdeCompleteChecker<binary::ReadHelper<eastl::vector<V, Allocator>>>
    : std::bool_constant<is_complete_serde_v<json::ReadHelper<V>>> {
};

template <class ...Ts>
struct SerdeCompleteChecker<binary::ReadHelper<skr::variant<Ts...>>>
    : std::bool_constant<(is_complete_serde_v<json::ReadHelper<Ts>> && ...)> {
};

} // namespace skr