#include "SkrRT/resource/resource_handle.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

namespace skr::json
{
void WriteTrait<skr_resource_handle_t>::Write(skr_json_writer_t* writer, const skr_resource_handle_t& handle)
{
    WriteTrait<skr_guid_t>::Write(writer, handle.get_serialized());
}
error_code ReadTrait<skr_resource_handle_t>::Read(simdjson::ondemand::value&& json, skr_resource_handle_t& handle)
{
    SkrZoneScopedN("json::ReadTrait<skr_resource_handle_t>::Read");
    auto result = json.get_string();
    if (result.error() == simdjson::SUCCESS)
    {
        std::string_view view = result.value_unsafe();
        skr_guid_t       guid;
        if (!skr::guid::make_guid({ (const char8_t*)view.data(), view.length() }, guid))
        {
            return error_code::GUID_ERROR;
        }
        handle.set_guid(guid);
    }
    return (error_code)result.error();
}
} // namespace skr::json

namespace skr::binary
{
int ReadTrait<skr_resource_handle_t>::Read(skr_binary_reader_t* reader, skr_resource_handle_t& handle)
{
    skr_guid_t guid;
    int        ret = ReadTrait<skr_guid_t>::Read(reader, guid);
    if (ret != 0)
    {
        SKR_LOG_FATAL(u8"failed to read resource handle guid! ret code: %d", ret);
        return ret;
    }
    handle.set_guid(guid);
    return ret;
}
int WriteTrait<skr_resource_handle_t>::Write(skr_binary_writer_t* writer, const skr_resource_handle_t& handle)
{
    return WriteTrait<skr_guid_t>::Write(writer, handle.get_serialized());
}
} // namespace skr::binary