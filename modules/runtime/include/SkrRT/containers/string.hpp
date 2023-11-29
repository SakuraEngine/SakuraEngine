#pragma once
#include "SkrRT/config.h"
#include "OpenString/text.h"
#include "OpenString/format.h"
#include "SkrRT/misc/types.h"

namespace skr
{
using namespace ostr;
using string      = ostr::text;
using string_view = ostr::text_view;

template <>
struct Hash<string> {
    inline size_t operator()(const string& x) const { return ostr::hash_sequence_crc64(x.c_str(), x.size()); }
};

template <>
struct Hash<string_view> {
    inline size_t operator()(const string_view& x) const { return ostr::hash_sequence_crc64(x.raw().data(), x.size()); }
};

namespace string_literals
{
}

} // namespace skr

namespace ostr
{

#if __cpp_char8_t
template <>
struct argument_formatter<const char*> {
    static codeunit_sequence produce(const char* raw, const codeunit_sequence_view& specification)
    {
        // we cast directly to c8* because we use source encoding as utf8
        return skr::format(u8"{}", (const ochar8_t*)raw);
    }
};
#endif

template <>
struct argument_formatter<skr_md5_t> {
    static codeunit_sequence produce(const skr_md5_t& md5, const codeunit_sequence_view& specification)
    {
        return skr::format(
        u8"{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        md5.digest[0], md5.digest[1], md5.digest[2], md5.digest[3],
        md5.digest[4], md5.digest[5], md5.digest[6], md5.digest[7],
        md5.digest[8], md5.digest[9], md5.digest[10], md5.digest[11],
        md5.digest[12], md5.digest[13], md5.digest[14], md5.digest[15]);
    }
};

template <>
struct argument_formatter<skr_guid_t> {
    static codeunit_sequence produce(const skr_guid_t& g, const codeunit_sequence_view& specification)
    {
        return skr::format(
        u8"{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", g.Data1(), g.Data2(), g.Data3(), g.Data4(0), g.Data4(1), g.Data4(2), g.Data4(3), g.Data4(4), g.Data4(5), g.Data4(6), g.Data4(7));
    }
};

} // namespace ostr

#include "SkrRT/serde/binary/blob_fwd.h"
namespace skr::binary
{
template <>
struct BlobBuilderType<skr::string_view> {
    using type = skr::string;
};
} // namespace skr::binary

// binary reader
#include "SkrRT/serde/binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template <>
struct SKR_STATIC_API ReadTrait<skr::string> {
    static int Read(skr_binary_reader_t* reader, skr::string& str);
};

template <>
struct SKR_STATIC_API ReadTrait<skr::string_view> {
    static int Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena, skr::string_view& str);
};
} // namespace binary
} // namespace skr

// binary writer
#include "SkrRT/serde/binary/writer_fwd.h"

namespace skr
{
namespace binary
{

template <>
struct SKR_STATIC_API WriteTrait<skr::string> {
    static int Write(skr_binary_writer_t* writer, const skr::string& str);
};

template <>
struct SKR_STATIC_API WriteTrait<skr::string_view> {
    static int Write(skr_binary_writer_t* writer, const skr::string_view& str);
    static int Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const skr::string_view& str);
};

} // namespace binary
} // namespace skr