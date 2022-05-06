/* Provides yet another spinlock
(C) 2013-2017 Niall Douglas <http://www.nedproductions.biz/> (14 commits)
File Created: Sept 2013


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

/*! \file spinlock.hpp
\brief Provides a Lockable policy driven spinlock
*/

#ifndef QUICKCPPLIB_SPINLOCK_H
#define QUICKCPPLIB_SPINLOCK_H

#include "config.hpp"

#include <atomic>
#include <chrono>
#include <thread>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace configurable_spinlock
{
  template <class T> using atomic = std::atomic<T>;
  namespace chrono = std::chrono;
  namespace this_thread = std::this_thread;
  using std::memory_order;
  using std::memory_order_relaxed;
  using std::memory_order_consume;
  using std::memory_order_acquire;
  using std::memory_order_release;
  using std::memory_order_acq_rel;
  using std::memory_order_seq_cst;

  template <class T> class lock_guard
  {
    T *_m;

  public:
    using mutex_type = T;
    explicit lock_guard(mutex_type &m) noexcept : _m(&m) { _m->lock(); }
    explicit lock_guard(mutex_type &&m) noexcept : _m(&m) { _m->lock(); }
    lock_guard(const lock_guard &) = delete;
    lock_guard(lock_guard &&o) noexcept : _m(std::move(o._m)) { o._m = nullptr; }
    ~lock_guard()
    {
      if(_m)
        _m->unlock();
    }
  };

  namespace detail
  {
    template <class T> struct choose_half_type
    {
      static_assert(!std::is_same<T, T>::value, "detail::choose_half_type<T> not specialised for type T");
    };
    template <> struct choose_half_type<uint64_t>
    {
      using type = uint32_t;
    };
    template <> struct choose_half_type<uint32_t>
    {
      using type = uint16_t;
    };
    template <> struct choose_half_type<uint16_t>
    {
      using type = uint8_t;
    };
  }

  /*! \tparam T The type of the pointer
   * \brief Lets you use a pointer to memory as a spinlock :)
   */
  template <typename T> struct lockable_ptr : atomic<T *>
  {
    constexpr lockable_ptr(T *v = nullptr)
        : atomic<T *>(v)
    {
    }
    //! Returns the memory pointer part of the atomic
    T *get() noexcept
    {
      union {
        T *v;
        size_t n;
      } value;
      value.v = atomic<T *>::load(memory_order_relaxed);
      value.n &= ~(size_t) 1;
      return value.v;
    }
    //! Returns the memory pointer part of the atomic
    const T *get() const noexcept
    {
      union {
        T *v;
        size_t n;
      } value;
      value.v = atomic<T *>::load(memory_order_relaxed);
      value.n &= ~(size_t) 1;
      return value.v;
    }
    T &operator*() noexcept { return *get(); }
    const T &operator*() const noexcept { return *get(); }
    T *operator->() noexcept { return get(); }
    const T *operator->() const noexcept { return get(); }
  };
  template <typename T> struct spinlockbase
  {
  protected:
    atomic<T> v;

  public:
    typedef T value_type;

#ifndef QUICKCPPLIB_ENABLE_VALGRIND
    constexpr
#endif
    spinlockbase() noexcept : v(0)
    {
      QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(this);
#if QUICKCPPLIB_IN_THREAD_SANITIZER
      v.store(0, memory_order_release);
#endif
    }
    spinlockbase(const spinlockbase &) = delete;
//! Atomically move constructs
#ifndef QUICKCPPLIB_ENABLE_VALGRIND
    constexpr
#endif
    spinlockbase(spinlockbase &&) noexcept : v(0)
    {
      QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(this);
// v.store(o.v.exchange(0, memory_order_acq_rel));
#if QUICKCPPLIB_IN_THREAD_SANITIZER
      v.store(0, memory_order_release);
#endif
    }
    ~spinlockbase()
    {
#ifdef QUICKCPPLIB_ENABLE_VALGRIND
      if(v.load(memory_order_acquire))
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, true);
      }
#endif
      QUICKCPPLIB_ANNOTATE_RWLOCK_DESTROY(this);
    }
    spinlockbase &operator=(const spinlockbase &) = delete;
    spinlockbase &operator=(spinlockbase &&) = delete;
    //! Returns the raw atomic
    constexpr T load(memory_order o = memory_order_seq_cst) const noexcept { return v.load(o); }
    //! Sets the raw atomic
    void store(T a, memory_order o = memory_order_seq_cst) noexcept { v.store(a, o); }
    //! If atomic is zero, sets to 1 and returns true, else false.
    bool try_lock() noexcept
    {
#if !QUICKCPPLIB_IN_THREAD_SANITIZER  // no early outs for the sanitizer
#ifdef QUICKCPPLIB_USE_VOLATILE_READ_FOR_AVOIDING_CMPXCHG
      // MSVC's atomics always seq_cst, so use volatile read to create a true acquire
      volatile T *_v = (volatile T *) &v;
      if(*_v)  // Avoid unnecessary cache line invalidation traffic
        return false;
#else
      if(v.load(memory_order_relaxed))  // Avoid unnecessary cache line invalidation traffic
        return false;
#endif
#endif
#if 0 /* Disabled as CMPXCHG seems to have sped up on recent Intel */  // defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
      // Intel is a lot quicker if you use XCHG instead of CMPXCHG. ARM is definitely not!
      T ret = v.exchange(1, memory_order_acquire);
      if(!ret)
#else
      T expected = 0;
      bool ret = v.compare_exchange_weak(expected, 1, memory_order_acquire, memory_order_relaxed);
      if(ret)
#endif
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(this, true);
        return true;
      }
      else return false;
    }
    constexpr bool try_lock() const noexcept
    {
      return v.load(memory_order_consume) ? false : true;  // Avoid unnecessary cache line invalidation traffic
    }
    //! If atomic equals expected, sets to 1 and returns true, else false with expected updated to actual value.
    bool try_lock(T &expected) noexcept
    {
      T t(0);
#if !QUICKCPPLIB_IN_THREAD_SANITIZER  // no early outs for the sanitizer
#ifdef QUICKCPPLIB_USE_VOLATILE_READ_FOR_AVOIDING_CMPXCHG
      // MSVC's atomics always seq_cst, so use volatile read to create a true acquire
      volatile T *_v = (volatile T *) &v;
      if((t = *_v))  // Avoid unnecessary cache line invalidation traffic
#else
      t = v.load(memory_order_relaxed);
      if(t)  // Avoid unnecessary cache line invalidation traffic
#endif
      {
        expected = t;
        return false;
      }
#endif
      bool ret = v.compare_exchange_weak(expected, 1, memory_order_acquire, memory_order_relaxed);
      if(ret)
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(this, true);
        return true;
      }
      else
        return false;
    }
    //! Sets the atomic to zero
    void unlock() noexcept
    {
      // assert(v == 1);
      QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, true);
      v.store(0, memory_order_release);
    }
    constexpr bool int_yield(size_t) noexcept { return false; }
  };
  template <typename T> struct spinlockbase<lockable_ptr<T>>
  {
  private:
    lockable_ptr<T> v;

  public:
    typedef T *value_type;
    spinlockbase() noexcept {}
    spinlockbase(const spinlockbase &) = delete;
    //! Atomically move constructs
    spinlockbase(spinlockbase &&o) noexcept { v.store(o.v.exchange(nullptr, memory_order_acq_rel), memory_order_release); }
    spinlockbase &operator=(const spinlockbase &) = delete;
    spinlockbase &operator=(spinlockbase &&) = delete;
    //! Returns the memory pointer part of the atomic
    T *get() noexcept { return v.get(); }
    T *operator->() noexcept { return get(); }
    //! Returns the raw atomic
    T *load(memory_order o = memory_order_seq_cst) noexcept { return v.load(o); }
#if 0  // Forces cmpxchng on everything else, so avoid if at all possible.
      //! Sets the memory pointer part of the atomic preserving lockedness
      void set(T *a) noexcept
      {
        union
        {
          T *v;
          size_t n;
        } value;
        T *expected;
        do
        {
          value.v=v.load(memory_order_relaxed);
          expected=value.v;
          bool locked=value.n&1;
          value.v=a;
          if(locked) value.n|=1;
        } while(!v.compare_exchange_weak(expected, value.v, memory_order_acquire, memory_order_relaxed));
      }
#endif
    //! Sets the raw atomic
    void store(T *a, memory_order o = memory_order_seq_cst) noexcept { v.store(a, o); }
    bool try_lock() noexcept
    {
      union {
        T *v;
        size_t n;
      } value;
      value.v = v.load(memory_order_relaxed);
      if(value.n & 1)  // Avoid unnecessary cache line invalidation traffic
        return false;
      T *expected = value.v;
      value.n |= 1;
      return v.compare_exchange_weak(expected, value.v, memory_order_acquire, memory_order_relaxed);
    }
    void unlock() noexcept
    {
      union {
        T *v;
        size_t n;
      } value;
      value.v = v.load(memory_order_relaxed);
      // assert(value.n & 1);
      value.n &= ~(size_t) 1;
      v.store(value.v, memory_order_release);
    }
    constexpr bool int_yield(size_t) noexcept { return false; }
  };
  template <typename T> struct ordered_spinlockbase
  {
#ifndef _MSC_VER  // Amazingly VS2015 incorrectly fails when T is an unsigned!
    static_assert(((T) -1) > 0, "T must be an unsigned type for ordered_spinlockbase");
#endif
    typedef T value_type;

  protected:
    atomic<value_type> _v;
    using _halfT = typename detail::choose_half_type<value_type>::type;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)  // nameless struct/union
