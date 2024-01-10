#pragma once
#include "SkrRT/misc/log.h"
#include "SkrRT/platform/time.h"
#include "SkrRT/platform/thread.h"

#include "SkrProfile/profile.h"

template <size_t N>
struct __zzzStringLiteral {
    constexpr __zzzStringLiteral(const char8_t (&str)[N]) { std::copy_n(str, N, __zzValue); }
    char8_t __zzValue[N];
};

template <__zzzStringLiteral what = u8"drain timeout, force quit">
struct wait_timeout {
    template <typename F>
    wait_timeout(F f, uint32_t seconds_timeout = 3)
    {
        SkrZoneScopedN("WaitTimeOut");
        uint64_t   milliseconds = 0;
        const auto start        = skr_sys_get_usec(true);
        auto       current      = start;
        while (!f())
        {
            if (milliseconds > seconds_timeout * 1000)
            {
                SKR_LOG_ERROR(what.__zzValue);
                ret = false;
                return;
            }
            for (auto waited = 0; waited < 40; ++waited)
                skr_thread_sleep(0);
            current      = skr_sys_get_usec(true);
            milliseconds = (current - start) / 1000;
        }
        ret = true;
    }
    operator bool() const { return ret; }
    bool ret = false;
};
