#pragma once
#include "SkrBase/config.h"

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
SKR_CORE_API int64_t skr_sys_get_usec(bool precise);
SKR_CORE_API int64_t skr_sys_get_timer_freq(void);

// Time related functions
SKR_CORE_API uint32_t skr_sys_get_time(void);
SKR_CORE_API uint32_t skr_sys_get_time_since_start(void);

/// Low res OS timer
typedef struct STimer {
    uint32_t mStartTime;
} STimer;

SKR_CORE_API void skr_init_timer(STimer* pTimer);
SKR_CORE_API void skr_timer_reset(STimer* pTimer);
SKR_CORE_API uint32_t skr_timer_get_msec(STimer* pTimer, bool reset);
SKR_CORE_API double skr_timer_get_seconds(STimer* pTimer, bool reset);

/// High-resolution OS timer
#define HIRES_TIMER_LENGTH_OF_HISTORY 60

typedef struct SHiresTimer {
    int64_t mStartTime;
    int64_t mHistory[HIRES_TIMER_LENGTH_OF_HISTORY];
    uint32_t mHistoryIndex;
} SHiresTimer;

SKR_CORE_API void skr_init_hires_timer(SHiresTimer* pTimer);
SKR_CORE_API int64_t skr_hires_timer_get_usec(SHiresTimer* pTimer, bool reset);
SKR_CORE_API int64_t skr_hires_timer_get_usec_average(SHiresTimer* pTimer);
SKR_CORE_API double skr_hires_timer_get_seconds(SHiresTimer* pTimer, bool reset);
SKR_CORE_API double skr_hires_timer_get_secondsAverage(SHiresTimer* pTimer);
SKR_CORE_API void skr_hires_timer_reset(SHiresTimer* pTimer);

#ifdef __cplusplus
}
#endif