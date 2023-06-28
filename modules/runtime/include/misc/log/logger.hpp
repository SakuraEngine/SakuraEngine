#pragma once
#include "log_formatter.hpp"

namespace skr {
namespace log {

template<typename T>
struct IsCopyableArgument {
    static constexpr bool value = std::is_copy_constructible_v<T>;
};

struct Logger
{
    Logger() SKR_NOEXCEPT;
    ~Logger() SKR_NOEXCEPT;

    template <typename...Args>
    void log(LogEvent ev, skr::string_view format, Args&&... args) SKR_NOEXCEPT
    {
        bool sucess = false;
        if (canPushToQueue())
        {
            constexpr bool copyable = checkCopyable(args...);
            if constexpr (copyable)
            {
                ArgsList<> args_list = {};
                args_list.push(skr::forward<Args>(args)...);
                sucess = tryPushToQueue(ev, format, skr::move(args_list));
            }
            else // foramt inplace, expensive
            {
                skr::string s = skr::format(format, skr::forward<Args>(args)...);
                sucess = tryPushToQueue(ev, skr::move(s));
            }
        }
        if (!sucess) // sink immediate
        {
            skr::string s = skr::format(format, skr::forward<Args>(args)...);
            printf("%s", s.c_str());
        }
    }

    void log(LogEvent ev, skr::string_view format, va_list va_args) SKR_NOEXCEPT
    {
        bool sucess = false;
        // va_list can only be formatted inplace
        skr::string fmt(format);
        char8_t buffer[1024];
        vsnprintf((char* const)buffer, sizeof(buffer), fmt.c_str(), va_args);

        if (canPushToQueue())
        {
            sucess = tryPushToQueue(ev, skr::string(buffer));
        }
        if (!sucess) // sink immediate
        {
            printf("%s", (const char*)buffer);
        }
    }

private:
    template <typename... Args>
    constexpr bool checkCopyable(Args&&... arg) const SKR_NOEXCEPT
    {
        return (IsCopyableArgument<Args>::value && ...);
    }

    bool canPushToQueue() const SKR_NOEXCEPT;
    bool tryPushToQueue(LogEvent ev, skr::string_view format, ArgsList<>&& args) SKR_NOEXCEPT;
    bool tryPushToQueue(LogEvent ev, skr::string&& what) SKR_NOEXCEPT;
    void notifyWorker() SKR_NOEXCEPT;
};

} } // namespace skr::log