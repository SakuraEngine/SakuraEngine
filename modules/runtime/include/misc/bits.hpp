
#include <cstdint>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
namespace skr
{
inline int CountLeadingZeros64Slow(uint64_t n) {
    int zeroes = 60;
    if (n >> 32) zeroes -= 32, n >>= 32;
    if (n >> 16) zeroes -= 16, n >>= 16;
    if (n >> 8) zeroes -= 8, n >>= 8;
    if (n >> 4) zeroes -= 4, n >>= 4;
    return "\4\3\2\2\1\1\1\1\0\0\0\0\0\0\0"[n] + zeroes;
}

inline int CountTrailingZeros64Slow(uint64_t n) {
    int zeroes = 0;
    if ((n & 0xffffffff) == 0) zeroes += 32, n >>= 32;
    if ((n & 0xffff) == 0) zeroes += 16, n >>= 16;
    if ((n & 0xff) == 0) zeroes += 8, n >>= 8;
    if ((n & 0xf) == 0) zeroes += 4, n >>= 4;
    return "\0\1\0\2\0\1\0\3\0\1\0\2\0\1\0"[n] + zeroes;
}

inline int CountLeadingZeros64(uint64_t n)
{
#if defined(_MSC_VER) && defined(_M_X64)
    // MSVC does not have __buitin_clzll. Use _BitScanReverse64.
    unsigned long result = 0; // NOLINT(runtime/int)
    if (_BitScanReverse64(&result, n))
    {
        return (int)(63 - result);
    }
    return 64;
#elif defined(_MSC_VER) && !defined(__clang__)
    // MSVC does not have __buitin_clzll. Compose two calls to _BitScanReverse
    unsigned long result = 0; // NOLINT(runtime/int)
    if ((n >> 32) && _BitScanReverse(&result, (unsigned long)(n >> 32)))
    {
        return 31 - result;
    }
    if (_BitScanReverse(&result, (unsigned long)n))
    {
        return 63 - result;
    }
    return 64;
#elif defined(__GNUC__) || defined(__clang__)
    // Use __builtin_clzll, which uses the following instructions:
    //  x86: bsr
    //  ARM64: clz
    //  PPC: cntlzd
    static_assert(sizeof(unsigned long long) == sizeof(n), // NOLINT(runtime/int)
    "__builtin_clzll does not take 64-bit arg");

    // Handle 0 as a special case because __builtin_clzll(0) is undefined.
    if (n == 0)
    {
        return 64;
    }
    return __builtin_clzll(n);
#else
    return CountLeadingZeros64Slow(n);
#endif
}

inline int CountTrailingZeros64(uint64_t n)
{
#if defined(_MSC_VER) && defined(_M_X64)
    // MSVC does not have __buitin_ctzll. Use _BitScanForward64.
    unsigned long result = 0; // NOLINT(runtime/int)
    if (_BitScanForward64(&result, n))
    {
        return (int)result;
    }
    return 64;
#elif defined(_MSC_VER) && !defined(__clang__)
    // MSVC does not have __buitin_ctzll. Compose two calls to _BitScanForward
    unsigned long result = 0; // NOLINT(runtime/int)
    if ((n & 0xffffffff) == 0 && _BitScanForward(&result, (unsigned long)(n >> 32)))
    {
        return 32 + result;
    }
    if (_BitScanForward(&result, (unsigned long)n))
    {
        return result;
    }
    return 0;
#elif defined(__GNUC__) || defined(__clang__)
    // Use __builtin_ctzll, which uses the following instructions:
    //  x86: bsf
    //  ARM64: rbit, clz
    //  PPC: cntlzd
    static_assert(sizeof(unsigned long long) == sizeof(n), // NOLINT(runtime/int)
    "__builtin_ctzll does not take 64-bit arg");

    // Handle 0 as a special case because __builtin_ctzll(0) is undefined.
    if (n == 0)
    {
        return 0;
    }
    return __builtin_ctzll(n);
#else
    return CountTrailingZeros64Slow(n);
#endif
}
} // namespace dual