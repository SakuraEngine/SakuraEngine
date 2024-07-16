#pragma once
#include "SkrContainersDef/string.hpp"

// bin serde
#include "SkrCore/log.h"
#include "SkrContainersDef/vector.hpp"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
namespace skr::binary
{
template <>
struct SKR_STATIC_API ReadTrait<skr::String> {
    inline static bool Read(SBinaryReader* reader, skr::String& str)
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
};
template <>
struct SKR_STATIC_API WriteTrait<skr::String> {
    inline static bool Write(SBinaryWriter* writer, const skr::String& str)
    {
        if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
            return false;
        return writer->write(str.u8_str(), str.size());
    }
};
template <>
struct SKR_STATIC_API WriteTrait<skr::StringView> {
    inline static bool Write(SBinaryWriter* writer, const skr::StringView& str)
    {
        if (!WriteTrait<uint32_t>::Write(writer, (uint32_t)str.size()))
            return true;
        return writer->write(str.raw().data(), str.size());
    }
};
} // namespace skr::binary

// json serde
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
namespace skr::json
{
template <>
struct SKR_STATIC_API ReadTrait<skr::String> {
    inline static bool Read(skr::archive::JsonReader* json, skr::String& value)
    {
        SkrZoneScopedN("json::ReadTrait<skr::String>::Read");

        if (json->String(value).has_value())
            return true;
        return false;
    }
};
template <>
struct SKR_STATIC_API WriteTrait<skr::StringView> {
    inline static bool Write(skr::archive::JsonWriter* writer, const skr::StringView& str)
    {
        if (!(writer->String(str)).has_value()) return false;
        return true;
    }
};
template <>
struct SKR_STATIC_API WriteTrait<skr::String> {
    inline static bool Write(skr::archive::JsonWriter* writer, const skr::String& str)
    {
        if (!(writer->String(str.view())).has_value()) return false;
        return true;
    }
};
} // namespace skr::json