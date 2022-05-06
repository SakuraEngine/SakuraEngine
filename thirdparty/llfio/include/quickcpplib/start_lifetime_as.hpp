/* bless support
(C) 2019 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Oct 2019


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

#ifndef QUICKCPPLIB_START_LIFETIME_AS_HPP
#define QUICKCPPLIB_START_LIFETIME_AS_HPP

#include "config.hpp"

#include <new>

QUICKCPPLIB_NAMESPACE_BEGIN
namespace start_lifetime_as
{
  namespace detail
  {
    using namespace std;
    template <class T> constexpr inline T *launder(T *p, ...) noexcept { return p; }
    template <typename T> inline T *start_lifetime_as(void *p, ...) { return reinterpret_cast<T *>(p); }

    template <class T> constexpr inline T *_launder(T *p) noexcept { return launder<T>(p); }
    template <typename T> inline T *_start_lifetime_as(void *p) { return start_lifetime_as<T>(p); }
  }  // namespace detail

  //! `std::launder<T>` from C++ 17, or an emulation
  template <class T> QUICKCPPLIB_NODISCARD constexpr inline T *launder(T *p) noexcept { return detail::_launder(p); }
  //! `std::start_lifetime_as<T>` from C++ 23, or an emulation
  template <typename T> QUICKCPPLIB_NODISCARD inline T *start_lifetime_as(void *p) { return detail::_start_lifetime_as<T>(p); }
  //! `std::start_lifetime_as<T>` from C++ 23, or an emulation
  template <typename T> QUICKCPPLIB_NODISCARD inline const T *start_lifetime_as(const void *p)
  {
    return detail::_start_lifetime_as<const T>(const_cast<void *>(p)); /* works around a GCC bug */
  }

}  // namespace start_lifetime_as

QUICKCPPLIB_NAMESPACE_END

#endif
