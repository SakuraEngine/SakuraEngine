#pragma once
// #include "../Core/Config.h"
#include "platform/configure.h"

#ifdef __cplusplus
extern "C" {
#endif

/* prevent 64-bit overflow when computing relative timestamp
    see https://gist.github.com/jspohr/3dc4f00033d79ec5bdaf67bc46c813e3
*/
/* TODO: Move this function to the Math Library as soon as it's in C */
static inline int64_t int64MulDiv(int64_t value, int64_t numer, int64_t denom)
{
    int64_t q = value / denom;
    int64_t r = value % denom;
    return q * numer + r * numer / denom;
}

// High res timer functions
RUNTIME_API int64_t getUSec(bool precise);
RUNTIME_API int64_t getTimerFrequency(void);

// Time related functions
RUNTIME_API uint32_t getSystemTime(void);
RUNTIME_API uint32_t getTimeSinceStart(void);

/// Low res OS timer
typedef struct STimer {
    uint32_t mStartTime;
} STimer;

RUNTIME_API void skr_init_timer(STimer* pTimer);
RUNTIME_API void skr_timer_reset(STimer* pTimer);
RUNTIME_API uint32_t skr_timer_get_msec(STimer* pTimer, bool reset);
RUNTIME_API double skr_timer_get_seconds(STimer* pTimer, bool reset);

/// High-resolution OS timer
#define HIRES_TIMER_LENGTH_OF_HISTORY 60

typedef struct SHiresTimer {
    int64_t mStartTime;
    int64_t mHistory[HIRES_TIMER_LENGTH_OF_HISTORY];
    uint32_t mHistoryIndex;
} SHiresTimer;

RUNTIME_API void skr_init_hires_timer(SHiresTimer* pTimer);
RUNTIME_API int64_t skr_hires_timer_get_usec(SHiresTimer* pTimer, bool reset);
RUNTIME_API int64_t skr_hires_timer_get_usec_average(SHiresTimer* pTimer);
RUNTIME_API double skr_hires_timer_get_seconds(SHiresTimer* pTimer, bool reset);
RUNTIME_API double skr_hires_timer_get_secondsAverage(SHiresTimer* pTimer);
RUNTIME_API void skr_hires_timer_reset(SHiresTimer* pTimer);

#ifdef __cplusplus
}
#endif