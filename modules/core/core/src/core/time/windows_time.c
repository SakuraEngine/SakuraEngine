#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> 
#include <timeapi.h>

#include "SkrBase/misc/debug.h" 
#include "SkrOS/thread.h"
#include "SkrCore/time.h"

#include <time.h>
#include <stdint.h>

#pragma comment(lib, "winmm.lib")

/************************************************************************/
// Time Related Functions
/************************************************************************/
uint32_t skr_sys_get_time() { return (uint32_t)timeGetTime(); }

uint32_t skr_sys_get_time_since_start() { return (uint32_t)time(NULL); }

static SCallOnceGuard timeInitGuard = INIT_ONCE_STATIC_INIT;
static int64_t highResTimerFrequency = 0;
static int64_t highResTimerStart = 0;

static bool alwaysSimpleMulDiv = false;
static int64_t timerToUSecMul = 0;
static int64_t timerToUSecDiv = 0;

static int64_t timeGCD(int64_t a, int64_t b)
{
    return (a == 0) ? b : timeGCD(b % a, a);
}

static void initTime(void)
{
    LARGE_INTEGER frequency;
    BOOL qpcResult = QueryPerformanceFrequency(&frequency);
    SKR_ASSERT(qpcResult);
    if (qpcResult)
    {
        highResTimerFrequency = frequency.QuadPart;
    }
    else
    {
        highResTimerFrequency = 1000LL;
    }

    LARGE_INTEGER counter;
    qpcResult = QueryPerformanceCounter(&counter);
    SKR_ASSERT(qpcResult);
    if (qpcResult)
    {
        highResTimerStart = counter.QuadPart;
    }
    else
    {
        highResTimerStart = 0;
    }

    timerToUSecMul = (int64_t)1e6; // 1 second = 1,000,000 microseconds
    timerToUSecDiv = highResTimerFrequency;
    const int64_t divisor = timeGCD(timerToUSecMul, timerToUSecDiv);
    timerToUSecMul /= divisor;
    timerToUSecDiv /= divisor;

    // If the multiplier is 1, there's no way our "simple" formula will overflow.
    // If the divisor is 1, then we still might overflow, but int64MulDiv wouldn't prevent it.
    alwaysSimpleMulDiv = (timerToUSecMul == 1) || (timerToUSecDiv == 1);
}

static void ensureTimeInit()
{
    // Make sure time constants are initialized before anyone tries to use them
    skr_call_once(&timeInitGuard, initTime);
}

int64_t skr_sys_get_timer_freq()
{
    ensureTimeInit();

    return highResTimerFrequency;
}

// The `precise` param is being used to specify the way in which the usec is calculated.
// If it's false, then a normal unsafe multiplication and division operations are made.
// If it's true, a special int64MulDiv function is called that avoids the overflow.
int64_t skr_sys_get_usec(bool precise)
{
    ensureTimeInit();

    LARGE_INTEGER counter;
    BOOL qpcResult = QueryPerformanceCounter(&counter);
    SKR_ASSERT(qpcResult);
    counter.QuadPart -= highResTimerStart;

    if (alwaysSimpleMulDiv || !precise)
    {
        return counter.QuadPart * timerToUSecMul / timerToUSecDiv;
    }
    else
    {
        return int64MulDiv(counter.QuadPart, timerToUSecMul, timerToUSecDiv);
    }
}
