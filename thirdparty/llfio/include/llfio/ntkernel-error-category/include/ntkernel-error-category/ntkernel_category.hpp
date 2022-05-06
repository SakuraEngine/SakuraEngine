/* NT kernel error code category
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: July 2017


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

#ifndef NTKERNEL_CATEGORY_HPP
#define NTKERNEL_CATEGORY_HPP

#include "config.hpp"

#include <system_error>

namespace ntkernel_error_category
{
  NTKERNEL_ERROR_CATEGORY_API NTKERNEL_ERROR_CATEGORY_INLINE_API const std::error_category &ntkernel_category() noexcept;
}

#if NTKERNEL_ERROR_CATEGORY_INLINE
#include "detail/ntkernel_category_impl.ipp"
#endif

#endif
