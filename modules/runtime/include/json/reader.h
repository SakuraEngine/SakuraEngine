#pragma once
#include "platform/configure.h"

#if defined(__cplusplus)
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include "simdjson.h"
#include "containers/hashmap.hpp"

// forward declaration for resources
struct skr_resource_handle_t;
namespace skr::resource { template <class T> struct TResourceHandle; }
// end forward declaration for resources

struct RUNTIME_API skr_json_reader_t {
    simdjson::ondemand::value* json;
};
typedef struct skr_guid_t skr_guid_t;
// utils for codegen
namespace skr
{
namespace json
{
enum error_code
{
    SUCCESS = 0,                ///< No error
    CAPACITY,                   ///< This parser can't support a document that big
    MEMALLOC,                   ///< Error allocating memory, most likely out of memory
    TAPE_ERROR,                 ///< Something went wrong while writing to the tape (stage 2), this is a generic error
    DEPTH_ERROR,                ///< Your document exceeds the user-specified depth limitation
    STRING_ERROR,               ///< Problem while parsing a string
    T_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 't'
    F_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 'f'
    N_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 'n'
    NUMBER_ERROR,               ///< Problem while parsing a number
    UTF8_ERROR,                 ///< the input is not valid UTF-8
    UNINITIALIZED,              ///< unknown error, or uninitialized document
    EMPTY,                      ///< no structural element found
    UNESCAPED_CHARS,            ///< found unescaped characters in a string.
    UNCLOSED_STRING,            ///< missing quote at the end
    UNSUPPORTED_ARCHITECTURE,   ///< unsupported architecture
    INCORRECT_TYPE,             ///< JSON element has a different type than user expected
    NUMBER_OUT_OF_RANGE,        ///< JSON number does not fit in 64 bits
    INDEX_OUT_OF_BOUNDS,        ///< JSON array index too large
    NO_SUCH_FIELD,              ///< JSON field not found in object
    IO_ERROR,                   ///< Error reading a file
    INVALID_JSON_POINTER,       ///< Invalid JSON pointer reference
    INVALID_URI_FRAGMENT,       ///< Invalid URI fragment
    UNEXPECTED_ERROR,           ///< indicative of a bug in simdjson
    PARSER_IN_USE,              ///< parser is already in use.
    OUT_OF_ORDER_ITERATION,     ///< tried to iterate an array or object out of order
    INSUFFICIENT_PADDING,       ///< The JSON doesn't have enough padding for simdjson to safely parse it.
    INCOMPLETE_ARRAY_OR_OBJECT, ///< The document ends early.
    SCALAR_DOCUMENT_AS_VALUE,   ///< A scalar document is treated as a value.
    OUT_OF_BOUNDS,              ///< Attempted to access location outside of document.
    NUM_JSON_ERROR_CODES,
    ENUMERATOR_ERROR,
    GUID_ERROR,
    NUM_ERROR_CODES,
};
struct error_code_info {
    error_code code;
    const char* message; // do not use a fancy std::string where a simple C string will do (no alloc, no destructor)
};
RUNTIME_API const char* error_message(error_code err) noexcept;
RUNTIME_API void set_error_message(error_code err) noexcept;

template <class T>
std::enable_if_t<!std::is_enum_v<T>, error_code> ReadValue(simdjson::ondemand::value&& json, T& value)
{
    static_assert(!sizeof(T), "ReadValue not implemented for this type, please include the appropriate generated header!");
    return error_code::SUCCESS;
}

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
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, double& b);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, eastl::string& guid);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, struct skr_guid_t& guid);
template <>
RUNTIME_API error_code ReadValue(simdjson::ondemand::value&& json, skr_resource_handle_t& handle);

template <class T>
std::enable_if_t<std::is_enum_v<T>, error_code> ReadValue(simdjson::ondemand::value&& json, T& value)
{
    return ReadValue(std::move(json), reinterpret_cast<std::underlying_type_t<T>&>(value));
}

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