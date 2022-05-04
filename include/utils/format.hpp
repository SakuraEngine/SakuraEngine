#include "fmt/core.h"
#include "fmt/format.h"
#include "EASTL/string.h"

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
} // namespace fmt
template <typename... T>
auto format(fmt::string_view fmt, T&&... args)
-> eastl::string
{
    auto buffer = fmt::memory_buffer();
    fmt::detail::vformat_to(buffer, fmt, fmt::make_format_args(args...));
    return eastl::string{ buffer.data(), buffer.size() };
}
