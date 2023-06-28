#pragma once
#include "platform/memory.h"
#include "log_base.hpp"

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
    eastl::fixed_function<8 * sizeof(uint64_t), bool(LogFormatter&)> format_;
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

// Capture args and add them as additional arguments
template <typename Lambda, typename ... Args>
auto capture_call(Lambda&& lambda, Args&& ... args)
{
    return [
        lambda = std::forward<Lambda>(lambda),
        capture_args = std::make_tuple(skr::forward<Args>(args) ...)
    ](auto&& ... original_args) mutable {
        return std::apply([&lambda](auto&& ... args){
            lambda(std::forward<decltype(args)>(args) ...);
        }, std::tuple_cat(
            std::forward_as_tuple(original_args ...),
            std::apply([](auto&& ... args){
                return std::forward_as_tuple< Args ... >(
                    std::move(args) ...);
            }, std::move(capture_args))
        ));
    };
}

template <typename...Args>
void ArgsList::push(Args&&...args) SKR_NOEXCEPT
{
    auto f_format_ = capture_call(
        [](skr::string& format, LogFormatter& formatter, Args&& ... args)
        {
            formatter.formatted_string = skr::format(format, skr::forward<Args>(args)...);
            return true;
        }, skr::forward(args)...
    );
}

} // namespace log
} // namespace skr