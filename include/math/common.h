#pragma once
#include "platform/configure.h"

#ifdef __cplusplus
extern "C" {
#endif
inline uint32_t smath_round_up(uint32_t value, uint32_t multiple) { return ((value + multiple - 1) / multiple) * multiple; }
inline uint64_t smath_round_up_64(uint64_t value, uint64_t multiple) { return ((value + multiple - 1) / multiple) * multiple; }

inline uint32_t smath_round_down(uint32_t value, uint32_t multiple) { return value  - value % multiple; }
inline uint64_t smath_round_down_64(uint64_t value, uint64_t multiple) { return value - value % multiple; }

#ifdef __cplusplus
}
#endif