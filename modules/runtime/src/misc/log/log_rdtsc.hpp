#pragma once
#include "platform/configure.h"

#if defined(__aarch64__)
#elif defined(__ARM_ARCH)
    #include <chrono>
#elif (defined(_M_ARM) || defined(_M_ARM64))
#else
    // assume x86-64 ..
    #if defined(_WIN32)
        #include <intrin.h>
    #elif (defined(__GNUC__) && __GNUC__ > 10) || (defined(__clang_major__) && __clang_major__ > 11)
        #include <x86gprintrin.h>
    #else
        // older compiler versions do not have <x86gprintrin.h>
        #include <x86intrin.h>
    #endif
#endif

namespace skr::log
{
#if defined(__aarch64__)
// arm64
FORCEINLINE uint64_t rdtsc() SKR_NOEXCEPT
{
    // System timer of ARMv8 runs at a different frequency than the CPU's.
    // The frequency is fixed, typically in the range 1-50MHz.  It can be
    // read at CNTFRQ special register.  We assume the OS has set up the virtual timer properly.
    int64_t virtual_timer_value;
    __asm__ volatile("mrs %0, cntvct_el0"
                     : "=r"(virtual_timer_value));
    return static_cast<uint64_t>(virtual_timer_value);
}
#elif defined(__ARM_ARCH)
FORCEINLINE uint64_t rdtsc() SKR_NOEXCEPT
{
    #if (__ARM_ARCH >= 6)
    // V6 is the earliest arch that has a standard cyclecount
    uint32_t pmccntr;
    uint32_t pmuseren;
    uint32_t pmcntenset;

    __asm__ volatile("mrc p15, 0, %0, c9, c14, 0"
                     : "=r"(pmuseren));
    if (pmuseren & 1)
    {
        __asm__ volatile("mrc p15, 0, %0, c9, c12, 1"
                         : "=r"(pmcntenset));
        if (pmcntenset & 0x80000000ul)
        {
            __asm__ volatile("mrc p15, 0, %0, c9, c13, 0"
                             : "=r"(pmccntr));
            return (static_cast<uint64_t>(pmccntr)) * 64u;
        }
    }
    #endif
    // soft failover
    return static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());
}
#elif defined(_WIN32)
/**
 * Get the TSC counter
 * @return rdtsc timestamp
 */
FORCEINLINE uint64_t rdtsc() SKR_NOEXCEPT { return __rdtsc(); }
#elif defined(__cplusplus)
// __PPC64__, _M_ARM, _M_ARM64
FORCEINLINE uint64_t rdtsc() SKR_NOEXCEPT
{
    // soft failover
    return static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());
}
#endif

} // namespace skr::log
