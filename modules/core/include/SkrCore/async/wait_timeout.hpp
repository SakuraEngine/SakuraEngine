#pragma once
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrCore/time.h"

#include "SkrProfile/profile.h"

template <size_t N>
struct __zzzStringLiteral {
    constexpr __zzzStringLiteral(const char8_t (&str)[N]) { std::copy_n(str, N, __zzValue); }
    constexpr const char* c_str() const { return __zzValue; }
    const char8_t* u8_str() const { return (const char8_t*)__zzValue; }
    char __zzValue[N];
};

template <__zzzStringLiteral what = u8"unnamed">
struct wait_timeout {
    template <typename F>
    wait_timeout(F f, uint32_t seconds_timeout = 3)
    {
        SkrZoneScopedN(what.c_str());
        uint64_t   milliseconds = 0;
        const auto start        = skr_sys_get_usec(true);
        auto       current      = start;
        while (!f())
        {
            if (milliseconds > seconds_timeout * 1000)
            {
                SKR_LOG_ERROR(u8"wait_timeout: %s timeout, force quit!", what.u8_str());
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