#endif
    union _internals {
      value_type uint;
      struct
      {
        _halfT exit, entry;
      };
    };
    static_assert(sizeof(_internals) == sizeof(value_type), "");
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  public:
#ifndef QUICKCPPLIB_ENABLE_VALGRIND
    constexpr
#endif
    ordered_spinlockbase() noexcept : _v(0)
    {
      QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(this);
      // v.store(0, memory_order_release);
    }
    ordered_spinlockbase(const ordered_spinlockbase &) = delete;
    //! Atomically move constructs
    ordered_spinlockbase(ordered_spinlockbase &&) noexcept : _v(0)
    {
      QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(this);
      // v.store(o.v.exchange(0, memory_order_acq_rel));
      // v.store(0, memory_order_release);
    }
    ~ordered_spinlockbase()
    {
#ifdef QUICKCPPLIB_ENABLE_VALGRIND
      _internals i = {_v.load(memory_order_relaxed)};
      if(i.entry != i.exit)
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, true);
      }
#endif
      QUICKCPPLIB_ANNOTATE_RWLOCK_DESTROY(this);
    }
    ordered_spinlockbase &operator=(const ordered_spinlockbase &) = delete;
    ordered_spinlockbase &operator=(ordered_spinlockbase &&) = delete;
    //! Returns the raw atomic
    constexpr T load(memory_order o = memory_order_seq_cst) const noexcept { return _v.load(o); }
    //! Sets the raw atomic
    void store(T a, memory_order o = memory_order_seq_cst) noexcept { _v.store(a, o); }

    //! Tries to lock the spinlock, returning true if successful.
    bool try_lock() noexcept
    {
      _internals i = {_v.load(memory_order_relaxed)}, o = i;
      // If locked, bail out immediately
      if(i.entry != i.exit)
        return false;
      o.entry++;
      if(_v.compare_exchange_weak(i.uint, o.uint, memory_order_acquire, memory_order_relaxed))
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(this, true);
        return true;
      }
      return false;
    }

  protected:
    value_type _begin_try_lock() noexcept { return _v.load(memory_order_relaxed); }
    bool _try_lock(value_type &state) noexcept
    {
      _internals i = {state}, o;
      o = i;
      o.entry++;
      if(_v.compare_exchange_weak(i.uint, o.uint, memory_order_acquire, memory_order_relaxed))
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(this, true);
        return true;
      }
      state = i.uint;
      return false;
    }

  public:
    //! Releases the lock
    void unlock() noexcept
    {
      QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, true);
      _internals i = {_v.load(memory_order_relaxed)}, o;
      for(;;)
      {
        o = i;
        o.exit++;
        if(_v.compare_exchange_weak(i.uint, o.uint, memory_order_release, memory_order_relaxed))
          return;
      }
    }
    bool int_yield(size_t) noexcept { return false; }
  };
  template <typename T> struct shared_spinlockbase
  {
#ifndef _MSC_VER  // Amazingly VS2015 incorrectly fails when T is an unsigned!
    static_assert(((T) -1) > 0, "T must be an unsigned type for shared_spinlockbase");
#endif
    typedef T value_type;

  protected:
    atomic<value_type> _v;

  public:
#ifndef QUICKCPPLIB_ENABLE_VALGRIND
    constexpr
#endif
    shared_spinlockbase() noexcept : _v(0)
    {
      QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(this);
#if QUICKCPPLIB_IN_THREAD_SANITIZER
      _v.store(0, memory_order_release);
#endif
    }
    shared_spinlockbase(const shared_spinlockbase &) = delete;
    //! Atomically move constructs
    shared_spinlockbase(shared_spinlockbase &&) noexcept : _v(0)
    {
      QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(this);
// v.store(o.v.exchange(0, memory_order_acq_rel));
#if QUICKCPPLIB_IN_THREAD_SANITIZER
      _v.store(0, memory_order_release);
#endif
    }
    ~shared_spinlockbase()
    {
#ifdef QUICKCPPLIB_ENABLE_VALGRIND
      value_type i = _v.load(memory_order_relaxed);
      if(i == 1)
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, true);
      }
      else if(i != 0)
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, false);
      }
