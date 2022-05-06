/* PMR support
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Nov 2018


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

#ifndef QUICKCPPLIB_MEMORY_RESOURCE_HPP
#define QUICKCPPLIB_MEMORY_RESOURCE_HPP

#include "config.hpp"

#ifdef QUICKCPPLIB_USE_STD_MEMORY_RESOURCE

#include <memory_resource>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace pmr = std::pmr;

QUICKCPPLIB_NAMESPACE_END

#elif(_HAS_CXX17 || __cplusplus >= 201700) && __has_include(<memory_resource>)

#include <memory_resource>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace pmr = std::pmr;

QUICKCPPLIB_NAMESPACE_END

#else

#include <cstddef>
#include <memory>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace pmr
{
  //! memory_resource emulation
  class memory_resource
  {
    friend inline bool operator==(const memory_resource &a, const memory_resource &b) noexcept;
    friend inline bool operator!=(const memory_resource &a, const memory_resource &b) noexcept;
    virtual void *do_allocate(size_t bytes, size_t alignment) = 0;
    virtual void do_deallocate(void *p, size_t bytes, size_t alignment) = 0;
    virtual bool do_is_equal(const memory_resource &other) const noexcept = 0;

  public:
    memory_resource() = default;
    memory_resource(const memory_resource &) = default;
    memory_resource &operator=(const memory_resource &) = default;
    virtual ~memory_resource() {}

    QUICKCPPLIB_NODISCARD void *allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) { return do_allocate(bytes, alignment); }
    void deallocate(void *p, size_t bytes, size_t alignment = alignof(std::max_align_t)) { return do_deallocate(p, bytes, alignment); }
  };
  inline bool operator==(const memory_resource &a, const memory_resource &b) noexcept { return a.do_is_equal(b); }
  inline bool operator!=(const memory_resource &a, const memory_resource &b) noexcept { return !a.do_is_equal(b); }

  //! A just barely good enough C++ 14 emulation of monotonic_buffer_resource
  class monotonic_buffer_resource : public memory_resource
  {
    char *_ptr{nullptr}, *_end{nullptr};

    virtual void *do_allocate(size_t bytes, size_t alignment) override
    {
      if(_ptr >= _end)
      {
        throw std::bad_alloc();
      }
      _ptr = (char *) (((uintptr_t) _ptr + alignment - 1) & ~(alignment - 1));
      void *ret = (void *) _ptr;
      _ptr += bytes;
      if(_ptr > _end)
      {
        throw std::bad_alloc();
      }
      return ret;
    }
    virtual void do_deallocate(void *p, size_t bytes, size_t alignment) override
    {
      (void) p;
      (void) bytes;
      (void) alignment;
    }
    virtual bool do_is_equal(const memory_resource &other) const noexcept override { return this == &other; }

  public:
    monotonic_buffer_resource() = default;
    monotonic_buffer_resource(const monotonic_buffer_resource &) = delete;
    monotonic_buffer_resource(void *buffer, size_t length)
        : _ptr((char *) buffer)
        , _end((char *) buffer + length)
    {
    }
    monotonic_buffer_resource &operator=(const monotonic_buffer_resource &) = delete;
  };

  //! The world's worst C++ 14 emulation of polymorphic_allocator, which maps onto std::allocator
  template <class T> class polymorphic_allocator
  {
    template <class U> friend class polymorphic_allocator;
    memory_resource *_r{nullptr};

  public:
    using value_type = T;

    polymorphic_allocator() = default;
    polymorphic_allocator(memory_resource *r)
        : _r(r)
    {
    }
    polymorphic_allocator(const polymorphic_allocator &o)
        : _r(o._r)
    {
    }
    template <class U>
    polymorphic_allocator(const polymorphic_allocator<U> &o) noexcept
        : _r(o._r)
    {
    }
    polymorphic_allocator &operator=(const polymorphic_allocator &) = delete;

    memory_resource *resource() const { return _r; }

    QUICKCPPLIB_NODISCARD T *allocate(size_t n) { return static_cast<T *>(resource()->allocate(n * sizeof(T), alignof(T))); }
    void deallocate(T *p, size_t n) { resource()->deallocate(p, n * sizeof(T), alignof(T)); }
    template <class U, class... Args> void construct(U *p, Args &&...args) { new(p) U(static_cast<Args &&>(args)...); }
    template <class U> void destroy(U *p) { p->~U(); }
  };
  template <class T1, class T2> inline bool operator==(const polymorphic_allocator<T1> &a, const polymorphic_allocator<T2> &b) noexcept
  {
    return *a.resource() == *b.resource();
  }
  template <class T1, class T2> inline bool operator!=(const polymorphic_allocator<T1> &a, const polymorphic_allocator<T2> &b) noexcept
  {
    return *a.resource() != *b.resource();
  }
}  // namespace pmr

QUICKCPPLIB_NAMESPACE_END

#endif

#endif
