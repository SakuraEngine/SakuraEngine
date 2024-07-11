#pragma once
#include "SkrBase/config.h"
#include "SkrBase/types.h"
#include "OpenString/text.h"
#include "OpenString/format.h"

namespace skr
{
using namespace ostr;
using String               = ostr::text;
using StringView           = ostr::text_view;
using SerializeConstString = String;

SKR_STATIC_API bool guid_from_sv(const skr::StringView& str, skr_guid_t& value);

template <>
struct Hash<String> {
    inline size_t operator()(const String& x) const
    {
        return ostr::hash_sequence_crc64(x.c_str(), x.size());
    }
    inline size_t operator()(const StringView& x) const
    {
        return ostr::hash_sequence_crc64(x.raw().data(), x.size());
    }
    inline size_t operator()(const char* x) const
    {
        return ostr::hash_sequence_crc64(x, std::strlen(x));
    }
    inline size_t operator()(const char8_t* x) const
    {
        return ostr::hash_sequence_crc64(x, std::strlen(reinterpret_cast<const char*>(x)));
    }
};

template <>
struct Hash<StringView> {
    inline size_t operator()(const StringView& x) const
    {
        return ostr::hash_sequence_crc64(x.raw().data(), x.size());
    }
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
        u8"{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", g.data1(), g.data2(), g.data3(), g.data4(0), g.data4(1), g.data4(2), g.data4(3), g.data4(4), g.data4(5), g.data4(6), g.data4(7));
    }
};

} // namespace ostr

// binary reader
namespace skr
{
namespace binary
{
template <>
struct SKR_STATIC_API ReadTrait<skr::String> {
    static bool Read(SBinaryReader* reader, skr::String& str);
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
    static bool Write(SBinaryWriter* writer, const skr::String& str);
};

template <>
struct SKR_STATIC_API WriteTrait<skr::StringView> {
    static bool Write(SBinaryWriter* writer, const skr::StringView& str);
};

} // namespace binary
} // namespace skr