#endif
      QUICKCPPLIB_ANNOTATE_RWLOCK_DESTROY(this);
    }
    shared_spinlockbase &operator=(const shared_spinlockbase &) = delete;
    shared_spinlockbase &operator=(shared_spinlockbase &&) = delete;
    //! Returns the raw atomic
    constexpr T load(memory_order o = memory_order_seq_cst) const noexcept { return _v.load(o); }
    //! Sets the raw atomic
    void store(T a, memory_order o = memory_order_seq_cst) noexcept { _v.store(a, o); }

    //! Tries to lock the spinlock for exclusive access, returning true if successful.
    bool try_lock() noexcept
    {
      value_type i = _v.load(memory_order_relaxed), o = i;
      // If locked by anybody, bail out immediately
      if(i)
        return false;
      o = 1;
      if(_v.compare_exchange_weak(i, o, memory_order_acquire, memory_order_relaxed))
      {
        QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(this, true);
        return true;
      }
      return false;
    }
    //! Releases the lock from exclusive access
    void unlock() noexcept
    {
      // assert(_v == 1);
      QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, true);
      _v.store(0, memory_order_release);
    }

    //! Tries to lock the spinlock for shared access, returning true if successful.
    bool try_lock_shared() noexcept
    {
      // OR in the exclusive lock bit
      value_type i = _v.fetch_or(1, memory_order_acquire);
      if(i & 1)
        return false;
      // If not locked, increment reader count and unlock
      i += 2;
      i &= ~1;
      _v.store(i, memory_order_release);
      QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(this, false);
      return true;
    }
    //! Releases the lock from shared access
    void unlock_shared() noexcept
    {
      value_type i;
      for(size_t n = 0;; n++)
      {
        i = _v.fetch_or(1, memory_order_acquire);
        // assert(i > 1);
        if(!(i & 1))
          break;
        // For very heavily contended locks, stop thrashing the cache line
        if(n > 2)
        {
          for(volatile size_t m = 0; m < 15000; m = m + 1)
            ;
        }
      }
      QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(this, false);
      i -= 2;
      i &= ~1;
      _v.store(i, memory_order_release);
    }

    //! Tries to convert an exclusive lock to a shared lock, returning true if successful.
    bool try_convert_lock_to_shared() noexcept
    {
      value_type expected = 1;
      return _v.compare_exchange_strong(expected, 2, memory_order_acquire, memory_order_relaxed);
    }
    //! Tries to convert a shared lock to an exclusive lock, returning true if successful.
    bool try_convert_lock_to_exclusive() noexcept
    {
      value_type expected = 2;
      return _v.compare_exchange_strong(expected, 1, memory_order_acquire, memory_order_relaxed);
    }

    bool int_yield(size_t) noexcept { return false; }
  };


  namespace detail
  {
    template <bool use_pause> inline void smt_pause() noexcept {};
    template <> inline void smt_pause<true>() noexcept
    {
#ifdef QUICKCPPLIB_SMT_PAUSE
      QUICKCPPLIB_SMT_PAUSE;
#endif
    };
  }
  //! \brief How many spins to loop, optionally calling the SMT pause instruction on Intel
  template <size_t spins, bool use_pause = true> struct spins_to_loop
  {
    template <class parenttype> struct policy : parenttype
    {
      static constexpr size_t spins_to_loop = spins;
      constexpr policy() {}
      policy(const policy &) = delete;
      constexpr policy(policy &&o) noexcept : parenttype(std::move(o)) {}
      constexpr inline bool int_yield(size_t n) noexcept
      {
        if(parenttype::int_yield(n))
          return true;
        if(n >= spins)
          return false;
        detail::smt_pause<use_pause>();
        return true;
      }
    };
  };
  //! \brief How many spins to yield the current thread's timeslice
  template <size_t spins> struct spins_to_yield
  {
    template <class parenttype> struct policy : parenttype
    {
      static constexpr size_t spins_to_yield = spins;
      constexpr policy() {}
      policy(const policy &) = delete;
      constexpr policy(policy &&o) noexcept : parenttype(std::move(o)) {}
      constexpr bool int_yield(size_t n) noexcept
      {
        if(parenttype::int_yield(n))
          return true;
        if(n >= spins)
          return false;
        this_thread::yield();
        return true;
      }
    };
  };
  //! \brief How many spins to sleep the current thread
  struct spins_to_sleep
  {
    template <class parenttype> struct policy : parenttype
    {
      constexpr policy() {}
      policy(const policy &) = delete;
      constexpr policy(policy &&o) noexcept : parenttype(std::move(o)) {}
      constexpr bool int_yield(size_t n) noexcept
      {
        if(parenttype::int_yield(n))
          return true;
        this_thread::sleep_for(chrono::milliseconds(1));
        return true;
      }
    };
  };
  //! \brief A spin policy which does nothing
  struct null_spin_policy
  {
    template <class parenttype> struct policy : parenttype
    {
    };
  };
  template <class T> inline bool is_lockable_locked(T &lockable) noexcept;

  /*! \class spinlock
  \brief A non-FIFO policy configurable spin lock meeting the `Mutex` concept providing
  the fastest possible spin lock.
  \tparam T An integral type capable of atomic usage

  `sizeof(spinlock<T>) == sizeof(T)`. Suitable for usage in shared memory.

  Meets the requirements of BasicLockable and Lockable. Provides a get() and set() for the
  type used for the spin lock. Suitable for limited usage in constexpr.

  \warning `spinlock<bool>` which might seem obvious is usually slower than `spinlock<uintptr_t>`
  on most architectures.

  So why reinvent the wheel?

  1. Policy configurable spin.

  2. Implemented in pure C++ 11 atomics so the thread sanitiser works as expected.

  3. Multistate locks are possible instead of just 0|1.

  4. I don't much care for doing writes during the spin which a lot of other spinlocks
  do. It generates an unnecessary amount of cache line invalidation traffic. Better to spin-read
  and only write when the read suggests you might have a chance.

  5. This spin lock can use a pointer to memory as the spin lock at the cost of some
  performance. It uses the bottom bit as the locked flag. See locked_ptr<T>.
  */
  template <typename T, template <class> class spinpolicy2 = spins_to_loop<125>::policy, template <class> class spinpolicy3 = spins_to_yield<250>::policy, template <class> class spinpolicy4 = spins_to_sleep::policy> class spinlock : public spinpolicy4<spinpolicy3<spinpolicy2<spinlockbase<T>>>>
  {
    typedef spinpolicy4<spinpolicy3<spinpolicy2<spinlockbase<T>>>> parenttype;

  public:
    constexpr spinlock() {}
    spinlock(const spinlock &) = delete;
    constexpr spinlock(spinlock &&o) noexcept : parenttype(std::move(o)) {}
    void lock() noexcept
    {
      for(size_t n = 0;; n++)
      {
        if(parenttype::try_lock())
          return;
        parenttype::int_yield(n);
      }
    }
    //! Locks if the atomic is not the supplied value, else returning false
    bool lock(T only_if_not_this) noexcept
    {
      for(size_t n = 0;; n++)
      {
        T expected = 0;
        if(parenttype::try_lock(expected))
          return true;
        if(expected == only_if_not_this)
          return false;
        parenttype::int_yield(n);
      }
    }
  };

