/* Offset pointer support
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: June 2018


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

#ifndef QUICKCPPLIB_OFFSET_PTR_HPP
#define QUICKCPPLIB_OFFSET_PTR_HPP

#include "config.hpp"

#include <atomic>
#include <cstdint>

QUICKCPPLIB_NAMESPACE_BEGIN

//! \brief The namespace for the offset pointer types
namespace offset_ptr
{
  namespace detail
  {
    template <class T> struct ptr_to_intptr
    {
      using type = intptr_t;
    };
    template <class T> struct ptr_to_intptr<const T>
    {
      using type = const intptr_t;
    };
    template <class T> struct ptr_to_intptr<volatile T>
    {
      using type = volatile intptr_t;
    };
    template <class T> struct ptr_to_intptr<const volatile T>
    {
      using type = const volatile intptr_t;
    };
  }

  /* \class offset_ptr
  \brief A pointer invariant to relocation if the target is equally relocated. Very useful for
  shared memory regions where one is pointing into the same shared memory region.

  On a Skylake x64 CPU, setting the offset pointer has a latency of 5 CPU cycles, throughput is 2.

  On a Skylake x64 CPU, getting the offset pointer has a latency of 3 CPU cycles, throughput is 2.
  */
  template <class T> class offset_ptr
  {
    using intptr_type = typename detail::ptr_to_intptr<T>::type;
    intptr_type _v;

    constexpr void _set(T *v = nullptr) { _v = reinterpret_cast<intptr_t>(v) - reinterpret_cast<intptr_t>(this); }
    constexpr T *_get() const { return reinterpret_cast<T *>(_v + reinterpret_cast<intptr_t>(this)); }
    struct _reference_disabled_type;
    using _element_type = std::conditional_t<!std::is_void<T>::value, T, _reference_disabled_type>;

  public:
    //! The pointer type
    using pointer = T *;
    //! The pointed to type
    using element_type = T;

    //! Construct a null pointer
    constexpr offset_ptr() noexcept { _set(); }
    //! Implicitly construct a null pointer
    constexpr offset_ptr(std::nullptr_t) noexcept { _set(); }
    //! Copy constructor
    constexpr offset_ptr(const offset_ptr &o) noexcept { _set(o._get()); }
    //! Move constructor
    constexpr offset_ptr(offset_ptr &&o) noexcept { _set(o._get()); }
    //! Copy assignment
    constexpr offset_ptr &operator=(const offset_ptr &o) noexcept
    {
      _set(o._get());
      return *this;
    }
    //! Move assignment
    constexpr offset_ptr &operator=(offset_ptr &&o) noexcept
    {
      _set(o._get());
      return *this;
    }
    ~offset_ptr() = default;

    //! Implicitly construct
    constexpr offset_ptr(pointer v) { _set(v); }

    //! Implicitly convert
    constexpr operator pointer() const noexcept { return _get(); }
    //! Dereference
    constexpr pointer operator->() const noexcept { return _get(); }
    //! Dereference
    constexpr _element_type &operator*() noexcept { return *_get(); }
    //! Dereference
    constexpr const _element_type &operator*() const noexcept { return *_get(); }
  };
  template <class T> class offset_ptr<const T>
  {
    using intptr_type = typename detail::ptr_to_intptr<T>::type;
    intptr_type _v;

    constexpr void _set(const T *v = nullptr) { _v = reinterpret_cast<intptr_t>(v) - reinterpret_cast<intptr_t>(this); }
    constexpr const T *_get() const { return reinterpret_cast<const T *>(_v + reinterpret_cast<intptr_t>(this)); }
    struct _reference_disabled_type;
    using _element_type = std::conditional_t<!std::is_void<T>::value, T, _reference_disabled_type>;

  public:
    using pointer = const T *;
    using element_type = const T;

    constexpr offset_ptr() noexcept { _set(); }
    constexpr offset_ptr(const offset_ptr &o) noexcept { _set(o._get()); }
    constexpr offset_ptr(offset_ptr &&o) noexcept { _set(o._get()); }
    constexpr offset_ptr &operator=(const offset_ptr &o) noexcept
    {
      _set(o._get());
      return *this;
    }
    constexpr offset_ptr &operator=(offset_ptr &&o) noexcept
    {
      _set(o._get());
      return *this;
    }
    ~offset_ptr() = default;

    constexpr offset_ptr(pointer v) { _set(v); }

    constexpr operator pointer() const noexcept { return _get(); }
    constexpr pointer operator->() const noexcept { return _get(); }
    constexpr _element_type &operator*() const noexcept { return *_get(); }
  };

  /* \class atomic_offset_ptr
  \brief A pointer invariant to relocation if the target is equally relocated. Very useful for
  shared memory regions where one is pointing into the same shared memory region.

  On a Skylake x64 CPU, setting the offset pointer has a latency of 21 CPU cycles, throughput is 1 (`memory_order_seq_cst`).

  On a Skylake x64 CPU, getting the offset pointer has a latency of 20 CPU cycles, throughput is 1 (`memory_order_seq_cst`).
  */
  template <class T> class atomic_offset_ptr
  {
    using intptr_type = typename detail::ptr_to_intptr<T>::type;
    std::atomic<intptr_type> _v;

    constexpr void _set(T *v, std::memory_order o) { _v.store(reinterpret_cast<intptr_t>(v) - reinterpret_cast<intptr_t>(this), o); }
    constexpr T *_get(std::memory_order o) const { return reinterpret_cast<T *>(_v.load(o) + reinterpret_cast<intptr_t>(this)); }
    struct _reference_disabled_type;
    using _element_type = std::conditional_t<!std::is_void<T>::value, T, _reference_disabled_type>;

  public:
    //! The pointer type
    using pointer = T *;
    //! The pointed to type
    using element_type = T;

    //! Construct a null pointer
    constexpr atomic_offset_ptr(std::memory_order w = std::memory_order_seq_cst) noexcept { _set(nullptr, w); }
    //! Implicitly construct a null pointer
    constexpr atomic_offset_ptr(std::nullptr_t, std::memory_order w = std::memory_order_seq_cst) noexcept { _set(nullptr, w); }
    //! Copy constructor
    constexpr atomic_offset_ptr(const atomic_offset_ptr &o, std::memory_order w = std::memory_order_seq_cst, std::memory_order r = std::memory_order_seq_cst) noexcept { _set(o._get(r), w); }
    //! Move constructor
    constexpr atomic_offset_ptr(atomic_offset_ptr &&o, std::memory_order w = std::memory_order_seq_cst, std::memory_order r = std::memory_order_seq_cst) noexcept { _set(o._get(r, w)); }
    //! Copy assignment
    constexpr atomic_offset_ptr &operator=(const atomic_offset_ptr &o) noexcept
    {
      _set(o._get(std::memory_order_seq_cst), std::memory_order_seq_cst);
      return *this;
    }
    //! Move assignment
    constexpr atomic_offset_ptr &operator=(atomic_offset_ptr &&o) noexcept
    {
      _set(o._get(std::memory_order_seq_cst), std::memory_order_seq_cst);
      return *this;
    }
    ~atomic_offset_ptr() = default;

    //! Implicitly construct
    constexpr atomic_offset_ptr(pointer v, std::memory_order w = std::memory_order_seq_cst) { _set(v, w); }

    //! Get
    constexpr pointer get(std::memory_order r = std::memory_order_seq_cst) const noexcept { return _get(r); }
    //! Set
    constexpr void set(pointer v, std::memory_order w = std::memory_order_seq_cst) noexcept { _set(v, w); }
  };
}

QUICKCPPLIB_NAMESPACE_END

#endif
