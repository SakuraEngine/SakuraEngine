#include "platform/time.h"

void skr_init_timer(STimer* pTimer) { skr_timer_reset(pTimer); }

unsigned skr_timer_get_msec(STimer* pTimer, bool reset)
{
    unsigned currentTime = getSystemTime();
    unsigned elapsedTime = currentTime - pTimer->mStartTime;
    if (reset)
        pTimer->mStartTime = currentTime;

    return elapsedTime;
}

void skr_timer_reset(STimer* pTimer) { pTimer->mStartTime = getSystemTime(); }

void skr_init_hires_timer(SHiresTimer* pTimer)
{
    *pTimer = (SHiresTimer){ 0 };
    skr_hires_timer_reset(pTimer);
}

int64_t skr_hires_timer_get_usec(SHiresTimer* pTimer, bool reset)
{
    int64_t currentTime = getUSec(false);
    int64_t elapsedTime = currentTime - pTimer->mStartTime;

    // Correct for possible weirdness with changing internal frequency
    if (elapsedTime < 0)
        elapsedTime = 0;

    if (reset)
        pTimer->mStartTime = currentTime;

    pTimer->mHistory[pTimer->mHistoryIndex] = elapsedTime;
    pTimer->mHistoryIndex = (pTimer->mHistoryIndex + 1) % HIRES_TIMER_LENGTH_OF_HISTORY;

    return elapsedTime;
}

int64_t skr_hires_timer_get_usec_average(SHiresTimer* pTimer)
{
    int64_t elapsedTime = 0;
    for (uint32_t i = 0; i < HIRES_TIMER_LENGTH_OF_HISTORY; ++i)
        elapsedTime += pTimer->mHistory[i];
    elapsedTime /= HIRES_TIMER_LENGTH_OF_HISTORY;

    // Correct for overflow
    if (elapsedTime < 0)
        elapsedTime = 0;

    return elapsedTime;
}

float skr_hires_timer_get_seconds(SHiresTimer* pTimer, bool reset) { return (float)(skr_hires_timer_get_usec(pTimer, reset) / 1e6); }

float skr_hires_timer_get_secondsAverage(SHiresTimer* pTimer) { return (float)(skr_hires_timer_get_usec_average(pTimer) / 1e6); }

void skr_hires_timer_reset(SHiresTimer* pTimer) { pTimer->mStartTime = getUSec(false); }
