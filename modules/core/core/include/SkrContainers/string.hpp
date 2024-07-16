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
#include "SkrSerde/json_serde.hpp"
namespace skr
{
template <>
struct JsonSerde<skr::String> {
    inline static bool read(skr::archive::JsonReader* r, skr::String& v)
    {
        SkrZoneScopedN("json::JsonSerde<skr::String>::read");

        SKR_EXPECTED_CHECK(r->String(v), false);
        return true;
    }
    inline static bool write(skr::archive::JsonWriter* w, const skr::String& v)
    {
        SKR_EXPECTED_CHECK(w->String(v.view()), false);
        return true;
    }
};
template <>
struct JsonSerde<skr::StringView> {
    inline static bool write(skr::archive::JsonWriter* w, const skr::StringView& v)
    {
        SKR_EXPECTED_CHECK(w->String(v), false);
        return true;
    }
};
} // namespace skr
