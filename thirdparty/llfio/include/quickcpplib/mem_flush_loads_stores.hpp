/* Ensuring no load nor dead store elimination
(C) 2018 - 2021 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: April 2018


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
    (See accompanying file Licence.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef QUICKCPPLIB_MEM_FLUSH_LOADS_STORES_HPP
#define QUICKCPPLIB_MEM_FLUSH_LOADS_STORES_HPP

#include "byte.hpp"

#include <atomic>
#include <cstddef>  // for size_t

#ifdef _MSC_VER
#include <intrin.h>
#endif

QUICKCPPLIB_NAMESPACE_BEGIN

namespace mem_flush_loads_stores
{
  using byte::byte;

  /*! \brief The kinds of cache line flushing which can be performed. */
  enum class memory_flush
  {
    memory_flush_none,    //!< No memory flushing.
    memory_flush_retain,  //!< Flush modified cache line to memory, but retain as unmodified in cache.
    memory_flush_evict    //!< Flush modified cache line to memory, and evict completely from all caches.
  };
  //! No memory flushing.
  constexpr memory_flush memory_flush_none = memory_flush::memory_flush_none;
  //! Flush modified cache line to memory, but retain as unmodified in cache.
  constexpr memory_flush memory_flush_retain = memory_flush::memory_flush_retain;
  //! Flush modified cache line to memory, and evict completely from all caches.
  constexpr memory_flush memory_flush_evict = memory_flush::memory_flush_evict;

  namespace detail
  {
    using flush_impl_type = memory_flush (*)(const void *addr, size_t bytes, memory_flush kind);
    inline QUICKCPPLIB_NOINLINE flush_impl_type make_flush_impl() noexcept
    {
#if defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
#if !defined(_MSC_VER) || (defined(_MSC_VER) && defined(__clang__) && !defined(__c2__))
      static const auto __cpuidex = [](int *cpuInfo, int func1, int func2) {
        __asm__ __volatile__("cpuid\n\t" : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3]) : "a"(func1), "c"(func2));
      };  // NOLINT
      // static constexpr auto _mm_clwb = [](const void *addr) { __asm__ __volatile__("clwb (%0)\n\t" : : "r"(addr)); }; // NOLINT
      static const auto _mm_clwb = [](const void *addr) { __asm__ __volatile__(".byte 0x66, 0x0f, 0xae, 0x30\n\t" : : "a"(addr)); };  // NOLINT
      static const auto _mm_clflushopt = [](const void *addr) { __asm__ __volatile__("clflushopt (%0)\n\t" : : "r"(addr)); };         // NOLINT
      static const auto _mm_clflush = [](const void *addr) { __asm__ __volatile__("clflush (%0)\n\t" : : "r"(addr)); };               // NOLINT
      static const auto _mm_sfence = []() { __asm__ __volatile__("sfence\n\t"); };                                                    // NOLINT
#endif
      int nBuff[4];
      __cpuidex(nBuff, 0x7, 0x0);
      if(nBuff[1] & (1 << 24))  // has CLWB instruction
      {
        return [](const void *addr, size_t bytes, memory_flush kind) -> memory_flush {
          if(kind == memory_flush_retain)
          {
            while(bytes > 0)
            {
              _mm_clwb(addr);
              addr = (void *) ((uintptr_t) addr + 64);
              bytes -= 64;
            }
            _mm_sfence();
            return memory_flush_retain;
          }
          while(bytes > 0)
          {
            _mm_clflushopt(addr);
            addr = (void *) ((uintptr_t) addr + 64);
            bytes -= 64;
          }
          _mm_sfence();
          return memory_flush_evict;
        };
      }
      if(nBuff[1] & (1 << 23))  // has CLFLUSHOPT instruction
      {
        return [](const void *addr, size_t bytes, memory_flush /*unused*/) -> memory_flush {
          while(bytes > 0)
          {
            _mm_clflushopt(addr);
            addr = (void *) ((uintptr_t) addr + 64);
            bytes -= 64;
          }
          _mm_sfence();
          return memory_flush_evict;
        };
      }
      else
      {
        // Use CLFLUSH instruction
        return [](const void *addr, size_t bytes, memory_flush /*unused*/) -> memory_flush {
          while(bytes > 0)
          {
            _mm_clflush(addr);
            addr = (void *) ((uintptr_t) addr + 64);
            bytes -= 64;
          }
          return memory_flush_evict;
        };
      }
