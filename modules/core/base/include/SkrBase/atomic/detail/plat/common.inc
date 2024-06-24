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

typedef enum skr_memory_order {
  skr_memory_order_relaxed,
  skr_memory_order_consume,
  skr_memory_order_acquire,
  skr_memory_order_release,
  skr_memory_order_acq_rel,
  skr_memory_order_seq_cst
} skr_memory_order;

#ifdef _WIN32
    #define _SAtomic(tp)         volatile tp

    #ifndef SATOMIC_VAR_INIT
    #define SATOMIC_VAR_INIT(x)  x
    #endif
#endif

#ifdef __cplusplus
}
#endif