#include "resource/resource_handle.h"
#include "json/reader.h"
#include "EASTL/string.h"
#include "utils/log.h"

namespace skr::json
{
error_code_info error_infos[error_code::NUM_ERROR_CODES - error_code::NUM_JSON_ERROR_CODES] = 
{
    {ENUMERATOR_ERROR, "Invalid enumerator liternal"},
    {GUID_ERROR, "Invalid GUID"},
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

template <>
error_code ReadValue(simdjson::ondemand::value&& json, bool& b)
{
    auto result = json.get_bool();
    if (result.error() == simdjson::SUCCESS)
        b = result.value_unsafe();
    return (error_code)result.error();
}
template <>
error_code ReadValue(simdjson::ondemand::value&& json, int32_t& b)
{
    auto result = json.get_int64();
    if (result.error() == simdjson::SUCCESS)
        b = (int32_t)result.value_unsafe();
    return (error_code)result.error();
}
template <>
error_code ReadValue(simdjson::ondemand::value&& json, uint32_t& b)
{
    auto result = json.get_uint64();
    if (result.error() == simdjson::SUCCESS)
        b = (uint32_t)result.value_unsafe();
    return (error_code)result.error();
}
template <>
error_code ReadValue(simdjson::ondemand::value&& json, int64_t& b)
{
    auto result = json.get_int64();
    if (result.error() == simdjson::SUCCESS)
        b = result.value_unsafe();
    return (error_code)result.error();
}
template <>
error_code ReadValue(simdjson::ondemand::value&& json, uint64_t& b)
{
    auto result = json.get_uint64();
    if (result.error() == simdjson::SUCCESS)
        b = result.value_unsafe();
    return (error_code)result.error();
}
template <>
error_code ReadValue(simdjson::ondemand::value&& json, float& f)
{
    auto result = json.get_double();
    if (result.error() == simdjson::SUCCESS)
        f = static_cast<float>(result.value_unsafe());
    return (error_code)result.error();
}
template <>
error_code ReadValue(simdjson::ondemand::value&& json, double& b)
{
    auto result = json.get_double();
    if (result.error() == simdjson::SUCCESS)
        b = result.value_unsafe();
    return (error_code)result.error();
}
template <>
error_code ReadValue(simdjson::ondemand::value&& json, eastl::string& str)
{

    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        str = eastl::string(view.data(), view.length());
    }
    return (error_code)result.error();
}

constexpr int parse_hex_digit(const char c)
{
    using namespace eastl::string_literals;
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else if ('A' <= c && c <= 'F')
        return 10 + c - 'A';
    else
        SKR_LOG_ERROR("Invalid character in GUID. Expected hex digit, got %c", c);
    return -1;
}

template <class T>
bool parse_hex(const char* ptr, T& value)
{
    constexpr size_t digits = sizeof(T) * 2;
    value = 0;
    for (size_t i = 0; i < digits; ++i)
    {
        int result = parse_hex_digit(ptr[i]);
        if(result < 0)
            return false;
        value |= result << (4 * (digits - i - 1));
    }
    return true;
}

bool make_guid_helper(const char* begin, skr_guid_t& value)
{
    uint32_t Data1 = 0;
    uint16_t Data2 = 0;
    uint16_t Data3 = 0;
    uint8_t Data4[8] = {};
    if(!parse_hex(begin, Data1))
        return false;
    begin += 8 + 1;
    if(!parse_hex(begin, Data2))
        return false;
    begin += 4 + 1;
    if(!parse_hex(begin, Data3))
        return false;
    begin += 4 + 1;
    if(!parse_hex(begin, Data4[0]))
        return false;
    begin += 2;
    if(!parse_hex(begin, Data4[1]))
        return false;
    begin += 2 + 1;
    for (size_t i = 0; i < 6; ++i)
        if(!parse_hex(begin + i * 2, Data4[i + 2]))
            return false;
    value = skr_guid_t(Data1, Data2, Data3, Data4);
    return true;
}

bool make_guid(const eastl::string_view& str, skr_guid_t& value)
{
    using namespace eastl::string_literals;
    constexpr size_t short_guid_form_length = 36;
    constexpr size_t long_guid_form_length = 38;

    if (str.size() != long_guid_form_length && str.size() != short_guid_form_length)
    {
        eastl::string str2(str.data(), str.size());
        SKR_LOG_ERROR("String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected, got %s", str2.c_str());
        return false;
    }

    if (str.size() == (long_guid_form_length + 1))
    {
        if (str[0] != '{' || str[long_guid_form_length - 1] != '}')
        {
            eastl::string str2(str.data(), str.size());
            SKR_LOG_ERROR("Opening or closing brace is expected, got %s", str2.c_str());
            return false;
        }
    }

    return make_guid_helper(str.data() + (str.size() == (long_guid_form_length + 1) ? 1 : 0), value);
}

template <>
error_code ReadValue(simdjson::ondemand::value&& json, skr_guid_t& guid)
{
    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        if(!make_guid({ view.data(), view.length() }, guid))
        {
            return error_code::GUID_ERROR;
        }
    }
    return (error_code)result.error();
}

template <>
error_code ReadValue(simdjson::ondemand::value&& json, skr_resource_handle_t& handle)
{
    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        skr_guid_t guid;
        if(!make_guid({ view.data(), view.length() }, guid))
        {
            return error_code::GUID_ERROR;
        }
        handle.set_guid(guid);
    }
    return (error_code)result.error();
}
} // namespace skr::json