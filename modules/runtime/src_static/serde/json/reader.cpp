#include "resource/resource_handle.h"
#include "serde/json/reader.h"
#include "EASTL/string.h"
#include "utils/log.h"

namespace skr::json
{

error_code_info error_infos[error_code::NUM_ERROR_CODES - error_code::NUM_JSON_ERROR_CODES] = {
    { ENUMERATOR_ERROR, "Invalid enumerator liternal" },
    { GUID_ERROR, "Invalid GUID" },
};
const char* error_message(error_code err) noexcept
{
    if (err < error_code::NUM_JSON_ERROR_CODES)
    {
        return simdjson::error_message(static_cast<simdjson::error_code>(err));
    }
    else
    {
        return error_infos[err - error_code::NUM_JSON_ERROR_CODES].message;
    }
}



error_code ReadTrait<bool>::Read(simdjson::ondemand::value&& json, bool& value)
{
    auto result = json.get_bool();
    if (result.error() == simdjson::SUCCESS)
        value = result.value_unsafe();
    return (error_code)result.error();
}

error_code ReadTrait<int32_t>::Read(simdjson::ondemand::value&& json, int32_t& value)
{
    auto result = json.get_int64();
    if (result.error() == simdjson::SUCCESS)
        value = (int32_t)result.value_unsafe();
    return (error_code)result.error();
}

error_code ReadTrait<uint32_t>::Read(simdjson::ondemand::value&& json, uint32_t& value)
{
    auto result = json.get_uint64();
    if (result.error() == simdjson::SUCCESS)
        value = (uint32_t)result.value_unsafe();
    return (error_code)result.error();
}

error_code ReadTrait<int64_t>::Read(simdjson::ondemand::value&& json, int64_t& value)
{
    auto result = json.get_int64();
    if (result.error() == simdjson::SUCCESS)
        value = result.value_unsafe();
    return (error_code)result.error();
}

error_code ReadTrait<uint64_t>::Read(simdjson::ondemand::value&& json, uint64_t& value)
{
    auto result = json.get_uint64();
    if (result.error() == simdjson::SUCCESS)
        value = result.value_unsafe();
    return (error_code)result.error();
}

error_code ReadTrait<float>::Read(simdjson::ondemand::value&& json, float& value)
{
    auto result = json.get_double();
    if (result.error() == simdjson::SUCCESS)
        value = (float)result.value_unsafe();
    return (error_code)result.error();
}

error_code ReadTrait<double>::Read(simdjson::ondemand::value&& json, double& value)
{
    auto result = json.get_double();
    if (result.error() == simdjson::SUCCESS)
        value = result.value_unsafe();
    return (error_code)result.error();
}

error_code ReadTrait<skr::string>::Read(simdjson::ondemand::value&& json, skr::string& value)
{
    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        value = skr::string(skr::string_view((const char8_t*)view.data(), view.length()));
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_md5_t>::Read(simdjson::ondemand::value&& json, skr_md5_t& value)
{
    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        if (!skr_parse_md5((const char8_t*)view.data(), &value))
        {
            return error_code::MD5_ERROR;
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_float2_t>::Read(simdjson::ondemand::value&& json, skr_float2_t& value)
{
    auto result = json.get_array();
    if (result.error() == simdjson::SUCCESS)
    {
        auto array = result.value_unsafe();
        auto count = array.count_elements();
        if(count.error() != simdjson::SUCCESS)
            return (error_code)count.error();
        if (count.value_unsafe() < 2)
            return error_code::CAPACITY;
        auto element = array.at(0);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.x = (float)result.value_unsafe();
        }
        element = array.at(1);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.y = (float)result.value_unsafe();
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_float3_t>::Read(simdjson::ondemand::value&& json, skr_float3_t& value)
{
    auto result = json.get_array();
    if (result.error() == simdjson::SUCCESS)
    {
        auto array = result.value_unsafe();
        auto count = array.count_elements();
        if(count.error() != simdjson::SUCCESS)
            return (error_code)count.error();
        if (count.value_unsafe() < 3)
            return error_code::CAPACITY;
        auto element = array.at(0);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.x = (float)result.value_unsafe();
        }
        element = array.at(1);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.y = (float)result.value_unsafe();
        }
        element = array.at(2);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.z = (float)result.value_unsafe();
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_float4_t>::Read(simdjson::ondemand::value&& json, skr_float4_t& value)
{
    auto result = json.get_array();
    if (result.error() == simdjson::SUCCESS)
    {
        auto array = result.value_unsafe();
        auto count = array.count_elements();
        if(count.error() != simdjson::SUCCESS)
            return (error_code)count.error();
        if (count.value_unsafe() < 4)
            return error_code::CAPACITY;
        auto element = array.at(0);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.x = (float)result.value_unsafe();
        }
        element = array.at(1);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.y = (float)result.value_unsafe();
        }
        element = array.at(2);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.z = (float)result.value_unsafe();
        }
        element = array.at(3);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.w = (float)result.value_unsafe();
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_rotator_t>::Read(simdjson::ondemand::value &&json, skr_rotator_t &value)
{
    auto result = json.get_array();
    if (result.error() == simdjson::SUCCESS)
    {
        auto array = result.value_unsafe();
        auto count = array.count_elements();
        if(count.error() != simdjson::SUCCESS)
            return (error_code)count.error();
        if (count.value_unsafe() < 3)
            return error_code::CAPACITY;
        auto element = array.at(0);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.pitch = (float)result.value_unsafe();
        }
        element = array.at(1);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.yaw = (float)result.value_unsafe();
        }
        element = array.at(2);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.roll = (float)result.value_unsafe();
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_quaternion_t>::Read(simdjson::ondemand::value &&json, skr_quaternion_t &value)
{
    auto result = json.get_array();
    if (result.error() == simdjson::SUCCESS)
    {
        auto array = result.value_unsafe();
        auto count = array.count_elements();
        if(count.error() != simdjson::SUCCESS)
            return (error_code)count.error();
        if (count.value_unsafe() < 4)
            return error_code::CAPACITY;
        auto element = array.at(0);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.x = (float)result.value_unsafe();
        }
        element = array.at(1);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.y = (float)result.value_unsafe();
        }
        element = array.at(2);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.z = (float)result.value_unsafe();
        }
        element = array.at(3);
        if (element.error() == simdjson::SUCCESS)
        {
            auto result = element.value_unsafe().get_double();
            if (result.error() == simdjson::SUCCESS)
                value.w = (float)result.value_unsafe();
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_float4x4_t>::Read(simdjson::ondemand::value &&json, skr_float4x4_t &value)
{
    auto result = json.get_array();
    if (result.error() == simdjson::SUCCESS)
    {
        auto array = result.value_unsafe();
        auto count = array.count_elements();
        if(count.error() != simdjson::SUCCESS)
            return (error_code)count.error();
        if (count.value_unsafe() < 16)
            return error_code::CAPACITY;
        for(int i = 0; i < 16; i++)
        {
            auto element = array.at(i);
            if (element.error() == simdjson::SUCCESS)
            {
                auto result = element.value_unsafe().get_double();
                if (result.error() == simdjson::SUCCESS)
                    value.M[i/4][i%4] = (float)result.value_unsafe();
            }
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_guid_t>::Read(simdjson::ondemand::value&& json, skr_guid_t& value)
{
    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        if (!skr::guid::make_guid({ (const char8_t*)view.data(), static_cast<int32_t>(view.length()) }, value))
        {
            return error_code::GUID_ERROR;
        }
    }
    return (error_code)result.error();
}

error_code ReadTrait<skr_resource_handle_t>::Read(simdjson::ondemand::value&& json, skr_resource_handle_t& handle)
{
    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        skr_guid_t guid;
        if (!skr::guid::make_guid({ (const char8_t*)view.data(), static_cast<int32_t>(view.length()) }, guid))
        {
            return error_code::GUID_ERROR;
        }
        handle.set_guid(guid);
    }
    return (error_code)result.error();
}
} // namespace skr::json