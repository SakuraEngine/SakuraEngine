#pragma once
#include "SkrContainersDef/string.hpp"

// bin serde
#include "SkrCore/log.h"
#include "SkrContainersDef/vector.hpp"
#include "SkrSerde/bin_serde.hpp"
namespace skr
{
template <>
struct BinSerde<skr::String> {
    inline static bool read(SBinaryReader* r, skr::String& v)
    {
        using SizeType = decltype(v.size());

        // read size
        SizeType size;
        if (!bin_read(r, size))
        {
            SKR_LOG_FATAL(u8"failed to read string buffer size!");
            return false;
        }

        // read content
        skr::InlineVector<char8_t, 64> temp;
        temp.resize_default(size);
        if (!r->read((void*)temp.data(), temp.size()))
        {
            SKR_LOG_FATAL(u8"failed to read string buffer size!");
            return false;
        }

        // move content
        v = skr::String(skr::StringView((const char8_t*)temp.data(), (int32_t)temp.size()));
        return true;
    }
    inline static bool write(SBinaryWriter* w, const skr::String& v)
    {
        // write size
        if (!bin_write(w, v.size()))
            return false;

        // write content
        return w->write(v.u8_str(), v.size());
    }
};
} // namespace skr

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