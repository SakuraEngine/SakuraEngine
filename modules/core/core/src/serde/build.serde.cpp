// binary
#include "binary/reader.cpp"
#include "binary/writer.cpp"

// json
#include "json/reader.cpp"
#include "json/writer.cpp"

// util include
#include "SkrCore/log.h"
#include "SkrContainersDef/vector.hpp"

// string bin
#include "SkrContainers/string.hpp"
namespace skr::binary
{
bool ReadTrait<skr::String>::Read(SBinaryReader* reader, skr::String& str)
{
    uint32_t size;
    if (!ReadTrait<uint32_t>::Read(reader, size))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer size!");
        return false;
    }
    skr::InlineVector<char8_t, 64> temp;
    temp.resize_default(size);
    if (!reader->read((void*)temp.data(), temp.size()))
    {
        SKR_LOG_FATAL(u8"failed to read string buffer size!");
        return false;
    }
    str = skr::String(skr::StringView((const char8_t*)temp.data(), (int32_t)temp.size()));
    return true;
}
bool WriteTrait<skr::String>::Write(SBinaryWriter* writer, const skr::String& str)
{
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
        return false;
    return writer->write(str.u8_str(), str.size());
}
bool WriteTrait<skr::StringView>::Write(SBinaryWriter* writer, const skr::StringView& str)
{
    if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
        return true;
    return writer->write(str.raw().data(), str.size());
}
} // namespace skr::binary

namespace skr::json
{
bool ReadTrait<skr::String>::Read(skr::archive::JsonReader* _json, skr::String& value)
{
    SkrZoneScopedN("json::ReadTrait<skr::String>::Read");

    if (_json->String(value).has_value())
        return true;
    return false;
}
bool WriteTrait<skr::StringView>::Write(skr::archive::JsonWriter* writer, const skr::StringView& str)
{
    if (!(writer->String(str)).has_value()) return false;
    return true;
}

bool WriteTrait<skr::String>::Write(skr::archive::JsonWriter* writer, const skr::String& str)
{
    if (!(writer->String(str.view())).has_value()) return false;
    return true;
}
} // namespace skr::json