#if 0  // fails its correctness testing in the unit tests, so disabled
  /*! \class ordered_spinlock
  \brief A FIFO policy configurable spin lock meeting the `Mutex` concept.
  \tparam T An unsigned integral type capable of atomic usage

  `sizeof(ordered_spinlock<T>) == sizeof(T)`. Suitable for usage in shared memory.

  Has all the advantages of spinlock apart from potential constexpr usage, but
  also guarantees FIFO ordering such that every lock grant is guaranteed to be in
  order of lock entry. It is implemented as two monotonically rising counters
  to enforce ordering of lock grants.  Maximum performance on Intel is roughly one
  third that of `spinlock<uintptr_t>`.

  \note Maximum contention is the largest integer fitting into half a `T`, so if
  T were an unsigned short, the maximum threads which could wait on this spinlock
  before experiencing undefined behaviour would be 255.
  */
  template <typename T = uintptr_t, template <class> class spinpolicy2 = spins_to_loop<125>::policy, template <class> class spinpolicy3 = spins_to_yield<250>::policy, template <class> class spinpolicy4 = spins_to_sleep::policy>
  class ordered_spinlock : public spinpolicy4<spinpolicy3<spinpolicy2<ordered_spinlockbase<T>>>>
  {
    typedef spinpolicy4<spinpolicy3<spinpolicy2<ordered_spinlockbase<T>>>> parenttype;

  public:
    constexpr ordered_spinlock() {}
    ordered_spinlock(const ordered_spinlock &) = delete;
    ordered_spinlock(ordered_spinlock &&o) noexcept : parenttype(std::move(o)) {}
    //! Locks the spinlock
    void lock() noexcept
    {
      auto state = parenttype::_begin_try_lock();
      for(size_t n = 0;; n++)
      {
        if(parenttype::_try_lock(state))
          return;
        parenttype::int_yield(n);
      }
    }
  };
