#pragma once
#include "misc/log.h"

template<typename F>
bool wait_timeout(F f, uint32_t seconds_timeout = 3)
{
    ZoneScopedN("WaitTimeOut");
    uint32_t milliseconds = 0;
    while (!f())
    {
        if (milliseconds > seconds_timeout * 1000)
        {
            SKR_LOG_ERROR("drain timeout, force quit");
            return false;
        }
        skr_thread_sleep(1);
        milliseconds++;
    }
    return true;
}
