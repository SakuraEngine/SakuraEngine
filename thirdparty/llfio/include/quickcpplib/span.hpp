/* span support
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Sept 2016


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

#ifndef QUICKCPPLIB_SPAN_HPP
#define QUICKCPPLIB_SPAN_HPP

// `_HAS_CXX20` is defined in `<vcruntime.h>`, i.e. we need to drag it in (indirectly).
// `<cstddef>` is inexpensive to include and would be included for `std::size_t` anyways.
// It is also guaranteed to include the MS STL versioning machinery for `std::byte`.
#include <cstddef>

#include "config.hpp"

#ifdef QUICKCPPLIB_USE_STD_SPAN

#include <span>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace span
{
  template <class T, size_t Extent = std::dynamic_extent> using span = std::span<T, Extent>;
}

QUICKCPPLIB_NAMESPACE_END

#elif(_HAS_CXX20 || __cplusplus >= 202002) && __has_include(<span>)

#include <span>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace span
{
  template <class T, size_t Extent = std::dynamic_extent> using span = std::span<T, Extent>;
}

#undef QUICKCPPLIB_USE_STD_SPAN
#define QUICKCPPLIB_USE_STD_SPAN 1

QUICKCPPLIB_NAMESPACE_END

#else

#define gsl_CONFIG_DEFAULTS_VERSION 1

#include "gsl-lite/include/gsl/gsl-lite.hpp"

QUICKCPPLIB_NAMESPACE_BEGIN

namespace span
{
  template <class T> using span = gsl_lite::span<T>;
}

QUICKCPPLIB_NAMESPACE_END

#endif

#endif
