#pragma once
#include "SkrBase/config.h"
#include "SkrBase/types.h"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrBase/misc/hash.h"
#include "SkrBase/containers/string/string_memory.hpp"
#include "SkrBase/containers/string/string.hpp"
#include "SkrBase/containers/string/format.hpp"
#include <SkrBase/type_info.hpp>

namespace skr
{
constexpr uint64_t kStringSSOSize = 31;

// string types
using String [[sfinal_alias]] = container::U8String<container::StringMemory<
    skr_char8,      /*type*/
    uint64_t,       /*size type*/
    kStringSSOSize, /*sso size*/
    SkrAllocator    /*allocator*/
    >>;
using StringView [[sfinal_alias]] = container::U8StringView<uint64_t>;
using SerializeConstString [[sfinal_alias]] = String;

// format
template <typename... Args>
inline void format_to(String& out, StringView fmt, Args&&... args)
{
    container::format_to(out, fmt, std::forward<Args>(args)...);
}
template <typename... Args>
inline String format(StringView fmt, Args&&... args)
{
    return container::format<String>(fmt, std::forward<Args>(args)...);
}

// hash
template <>
struct Hash<String>
{
    inline size_t operator()(const String& x) const
    {
        return skr_hash_of(x.c_str(), x.size(), 0);
    }
    inline skr_hash operator()(const StringView& x) const
    {
        return skr_hash_of(x.data(), x.size(), 0);
    }
    inline skr_hash operator()(const char* x) const
    {
        return skr_hash_of(x, std::strlen(x), 0);
    }
    inline skr_hash operator()(const char8_t* x) const
    {
        return skr_hash_of(x, std::strlen(reinterpret_cast<const char*>(x)), 0);
    }
};
template <>
struct Hash<StringView>
{
    inline skr_hash operator()(const StringView& x) const
    {
        return skr_hash_of(x.data(), x.size(), 0);
    }
};
} // namespace skr

// skr type integration
namespace skr::container
{
template <>
struct Formatter<MD5>
{
    template <typename TString>
    inline static void format(TString& out, const MD5& md5, typename TString::ViewType spec)
    {
        auto ref = out.add_unsafe(MD5::kHexLength);
        md5.encode_hex(ref.ptr());
    }
};
template <>
struct Formatter<GUID>
{
    template <typename TString>
    inline static void format(TString& out, const GUID& guid, typename TString::ViewType spec)
    {
        auto ref = out.add_unsafe(GUID::kHexSpecLength);
        guid.encode_spec(ref.ptr());
    }
};
template <>
struct Formatter<SHA256>
{
    template <typename TString>
    inline static void format(TString& out, const SHA256& sha256, typename TString::ViewType spec)
    {
        auto ref = out.add_unsafe(SHA256::kHexLength);
        sha256.encode_hex(ref.ptr());
    }
};
} // namespace skr::container

SKR_TYPE_INFO(skr::String, "214ED643-54BD-4213-BE37-E336A77FDE84");
SKR_TYPE_INFO(skr::StringView, "B799BA81-6009-405D-9131-E4B6101660DC");