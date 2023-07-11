#pragma once
#include "SkrRT/misc/log/log_base.hpp"
#include "SkrRT/misc/log/log_formatter.hpp"

namespace skr {
namespace log {

template<typename Arg>
struct IsCStringFamily {
    using ArgType = std::decay_t<Arg>;
    static constexpr bool value =
        std::disjunction_v<std::is_same<ArgType, char const*>, std::is_same<ArgType, char*>> ||
        std::disjunction_v<std::is_same<ArgType, char8_t const*>, std::is_same<ArgType, char8_t*>> ||
        std::disjunction_v<std::is_same<ArgType, wchar_t const*>, std::is_same<ArgType, wchar_t*>>;
};

template<typename Arg>
struct IsCopyableArgument {
    using ArgType = std::decay_t<Arg>;
    static constexpr bool value = !IsCStringFamily<ArgType>::value && 
        std::is_trivially_copy_constructible_v<ArgType>;
};

template <typename... Args>
static constexpr bool checkArgsCopyable() SKR_NOEXCEPT
{
    return (IsCopyableArgument<Args>::value && ...);
}

struct RUNTIME_API Logger
{
    Logger(const char8_t* name) SKR_NOEXCEPT;
    ~Logger() SKR_NOEXCEPT;

    static Logger* GetDefault() SKR_NOEXCEPT;

    template <typename...Args>
    void log(LogEvent ev, skr::string_view format, Args&&... args) SKR_NOEXCEPT
    {
        bool sucess = false;
        if (canPushToQueue())
        {
            constexpr bool copyable = checkArgsCopyable<Args...>();
            if constexpr (copyable)
            {
                ArgsList args_list = {};
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
            sinkDefaultImmediate(ev, s.view());
        }
        onLog(ev);
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
            sinkDefaultImmediate(ev, skr::string_view(buffer));
        }
        onLog(ev);
    }

    skr::string_view get_name() const SKR_NOEXCEPT { return name.view(); }

private:
    void onLog(const LogEvent& ev) SKR_NOEXCEPT;
    void sinkDefaultImmediate(const LogEvent& event, skr::string_view formatted_message) const SKR_NOEXCEPT;
    bool canPushToQueue() const SKR_NOEXCEPT;
    bool tryPushToQueue(LogEvent ev, skr::string_view format, ArgsList&& args) SKR_NOEXCEPT;
    bool tryPushToQueue(LogEvent ev, skr::string&& what) SKR_NOEXCEPT;
    void notifyWorker() SKR_NOEXCEPT;
    skr::string name;
};

} } // namespace skr::log