#elif defined(__aarch64__) || defined(_M_ARM64)
#if !defined(_MSC_VER) || (defined(_MSC_VER) && defined(__clang__) && !defined(__c2__))
      static const auto _dmb_ish = []() { __asm__ __volatile__("dmb ish" : : : "memory"); };
      static const auto _dc_cvac = [](const void *addr) { __asm__ __volatile__("dc cvac, %0" : : "r"(addr) : "memory"); };
      static const auto _dc_civac = [](const void *addr) { __asm__ __volatile__("dc civac, %0" : : "r"(addr) : "memory"); };
#else
      static const auto _dmb_ish = []() { __dmb(_ARM64_BARRIER_ISH); };
      static const auto _dc_cvac = [](const void *addr) {
        (void) addr;
        abort();  // currently MSVC doesn't have an intrinsic for this, could use __emit()?
      };
      static const auto _dc_civac = [](const void *addr) {
        (void) addr;
        abort();  // currently MSVC doesn't have an intrinsic for this, could use __emit()?
      };
#endif
      return [](const void *addr, size_t bytes, memory_flush kind) -> memory_flush {
        if(kind == memory_flush_retain)
        {
          while(bytes > 0)
          {
            _dc_cvac(addr);
            addr = (void *) ((uintptr_t) addr + 64);
            bytes -= 64;
          }
          _dmb_ish();
          return memory_flush_retain;
        }
        while(bytes > 0)
        {
          _dc_civac(addr);
          addr = (void *) ((uintptr_t) addr + 64);
          bytes -= 64;
        }
        _dmb_ish();
        return memory_flush_evict;
      };
