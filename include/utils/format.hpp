#include "fmt/core.h"
#include "fmt/format.h"
#include "EASTL/string.h"
#include "platform/guid.h"

namespace fmt
{
template <typename Char>
struct formatter<eastl::basic_string<Char>, Char> : formatter<basic_string_view<Char>, Char> {
    template <typename FormatContext>
    auto format(eastl::basic_string<Char> const& val, FormatContext& ctx) const
    -> decltype(ctx.out())
    {
        return formatter<basic_string_view<Char>, Char>::format(basic_string_view<Char>(val.data(), val.size()), ctx);
    }
};
struct formatter<skr_guid_t> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(skr_guid_t const& g, FormatContext& ctx) const
    -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "({:8x}-{:4x}-{:4x}-{:2x}{:2x}-{:2x}{:2x}{:2x}{:2x}{:2x}{:2x})", g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);
    }
};
} // namespace fmt

template <typename... T>
auto format(fmt::string_view fmt, T&&... args)
-> eastl::string
{
    auto buffer = fmt::memory_buffer();
    fmt::detail::vformat_to(buffer, fmt, fmt::make_format_args(args...));
    return eastl::string{ buffer.data(), buffer.size() };
}
