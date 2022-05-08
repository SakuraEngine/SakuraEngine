#pragma once
#include "platform/configure.h"
#include "resource/resource_handle.h"
#include "simdjson.h"

#if defined(__cplusplus)
    #include "EASTL/string.h"
struct RUNTIME_API skr_json_reader_t {
    simdjson::ondemand::value* json;
};
typedef struct skr_guid_t skr_guid_t;
// utils for codegen
namespace skr
{
namespace json
{
template <class T>
void Read(simdjson::ondemand::value&& json, T& value);

template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, bool& b);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, int32_t& b);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, uint32_t& b);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, int64_t& b);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, uint64_t& b);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, double& b);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, eastl::string& guid);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, struct skr_guid_t& guid);
template <>
RUNTIME_API void Read(simdjson::ondemand::value&& json, struct skr_resource_handle_t& handle);
template <class T>
void Read(simdjson::ondemand::value&& json, skr::resource::TResourceHandle<T>& handle)
{
    Read(json, (skr_resource_handle_t&)handle);
}
} // namespace json
} // namespace skr
#else
typedef struct skr_json_reader_t skr_json_reader_t;
#endif