#elif defined(__arm__) || defined(_M_ARM)
#if !defined(_MSC_VER) || (defined(_MSC_VER) && defined(__clang__) && !defined(__c2__))
#undef _MoveToCoprocessor
#define _MoveToCoprocessor(value, coproc, opcode1, crn, crm, opcode2)                                                                                          \
  __asm__ __volatile__("MCR p" #coproc ", " #opcode1 ", %0, c" #crn ", c" #crm ", " #opcode2 : : "r"(value) : "memory");  // NOLINT
#endif
      return [](const void *addr, size_t bytes, memory_flush kind) -> memory_flush {
        if(kind == memory_flush_retain)
        {
          while(bytes > 0)
          {
            // __asm__ __volatile__("MCR p15, 0, %0, c7, c10, 1" : : "r"(addr) : "memory");
            _MoveToCoprocessor(addr, 15, 0, 7, 10, 1);
            addr = (void *) ((uintptr_t) addr + 64);
            bytes -= 64;
          }
          // __asm__ __volatile__("mcr p15, 0, %0, c7, c10, 5" : : "r"(0) : "memory");
          _MoveToCoprocessor(0, 15, 0, 7, 10, 5);
          return memory_flush_retain;
        }
        while(bytes > 0)
        {
          // __asm__ __volatile__("MCR p15, 0, %0, c7, c14, 1" : : "r"(addr) : "memory");
          _MoveToCoprocessor(addr, 15, 0, 7, 14, 1);
          addr = (void *) ((uintptr_t) addr + 64);
          bytes -= 64;
        }
        // __asm__ __volatile__("mcr p15, 0, %0, c7, c10, 5" : : "r"(0) : "memory");
        _MoveToCoprocessor(0, 15, 0, 7, 10, 5);
        return memory_flush_evict;
      };
#else
#error Unsupported platform
#endif
    }
    inline flush_impl_type flush_impl() noexcept
    {
      static flush_impl_type f;
      if(f != nullptr)
        return f;
      f = make_flush_impl();
      return f;
    }

    using potentially_unknown_jump_ptr = void (*)(const byte *data, size_t bytes);
    extern inline potentially_unknown_jump_ptr potentially_unknown_jump(potentially_unknown_jump_ptr set = nullptr)
    {
      static potentially_unknown_jump_ptr f = +[](const byte * /*unused*/, size_t /*unused*/) -> void {};
      if(set != nullptr)
      {
        f = set;
      }
      return f;
    }
  }  // namespace detail

  /*! \brief Ensures that reload elimination does not happen for a region of
  memory, optionally synchronising the region with main memory.
  \addtogroup P1631
  \return The kind of memory flush actually used
  \param data The beginning of the byte array to ensure loads from.
  \param bytes The number of bytes to ensure loads from.
  \param kind Whether to ensure loads from the region are from main memory.
  Defaults to not doing so.
  \param order The atomic reordering constraints to apply to this operation. Defaults
  to atomic acquire constraints, which prevents reads and writes to this region
  subsequent to this operation being reordered to before this operation.

  \note `memory_flush_retain` has no effect for loads, it is the same as doing
  nothing. Only `memory_flush_evict` evicts all the cache lines for the region
  of memory, thus ensuring that subsequent loads are from main memory.
  */
  inline memory_flush mem_force_reload(const byte *data, size_t bytes, memory_flush kind = memory_flush_none,
                                       std::memory_order order = std::memory_order_acquire) noexcept
  {
    memory_flush ret = kind;
    // Ensure reload elimination does not occur on our region by calling a
    // potentially unknown external function which forces the compiler to
    // reload state after this call returns
    detail::potentially_unknown_jump()(data, bytes);
    if(memory_flush_evict == kind)
    {
      // TODO FIXME We assume a 64 byte cache line, which is bold.
      void *_data = (void *) (((uintptr_t) data) & ~63);
      size_t _bytes = ((uintptr_t) data + bytes + 63) - ((uintptr_t) _data);
      _bytes &= ~63;
      ret = detail::flush_impl()(_data, _bytes, kind);
    }
    // I really wish this would work on a region, not globally
    atomic_thread_fence(order);
    return ret;
  }

  /*! \brief Sized C byte array overload for `mem_force_reload()`.
  \addtogroup P1631
  */
  template <size_t N>
  inline memory_flush mem_force_reload(const byte (&region)[N], memory_flush kind = memory_flush_none,
                                       std::memory_order order = std::memory_order_acquire) noexcept
  {
    return mem_force_reload(region, N, kind, order);
  }

  /*! \brief Ensures that dead store elimination does not happen for a region of
  memory, optionally synchronising the region with main memory.
  \addtogroup P1631
  \return The kind of memory flush actually used
  \param data The beginning of the byte array to ensure stores to.
  \param bytes The number of bytes to ensure stores to.
  \param kind Whether to wait until all stores to the region reach main memory.
  Defaults to not waiting.
  \param order The atomic reordering constraints to apply to this operation. Defaults
  to atomic release constraints, which prevents reads and writes to this region
  preceding this operation being reordered to after this operation.

  \warning On older Intel CPUs, due to lack of hardware support, we always execute
  `memory_flush_evict` even if asked for `memory_flush_retain`. This can produce
  some very poor performance. Check the value returned to see what kind of flush
  was actually performed.
  */
  inline memory_flush mem_flush_stores(const byte *data, size_t bytes, memory_flush kind = memory_flush_none,
                                       std::memory_order order = std::memory_order_release) noexcept
  {
    // I really wish this would work on a region, not globally
    atomic_thread_fence(order);
    // Ensure dead store elimination does not occur on our region by calling a
    // potentially unknown external function which forces the compiler to dump
    // out all pending writes before this call
    detail::potentially_unknown_jump()(data, bytes);
    if(memory_flush_none != kind)
    {
      // TODO FIXME We assume a 64 byte cache line, which is bold.
      void *_data = (void *) (((uintptr_t) data) & ~63);
      size_t _bytes = ((uintptr_t) data + bytes + 63) - ((uintptr_t) _data);
      _bytes &= ~63;
      memory_flush ret = detail::flush_impl()(_data, _bytes, kind);
      return ret;
    }
    return kind;
  }

  /*! \brief Sized C byte array overload for `mem_flush_stores()`.
  \addtogroup P1631
  */
  template <size_t N>
  inline memory_flush mem_flush_stores(const byte (&region)[N], memory_flush kind = memory_flush_none,
                                       std::memory_order order = std::memory_order_release) noexcept
  {
    return mem_flush_stores(region, N, kind, order);
  }

}  // namespace mem_flush_loads_stores

QUICKCPPLIB_NAMESPACE_END

#endif
