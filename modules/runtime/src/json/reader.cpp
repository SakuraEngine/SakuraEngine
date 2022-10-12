#include "json/reader.h"
#include "EASTL/string.h"
#include "platform/guid.hpp"

namespace skr::json
{
// TODO: error handling
template <>
void ReadValue(simdjson::ondemand::value&& json, bool& b) { b = json.get_bool().value_unsafe(); }
template <>
void ReadValue(simdjson::ondemand::value&& json, int32_t& b) { b = (int32_t)json.get_int64().value_unsafe(); }
template <>
void ReadValue(simdjson::ondemand::value&& json, uint32_t& b) { b = (uint32_t)json.get_uint64().value_unsafe(); }
template <>
void ReadValue(simdjson::ondemand::value&& json, int64_t& b) { b = json.get_int64().value_unsafe(); }
template <>
void ReadValue(simdjson::ondemand::value&& json, uint64_t& b) { b = json.get_uint64().value_unsafe(); }
template <>
void ReadValue(simdjson::ondemand::value&& json, double& b) { b = json.get_double().value_unsafe(); }
template <>
void ReadValue(simdjson::ondemand::value&& json, eastl::string& str)
{
    std::string_view view = json.get_string().value_unsafe();
    str = eastl::string(view.data(), view.length());
}
template <>
void ReadValue(simdjson::ondemand::value&& json, skr_guid_t& guid)
{
    std::string_view view = json.get_string().value_unsafe();
    guid = skr::guid::make_guid({ view.data(), view.length() });
}
template <>
void ReadValue(simdjson::ondemand::value&& json, skr_resource_handle_t& handle)
{
    std::string_view view = json.get_string().value_unsafe();
    handle.set_guid(skr::guid::make_guid( { view.data(), view.length() }));
}
} // namespace skr::json