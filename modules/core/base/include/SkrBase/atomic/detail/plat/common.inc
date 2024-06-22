#pragma once
#include <stdint.h>
#include <intrin0.h>
#ifndef __cplusplus
#include <stdbool.h>
#else
#include <type_traits>
extern "C" {
#endif

inline static bool atomic_is_lock_free() { return true; }

typedef enum _plat_memory_order {
  memory_order_relaxed,
  memory_order_consume,
  memory_order_acquire,
  memory_order_release,
  memory_order_acq_rel,
  memory_order_seq_cst
} _plat_memory_order;

#ifdef _WIN32
    #define _Atomic(tp)         volatile tp

    #ifndef ATOMIC_VAR_INIT
    #define ATOMIC_VAR_INIT(x)  x
    #endif
#endif

#ifdef __cplusplus
}
#endif