#endif

  /*! \class shared_spinlock
  \brief A non-FIFO policy configurable shared/exclusive spin lock meeting the `SharedMutex` concept.
  \tparam T An unsigned integral type capable of atomic usage

  `sizeof(shared_spinlock<T>) == sizeof(T)`. Suitable for usage in shared memory.

  Implementing a fair shared spin lock with acceptable performance in just four
  bytes of storage is challenging, so this is a reader-biased shared lock which
  uses bit 0 as the exclusion bit, and the remainder of the bits to track how
  many readers are in the shared lock. Undefined behaviour will occur if the
  number of concurrent readers exceeds half the maximum value of a `T`.

  Maximum performance on Intel for exclusive locks is the same as a `spinlock<uintptr_t>`.
  For shared locks it is roughly one third that of `spinlock<uintptr_t>` due to performing
  twice as many atomic read-modify-updates.
  */
  template <typename T = uintptr_t, template <class> class spinpolicy2 = spins_to_loop<125>::policy, template <class> class spinpolicy3 = spins_to_yield<250>::policy, template <class> class spinpolicy4 = spins_to_sleep::policy> class shared_spinlock : public spinpolicy4<spinpolicy3<spinpolicy2<shared_spinlockbase<T>>>>
  {
    typedef spinpolicy4<spinpolicy3<spinpolicy2<shared_spinlockbase<T>>>> parenttype;

  public:
    constexpr shared_spinlock() {}
    shared_spinlock(const shared_spinlock &) = delete;
    shared_spinlock(shared_spinlock &&o) noexcept : parenttype(std::move(o)) {}
    //! Locks the spinlock for exclusive access
    void lock() noexcept
    {
      for(size_t n = 0;; n++)
      {
        if(parenttype::try_lock())
          return;
        parenttype::int_yield(n);
      }
    }
    //! Locks the spinlock for shared access
    void lock_shared() noexcept
    {
      for(size_t n = 0;; n++)
      {
        if(parenttype::try_lock_shared())
          return;
        parenttype::int_yield(n);
      }
    }
  };

  //! \brief Determines if a lockable is locked. Type specialise this for performance if your lockable allows examination.
  template <class T> inline bool is_lockable_locked(T &lockable) noexcept
  {
    if(lockable.try_lock())
    {
      lockable.unlock();
      return false;
    }
    return true;
  }
  // For when used with a spinlock
  template <class T, template <class> class spinpolicy2, template <class> class spinpolicy3, template <class> class spinpolicy4> constexpr inline T is_lockable_locked(spinlock<T, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) noexcept
  {
#ifdef QUICKCPPLIB_HAVE_TRANSACTIONAL_MEMORY_COMPILER
    // Annoyingly the atomic ops are marked as unsafe for atomic transactions, so ...
    return *((volatile T *) &lockable);
#else
    return lockable.load(memory_order_consume);
#endif
  }
  // For when used with a spinlock
  template <class T, template <class> class spinpolicy2, template <class> class spinpolicy3, template <class> class spinpolicy4> constexpr inline T is_lockable_locked(const spinlock<T, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) noexcept
  {
#ifdef QUICKCPPLIB_HAVE_TRANSACTIONAL_MEMORY_COMPILER
    // Annoyingly the atomic ops are marked as unsafe for atomic transactions, so ...
    return *((volatile T *) &lockable);
#else
    return lockable.load(memory_order_consume);
#endif
  }
  // For when used with a locked_ptr
  template <class T, template <class> class spinpolicy2, template <class> class spinpolicy3, template <class> class spinpolicy4> constexpr inline bool is_lockable_locked(spinlock<lockable_ptr<T>, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) noexcept { return ((size_t) lockable.load(memory_order_consume)) & 1; }
