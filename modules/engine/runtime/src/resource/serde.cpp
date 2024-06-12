#include "SkrRT/resource/resource_handle.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

namespace skr::json
{
bool WriteTrait<skr_resource_handle_t>::Write(skr::archive::JsonWriter* writer, const skr_resource_handle_t& handle)
{
    return WriteTrait<skr_guid_t>::Write(writer, handle.get_serialized());
}

bool ReadTrait<skr_resource_handle_t>::Read(skr::archive::JsonReader* json, skr_resource_handle_t& handle)
{
    SkrZoneScopedN("json::ReadTrait<skr_resource_handle_t>::Read");
    skr::String view;
    json->String(view);
    {
        skr_guid_t       guid;
        if (!skr::guid::make_guid(view.u8_str(), guid))
            return false;
        handle.set_guid(guid);
    }
    return true;
}
} // namespace skr::json

namespace skr::binary
{
bool ReadTrait<skr_resource_handle_t>::Read(SBinaryReader* reader, skr_resource_handle_t& handle)
{
    skr_guid_t guid;
    if (!ReadTrait<skr_guid_t>::Read(reader, guid))
    {
        SKR_LOG_FATAL(u8"failed to read resource handle guid! ret code: %d", -1);
        return false;
    }
    handle.set_guid(guid);
    return true;
}
bool WriteTrait<skr_resource_handle_t>::Write(SBinaryWriter* writer, const skr_resource_handle_t& handle)
{
    return WriteTrait<skr_guid_t>::Write(writer, handle.get_serialized());
}
} // namespace skr::binary