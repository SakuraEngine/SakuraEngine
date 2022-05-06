/* byte support
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Apr 2018


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

#ifndef QUICKCPPLIB_BYTE_HPP
#define QUICKCPPLIB_BYTE_HPP

#include "config.hpp"

#ifndef QUICKCPPLIB_USE_STD_BYTE
#if _HAS_CXX17 || __cplusplus >= 201700
#define QUICKCPPLIB_USE_STD_BYTE 1
#else
#define QUICKCPPLIB_USE_STD_BYTE 0
#endif
#endif

#if QUICKCPPLIB_USE_STD_BYTE

#include <cstddef>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace byte
{
  using std::byte;
  template <class IntegerType>
  inline constexpr byte to_byte(IntegerType v) noexcept {
    return static_cast<byte>(v);
  }
}

QUICKCPPLIB_NAMESPACE_END

#else

#include "byte/include/nonstd/byte.hpp"

QUICKCPPLIB_NAMESPACE_BEGIN

namespace byte
{
  using nonstd::byte;
  using nonstd::to_byte;
}

QUICKCPPLIB_NAMESPACE_END

#endif

#endif