#if 0
  // For when used with an ordered spinlock
  template <class T, template <class> class spinpolicy2, template <class> class spinpolicy3, template <class> class spinpolicy4> constexpr inline bool is_lockable_locked(ordered_spinlock<T, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) noexcept
  {
    using halfT = typename detail::choose_half_type<T>::type;
    union {
      T v;
      struct
      {
        halfT entry, exit;
      };
    };
#ifdef QUICKCPPLIB_HAVE_TRANSACTIONAL_MEMORY_COMPILER
    // Annoyingly the atomic ops are marked as unsafe for atomic transactions, so ...
    v = *((volatile T *) &lockable);
#else
    v = lockable.load(memory_order_consume);
#endif
    return entry != exit;
  }
#endif
  // For when used with a shared spinlock
  template <class T, template <class> class spinpolicy2, template <class> class spinpolicy3, template <class> class spinpolicy4> constexpr inline T is_lockable_locked(shared_spinlock<T, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) noexcept
  {
#ifdef QUICKCPPLIB_HAVE_TRANSACTIONAL_MEMORY_COMPILER
    // Annoyingly the atomic ops are marked as unsafe for atomic transactions, so ...
    return *((volatile T *) &lockable);
#else
    return lockable.load(memory_order_consume);
#endif
  }

