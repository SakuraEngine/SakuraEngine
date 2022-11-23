#pragma once
#include "json/reader_fwd.h"

#if defined(__cplusplus)
#include <EASTL/vector.h>
#include "simdjson.h"
#include "containers/hashmap.hpp"
#include "containers/variant.hpp"
#include "containers/string.hpp"
#include "type/type_id.hpp"
#include "platform/guid.hpp"

// forward declaration for resources
struct skr_resource_handle_t;
namespace skr::resource { template <class T> struct TResourceHandle; }
// end forward declaration for resources

struct RUNTIME_API skr_json_reader_t {
    simdjson::ondemand::value* json;
};
// utils for codegen
namespace skr
{
namespace json
{
struct error_code_info {
    error_code code;
    const char* message; // do not use a fancy std::string where a simple C string will do (no alloc, no destructor)
};
RUNTIME_API const char* error_message(error_code err) noexcept;
RUNTIME_API void set_error_message(error_code err) noexcept;

template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, bool& b);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, int32_t& b);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, uint32_t& b);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, int64_t& b);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, uint64_t& b);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, float& f);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, double& b);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, skr::string& guid);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, struct skr_guid_t& guid);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, skr_resource_handle_t& handle);


template <class T>
error_code Read(simdjson::ondemand::value&& json, T& value);

template <class T>
struct ReadHelper {
    static error_code Read(simdjson::ondemand::value&& json, T& map)
    {
        return ReadValue<T>(std::move(json), map);
    }
};

template <class K, class V, class Hash, class Eq>
struct ReadHelper<skr::flat_hash_map<K, V, Hash, Eq>> {
    static error_code Read(simdjson::ondemand::value&& json, skr::flat_hash_map<K, V, Hash, Eq>& map)
    {
        auto object = json.get_object();
        if (object.error() != simdjson::SUCCESS)
            return (error_code)object.error();
        for (auto pair : object.value_unsafe())
        {
            auto key = pair.key();
            if (key.error() != simdjson::SUCCESS)
                return (error_code)key.error();
            auto value = pair.value();
            if (value.error() != simdjson::SUCCESS)
                return (error_code)value.error();
            V v;
            error_code error = skr::json::Read<V>(std::move(value).value_unsafe(), v);
            if(error != SUCCESS)
                return error;
            map.insert(std::make_pair(key.value_unsafe().raw(), std::move(v)));
        }
        return SUCCESS;
    }
};

template<class V, class Allocator>
struct ReadHelper<eastl::vector<V, Allocator>> {
    static error_code Read(simdjson::ondemand::value&& json, eastl::vector<V, Allocator>& vec)
    {
        auto array = json.get_array();
        if (array.error() != simdjson::SUCCESS)
            return (error_code)array.error();
        vec.reserve(array.value_unsafe().count_elements().value_unsafe());
        for (auto value : array.value_unsafe())
        {
            if (value.error() != simdjson::SUCCESS)
                return (error_code)value.error();
            V v;
            error_code error = skr::json::Read<V>(std::move(value).value_unsafe(), v);
            if (error != SUCCESS)
                return error;
            vec.push_back(std::move(v));
        }
        return SUCCESS;
    }
};

template<class ...Ts>
struct ReadHelper<skr::variant<Ts...>>
{
    template<class T>
    static error_code ReadByIndex(simdjson::ondemand::value&& json, skr::variant<Ts...>& value, skr_guid_t index)
    {
        if (index == skr::type::type_id<T>::get())
        {
            T t;
            error_code ret = skr::json::Read(std::move(json), t);
            if (ret != error_code::SUCCESS)
                return ret;
            value = std::move(t);
            return error_code::SUCCESS;
        }
        return error_code::VARIANT_ERROR;
    }

    static error_code Read(simdjson::ondemand::value&& json, skr::variant<Ts...>& value)
    {
        auto object = json.get_object();
        if (object.error() != simdjson::SUCCESS)
            return (error_code)object.error();
        auto type = object.value_unsafe()["type"];
        if (type.error() != simdjson::SUCCESS)
            return (error_code)type.error();
        skr_guid_t index;
        error_code ret = skr::json::Read<skr_guid_t>(std::move(type).value_unsafe(), index);
        if (ret != error_code::SUCCESS)
            return ret;
        auto data = object.value_unsafe()["value"];
        if (data.error() != simdjson::SUCCESS)
            return (error_code)data.error();
        (void)(((ret = ReadByIndex<Ts>(std::move(data).value_unsafe(), value, index)) != error_code::SUCCESS) && ...);
        return ret;
    }
};

template <class T>
struct ReadHelper<skr::resource::TResourceHandle<T>> {
    static error_code Read(simdjson::ondemand::value&& json, skr::resource::TResourceHandle<T>& handle)
    {
        return skr::json::Read<skr_resource_handle_t>(std::move(json), (skr_resource_handle_t&)handle);
    }
};

template <class T>
error_code Read(simdjson::ondemand::value&& json, T& value)
{
    return ReadHelper<T>::Read(std::move(json), value);
}
} // namespace json
} // namespace skr
#else
typedef struct skr_json_reader_t skr_json_reader_t;
#endif