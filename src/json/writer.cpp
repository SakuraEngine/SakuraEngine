#include "json/writer.h"
#include "platform/guid.h"
#include "utils/format.hpp"

namespace skr::json
{
template <>
void Write(skr_json_writer_t* writer, bool b) { writer->Bool(b); }
template <>
void Write(skr_json_writer_t* writer, int32_t b) { writer->Int(b); }
template <>
void Write(skr_json_writer_t* writer, uint32_t b) { writer->UInt(b); }
template <>
void Write(skr_json_writer_t* writer, int64_t b) { writer->Int64(b); }
template <>
void Write(skr_json_writer_t* writer, uint64_t b) { writer->UInt64(b); }
template <>
void Write(skr_json_writer_t* writer, double b) { writer->Double(b); }
template <>
void Write(skr_json_writer_t* writer, const eastl::string& str) { writer->String(str.data(), (TSize)str.size()); }
template <>
void Write(skr_json_writer_t* writer, const skr_guid_t& guid)
{
    auto str = format("{8x}-{4x}-{4x}-{2x}{2x}-{2x}{2x}{2x}{2x}{2x}{2x}}", guid.Data1, guid.Data2, guid.Data3,
    guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
    guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    writer->String(str.data(), (TSize)str.size());
}
} // namespace skr::json