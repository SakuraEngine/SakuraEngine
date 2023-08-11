#pragma once
#include "SkrRT/misc/log.h"
#include "SkrRT/platform/time.h"
#include "SkrRT/platform/thread.h"

#include "SkrProfile/profile.h"

template<typename F>
bool wait_timeout(F f, uint32_t seconds_timeout = 3)
{
    SkrZoneScopedN("WaitTimeOut");
    uint64_t milliseconds = 0;
    const auto start = skr_sys_get_usec(true);
    auto current = start;
    while (!f())
    {
        if (milliseconds > seconds_timeout * 1000)
        {
            SKR_LOG_ERROR(u8"drain timeout, force quit");
            return false;
        }
        for (auto waited = 0; waited < 40; ++waited)
            skr_thread_sleep(0);
        current = skr_sys_get_usec(true);
        milliseconds = (current - start) / 1000;
    }
    return true;
}
