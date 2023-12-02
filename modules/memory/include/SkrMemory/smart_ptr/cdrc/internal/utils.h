#ifndef CDRC_INTERNAL_UTILS_H
#define CDRC_INTERNAL_UTILS_H

#include <cstdint>
#include <cstdlib>

#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include "SkrMemory/memory.h"

const int PADDING = 64;

// PARLAY_PREFETCH: Prefetch data into cache
#if defined(__GNUC__)
#define PARLAY_PREFETCH(addr, rw, locality) __builtin_prefetch ((addr), (rw), (locality))
#else
#define PARLAY_PREFETCH(addr, rw, locality)
#endif

namespace cdrc {

struct empty_guard {};

template<class F>
[[nodiscard]] auto scope_guard(F&& f) {
  return std::unique_ptr<void, std::decay_t<F>>{(void*)1, std::forward<F>(f)};
}

namespace internal {

// A vector that is stored at 128-byte-aligned memory (this
// means that the header of the vector, not the heap buffer,
// is aligned to 128 bytes)
template<typename Tp>
struct alignas(128) AlignedVector : public std::vector<Tp> { };

// A cache-line-aligned int to prevent false sharing
struct alignas(128) AlignedInt {
  AlignedInt() : x(0) {}
  /* implicit */ AlignedInt(unsigned int x_) : x(x_) {}
  /* implicit */ operator unsigned int() const { return x; }
 private:
  unsigned int x;
};

// A cache-line-aligned int to prevent false sharing
struct alignas(128) AlignedLong {
  AlignedLong() : x(0) {}
  /* implicit */ AlignedLong(uint64_t x_) : x(x_) {}
  /* implicit */ operator uint64_t() const { return x; }
private:
    uint64_t x;
};

// A cache-line-aligned bool to prevent false sharing
struct alignas(128) AlignedBool {
  AlignedBool() : b(false) {}
  /* implicit */ AlignedBool(bool b_) : b(b_) {}
  /* implicit */ operator bool() const { return b; }
 private:
  bool b;
};

}  // namespace internal

namespace utils {

static size_t num_threads() {
  static size_t n_threads = []() -> size_t {
    if (const auto env_p = std::getenv("NUM_THREADS")) {
      return std::stoi(env_p) + 1;
    } else {
      return std::thread::hardware_concurrency() + 1;
    }
  }();
  return n_threads;
}

template<typename T, typename Sfinae = void>
struct Padded;

// Use user-defined conversions to pad primitive types
template<typename T>
struct alignas(128) Padded<T, typename std::enable_if<std::is_fundamental<T>::value>::type> {
  Padded() = default;
  /* implicit */ Padded(T _x) : x(_x) { }
  /* implicit */ operator T() { return x; }
  T x;
};

// Use inheritance to pad class types
template<typename T>
struct alignas(128) Padded<T, typename std::enable_if<std::is_class<T>::value>::type> : public T { };

// A wait-free atomic counter that supports increment and decrement,
// such that attempting to increment the counter from zero fails and
// does not perform the increment.
//
// Useful for implementing reference counting, where the underlying
// managed memory is freed when the counter hits zero, so that other
// racing threads can not increment the counter back up from zero
//
// Assumption: The counter should never go negative. That is, the
// user should never decrement the counter by an amount greater
// than its current value
//
// Note: The counter steals the top two bits of the integer for book-
// keeping purposes. Hence the maximum representable value in the
// counter is 2^(8*sizeof(T)-2) - 1
template<typename T>
class StickyCounter {
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);

public:

  [[nodiscard]] bool is_lock_free() const { return true; }
  static constexpr bool is_always_lock_free = true;
  [[nodiscard]] constexpr T max_value() const { return zero_pending_flag - 1; }

  StickyCounter() noexcept : x(1) {}
  explicit StickyCounter(T desired) noexcept : x(desired == 0 ? zero_flag : desired) {}

  // Increment the counter by the given amount if the counter is not zero.
  //
  // Returns true if the increment was successful, i.e., the counter
  // was not stuck at zero. Returns false if the counter was zero
  bool increment(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept {
    //if (x.load() & zero_flag) return false;
    auto val = x.fetch_add(arg, order);
    return (val & zero_flag) == 0;
  }

  // Decrement the counter by the given amount. The counter must initially be
  // at least this amount, i.e., it is not permitted to decrement the counter
  // to a negative number.
  //
  // Returns true if the counter was decremented to zero. Returns
  // false if the counter was not decremented to zero
  bool decrement(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept {
    if (x.fetch_sub(arg, order) == arg) {
      T expected = 0;
      if (x.compare_exchange_strong(expected, zero_flag)) [[likely]] return true;
      else if ((expected & zero_pending_flag) && (x.exchange(zero_flag) & zero_pending_flag)) return true;
    }
    return false;
  }

  // Loads the current value of the counter. If the current value is zero, it is guaranteed
  // to remain zero until the counter is reset
  T load(std::memory_order order = std::memory_order_seq_cst) const noexcept {
    auto val = x.load(order);
    if (val == 0 && x.compare_exchange_strong(val, zero_flag | zero_pending_flag)) [[unlikely]] return 0;
    return (val & zero_flag) ? 0 : val;
  }

  // Resets the value of the counter to the given value. This may be called when the counter
  // is zero to bring it back to a non-zero value.
  //
  // It is not permitted to race with an increment or decrement.
  void reset(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
    x.store(desired == 0 ? zero_flag : desired, order);
  }

private:
  static constexpr inline T zero_flag = (T(1) << (sizeof(T)*8 - 1));
  static constexpr inline T zero_pending_flag = (T(1) << (sizeof(T)*8 - 2));

  mutable std::atomic<T> x;
};


struct SKR_MEMORY_API ThreadID {
  static std::vector<std::atomic<bool>> in_use;
  static ThreadID GetThreadID();
  int tid;

  ThreadID() {
    for(size_t i = 0; i < num_threads(); i++) {
      bool expected = false;
      if(!in_use[i] && in_use[i].compare_exchange_strong(expected, true)) {
        tid = i;
        return;
      }
    }
    std::cerr << "Error: more than " << num_threads() << " threads created" << std::endl;
    std::exit(1);
  }

  ~ThreadID() {
    in_use[tid] = false;
  }

  int getTID() const { return tid; }
};


// a slightly cheaper, but possibly not as good version
// based on splitmix64
inline uint64_t hash64_2(uint64_t x) {
  x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
  x = x ^ (x >> 31);
  return x;
}

template <typename T>
struct CustomHash;

template<>
struct CustomHash<uint64_t> {
  size_t operator()(uint64_t a) const {
    return hash64_2(a);
  }
};

template<class T>
struct CustomHash<T*> {
  size_t operator()(T* a) const {
    return hash64_2((uint64_t) a);
  }
};

namespace rand {
  thread_local static unsigned long x=123456789, y=362436069, z=521288629;

  inline static void init(int seed) {
    x += seed;
  }

  inline static unsigned long get_rand() {          //period 2^96-1
    unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
  }
}

}  // namespace utils

}  // namespace cdrc

#endif  // CDRC_INTERNAL_UTILS_H
