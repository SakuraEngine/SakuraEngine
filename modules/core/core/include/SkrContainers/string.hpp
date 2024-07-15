#pragma once
#include "SkrContainersDef/string.hpp"

// bin serde
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
namespace skr::binary
{
template <>
struct SKR_STATIC_API ReadTrait<skr::String> {
    static bool Read(SBinaryReader* reader, skr::String& str);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::String> {
    static bool Write(SBinaryWriter* writer, const skr::String& str);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::StringView> {
    static bool Write(SBinaryWriter* writer, const skr::StringView& str);
};
} // namespace skr::binary

// json serde
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
namespace skr::json
{
template <>
struct SKR_STATIC_API ReadTrait<skr::String> {
    static bool Read(skr::archive::JsonReader* json, skr::String& value);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::StringView> {
    static bool Write(skr::archive::JsonWriter* writer, const skr::StringView& str);
};
template <>
struct SKR_STATIC_API WriteTrait<skr::String> {
    static bool Write(skr::archive::JsonWriter* writer, const skr::String& str);
};
} // namespace skr::json