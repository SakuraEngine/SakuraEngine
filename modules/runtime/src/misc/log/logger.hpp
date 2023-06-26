#pragma once
#include "log_queue.hpp"
#include "log_worker.hpp"

namespace skr {
namespace log {

template<typename T>
struct IsCopyableArgument
{
    static constexpr bool value = std::is_copy_constructible_v<T>;
};

struct Logger
{
    Logger() SKR_NOEXCEPT;
    ~Logger() SKR_NOEXCEPT;

    template <typename... Args>
    constexpr bool checkCopyable(Args&&... arg) const SKR_NOEXCEPT
    {
        return (IsCopyableArgument<Args>::value && ...);
    }

    void notifyWorker() SKR_NOEXCEPT;

    template <typename...Args>
    void log(LogLevel level, skr::string_view format, Args&&... args) SKR_NOEXCEPT
    {
        LogEvent ev = LogEvent(level);

        constexpr bool copyable = checkCopyable(args...);
        if constexpr (copyable)
        {
            queue_.push(ev, format, skr::forward<Args>(args)...);
            skr_atomic64_add_relaxed(&cnt_, 1);
        }
        else // foramt inplace, expensive
        {
            skr::string s = skr::format(format, skr::forward<Args>(args)...);
            queue_.push(ev, skr::move(s));
            skr_atomic64_add_relaxed(&cnt_, 1);
        }
        notifyWorker();
    }

    void log(LogLevel level, skr::string_view format, va_list va_args) SKR_NOEXCEPT
    {
        LogEvent ev = LogEvent(level);

        skr::string fmt(format);
        char8_t buffer[1024];
        vsnprintf((char* const)buffer, sizeof(buffer), fmt.c_str(), va_args);
        queue_.push(ev, skr::string(buffer));
        skr_atomic64_add_relaxed(&cnt_, 1);
        notifyWorker();
    }

    int64_t query_cnt() const SKR_NOEXCEPT
    {
        return skr_atomic64_load_relaxed(&cnt_);
    }

    bool try_dequeue(LogQueueElement& element) SKR_NOEXCEPT
    {
        if (queue_.try_dequeue(element))
        {
            skr_atomic64_add_relaxed(&cnt_, -1);
            return true;
        }
        return false;
    }

private:
    LogQueue queue_;
    SAtomic64 cnt_ = 0;
};

} } // namespace skr::log