#ifndef QUICKCPPLIB_BEGIN_TRANSACT_LOCK
#ifdef QUICKCPPLIB_HAVE_TRANSACTIONAL_MEMORY_COMPILER
#undef QUICKCPPLIB_USING_INTEL_TSX
#define QUICKCPPLIB_BEGIN_TRANSACT_LOCK(lockable)                                                                                                                                                                                                                                                                              \
  __transaction_relaxed                                                                                                                                                                                                                                                                                                        \
  {                                                                                                                                                                                                                                                                                                                            \
    (void) QUICKCPPLIB_NAMESPACE::is_lockable_locked(lockable);                                                                                                                                                                                                                                                                \
    {
#define QUICKCPPLIB_BEGIN_TRANSACT_LOCK_ONLY_IF_NOT(lockable, only_if_not_this)                                                                                                                                                                                                                                                \
  __transaction_relaxed                                                                                                                                                                                                                                                                                                        \
  {                                                                                                                                                                                                                                                                                                                            \
    if((only_if_not_this) != QUICKCPPLIB_NAMESPACE::is_lockable_locked(lockable))                                                                                                                                                                                                                                              \
    {
#define QUICKCPPLIB_END_TRANSACT_LOCK(lockable)                                                                                                                                                                                                                                                                                \
  }                                                                                                                                                                                                                                                                                                                            \
  }
#define QUICKCPPLIB_BEGIN_NESTED_TRANSACT_LOCK(N) __transaction_relaxed
#define QUICKCPPLIB_END_NESTED_TRANSACT_LOCK(N)
#endif  // QUICKCPPLIB_BEGIN_TRANSACT_LOCK
#endif

#ifndef QUICKCPPLIB_BEGIN_TRANSACT_LOCK
#define QUICKCPPLIB_BEGIN_TRANSACT_LOCK(lockable)                                                                                                                                                                                                                                                                              \
  {                                                                                                                                                                                                                                                                                                                            \
    QUICKCPPLIB_NAMESPACE::configurable_spinlock::lock_guard<decltype(lockable)> __tsx_transaction(lockable);
#define QUICKCPPLIB_BEGIN_TRANSACT_LOCK_ONLY_IF_NOT(lockable, only_if_not_this)                                                                                                                                                                                                                                                \
  if(lockable.lock(only_if_not_this))                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    QUICKCPPLIB_NAMESPACE::configurable_spinlock::lock_guard<decltype(lockable)> __tsx_transaction(lockable, QUICKCPPLIB_NAMESPACE::adopt_lock_t());
#define QUICKCPPLIB_END_TRANSACT_LOCK(lockable) }
#define QUICKCPPLIB_BEGIN_NESTED_TRANSACT_LOCK(N)
#define QUICKCPPLIB_END_NESTED_TRANSACT_LOCK(N)
#endif  // QUICKCPPLIB_BEGIN_TRANSACT_LOCK
}

QUICKCPPLIB_NAMESPACE_END

#endif  // QUICKCPPLIB_HPP
