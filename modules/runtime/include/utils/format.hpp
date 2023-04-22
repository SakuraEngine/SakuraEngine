#pragma once
#include <containers/string.hpp>
#include "utils/types.h"
#include "fmt/core.h"
#include "fmt/format.h"

namespace fmt
{
#if !defined(SKR_USE_STL_STRING)
template <typename Char>
struct formatter<skr::basic_string<Char>, Char> : formatter<basic_string_view<Char>, Char> {
    template <typename FormatContext>
    auto format(eastl::basic_string<Char> const& val, FormatContext& ctx) const
    -> decltype(ctx.out())
    {
        return formatter<basic_string_view<Char>, Char>::format(basic_string_view<Char>(val.data(), val.size()), ctx);
    }
};

template <typename Char>
struct formatter<skr::basic_string_view<Char>, Char> : formatter<basic_string_view<Char>, Char> {
    template <typename FormatContext>
    auto format(eastl::basic_string_view<Char> const& val, FormatContext& ctx) const
    -> decltype(ctx.out())
    {
        return formatter<basic_string_view<Char>, Char>::format(basic_string_view<Char>(val.data(), val.size()), ctx);
    }
};
#endif

template <>
struct formatter<skr_guid_t> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(skr_guid_t const& g, FormatContext& ctx) const
    -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}"
        , g.Data1(), g.Data2(), g.Data3()
        , g.Data4(0), g.Data4(1), g.Data4(2), g.Data4(3)
        , g.Data4(4), g.Data4(5), g.Data4(6), g.Data4(7));
    }
};

template <>
struct formatter<skr_md5_t> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(skr_md5_t const& md5, FormatContext& ctx) const
    -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), 
            "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", 
            md5.digest[0], md5.digest[1], md5.digest[2], md5.digest[3],
            md5.digest[4], md5.digest[5], md5.digest[6], md5.digest[7],
            md5.digest[8], md5.digest[9], md5.digest[10], md5.digest[11],
            md5.digest[12], md5.digest[13], md5.digest[14], md5.digest[15]
        );
    }
};
} // namespace fmt

namespace skr
{
template <typename... T>
auto format(fmt::string_view fmt, T&&... args) -> skr::string
{
    auto buffer = fmt::memory_buffer();
    fmt::detail::vformat_to(buffer, fmt, fmt::make_format_args(args...));
    return skr::string{ buffer.data(), buffer.size() };
}
}