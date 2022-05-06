/* std::declval without #include <utility>
(C) 2021 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Nov 2021


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

#ifndef QUICKCPPLIB_DECLVAL_HPP
#define QUICKCPPLIB_DECLVAL_HPP

#include "config.hpp"

#include <cstdlib>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace detail
{
  template<class T, class U = T&&> U declval_selector(int);
  template<class T> T declval_selector(long);
  template<class T, class U> struct declval_issame { static constexpr bool value = false; };
  template<class T> struct declval_issame<T, T> { static constexpr bool value = true; };
}

//! \brief `std::declval` without `<utility>`
template<class T> auto declval() noexcept -> decltype(detail::declval_selector<T>(0))
{
  static_assert(!detail::declval_issame<T, T>::value, "declval() must not be instanced!");
  abort();
}

QUICKCPPLIB_NAMESPACE_END

#endif
