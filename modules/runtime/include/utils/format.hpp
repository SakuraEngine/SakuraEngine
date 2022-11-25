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
        return format_to(ctx.out(), "{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}"
        , g.Data1(), g.Data2(), g.Data3()
        , g.Data4(0), g.Data4(1), g.Data4(2), g.Data4(3), g.Data4(4), g.Data4(5), g.Data4(6), g.Data4(7));
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
        return format_to(ctx.out(), "{:08X}{:08X}{:08X}{:08X}", md5.a, md5.b, md5.c, md5.d);
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