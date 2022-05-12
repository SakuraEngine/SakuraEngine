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
    auto str = format("{}", guid);
    writer->String(str.data(), (TSize)str.size());
}
template <>
void Write(skr_json_writer_t* writer, const skr_resource_handle_t& handle)
{
    Write<const skr_guid_t&>(writer, handle.get_serialized());
}
} // namespace skr::json