#pragma once
#include "SkrBase/config.h"
#include "SkrBase/types.h"
#include "OpenString/text.h"
#include "OpenString/format.h"

namespace skr
{
using namespace ostr;
using String      = ostr::text;
using StringView = ostr::text_view;

template <>
struct Hash<String> {
    inline size_t operator()(const String& x) const { return ostr::hash_sequence_crc64(x.c_str(), x.size()); }
};

template <>
struct Hash<StringView> {
    inline size_t operator()(const StringView& x) const { return ostr::hash_sequence_crc64(x.raw().data(), x.size()); }
};

namespace StringLiterals
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

namespace skr::binary
{
template <>
struct BlobBuilderType<skr::StringView> {
    using type = skr::String;
};
} // namespace skr::binary

// binary reader
namespace skr
{
namespace binary
{
template <>
struct SKR_STATIC_API ReadTrait<skr::String> {
    static int Read(skr_binary_reader_t* reader, skr::String& str);
};

template <>
struct SKR_STATIC_API ReadTrait<skr::StringView> {
    static int Read(skr_binary_reader_t* reader, skr_blob_arena_t& arena, skr::StringView& str);
};
} // namespace binary
} // namespace skr

// binary writer
namespace skr
{
namespace binary
{

template <>
struct SKR_STATIC_API WriteTrait<skr::String> {
    static int Write(skr_binary_writer_t* writer, const skr::String& str);
};

template <>
struct SKR_STATIC_API WriteTrait<skr::StringView> {
    static int Write(skr_binary_writer_t* writer, const skr::StringView& str);
    static int Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const skr::StringView& str);
};

} // namespace binary
} // namespace skr