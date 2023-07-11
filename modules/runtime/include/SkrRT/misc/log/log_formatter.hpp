#pragma once
#include "SkrRT/platform/memory.h"
#include "SkrRT/containers/string.hpp"

#include <tuple>
#include <EASTL/fixed_function.h>

namespace skr {
namespace log {

struct LogFormatter;

struct ArgsList 
{
    template <typename...Args>
    void push(Args&&...args) SKR_NOEXCEPT;

protected:
    friend struct LogFormatter;
    eastl::fixed_function<8 * sizeof(uint64_t), bool(const skr::string& format, LogFormatter&)> format_;
};

struct LogFormatter
{
    ~LogFormatter() SKR_NOEXCEPT;

    [[nodiscard]] skr::string const& format(
        const skr::string& format,
        const ArgsList& args_list
    );

    skr::string formatted_string = u8"";
};

template <typename...Args>
FORCEINLINE void ArgsList::push(Args&&...args) SKR_NOEXCEPT
{
    format_ = [args = std::make_tuple(std::forward<Args>(args) ...)](const skr::string& format, LogFormatter& formatter) mutable {
        return std::apply([&](auto&& ... args){
            formatter.formatted_string = skr::format(format, skr::forward<Args>(args)...);
            return true;
        }, std::move(args));
    };
}

} // namespace log
} // namespace skr