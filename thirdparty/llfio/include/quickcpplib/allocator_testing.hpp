/* Provides an allocator useful for unit testing exception safety
(C) 2014-2017 Niall Douglas <http://www.nedproductions.biz/> (2 commits)
File Created: Aug 2014


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

#ifndef QUICKCPPLIB_ALLOCATOR_TESTING_HPP
#define QUICKCPPLIB_ALLOCATOR_TESTING_HPP

#include "config.hpp"

#include <atomic>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace allocator_testing
{

  struct config
  {
    std::atomic<size_t> count, fail_from, fail_at;
    config()
        : count(0)
        , fail_from((size_t) -1)
        , fail_at((size_t) -1)
    {
    }
  };

  static inline config &get_config(bool reset = false)
  {
    static config c;
    if(reset)
    {
      c.count = 0;
      c.fail_from = (size_t) -1;
      c.fail_at = (size_t) -1;
    }
    return c;
  }

  template <class T, class A = std::allocator<T>> struct allocator : public A
  {
    template <class U> struct rebind
    {
      typedef allocator<U> other;
    };
    allocator() {}
    allocator(const allocator &o)
        : A(o)
    {
    }
    template <class U>
    allocator(const allocator<U> &o)
        : A(o)
    {
    }
    typename A::pointer allocate(typename A::size_type n, typename std::allocator<void>::const_pointer hint = 0)
    {
      config &c = get_config();
      size_t count = ++c.count;
      if(count >= c.fail_from || count == c.fail_at)
        throw std::bad_alloc();
      return A::allocate(n, hint);
    }
  };
}

QUICKCPPLIB_NAMESPACE_END

#endif
