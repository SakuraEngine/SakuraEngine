/* Tells C++ coroutines about Outcome's result
(C) 2019 Niall Douglas <http://www.nedproductions.biz/> (12 commits)
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

#ifndef OUTCOME_EXPERIMENTAL_COROUTINE_SUPPORT_HPP
#define OUTCOME_EXPERIMENTAL_COROUTINE_SUPPORT_HPP

#include "../config.hpp"

#define OUTCOME_COROUTINE_SUPPORT_NAMESPACE_BEGIN                                                                                                                                                                                                                                                                              \
  OUTCOME_V2_NAMESPACE_BEGIN namespace experimental                                                                                                                                                                                                                                                                            \
  {                                                                                                                                                                                                                                                                                                                            \
    namespace awaitables                                                                                                                                                                                                                                                                                                       \
    {
//
#define OUTCOME_COROUTINE_SUPPORT_NAMESPACE_EXPORT_BEGIN                                                                                                                                                                                                                                                                       \
  OUTCOME_V2_NAMESPACE_EXPORT_BEGIN namespace experimental                                                                                                                                                                                                                                                                     \
  {                                                                                                                                                                                                                                                                                                                            \
    namespace awaitables                                                                                                                                                                                                                                                                                                       \
    {
//
#define OUTCOME_COROUTINE_SUPPORT_NAMESPACE_END                                                                                                                                                                                                                                                                                \
  }                                                                                                                                                                                                                                                                                                                            \
  }                                                                                                                                                                                                                                                                                                                            \
  OUTCOME_V2_NAMESPACE_END

#ifdef __cpp_exceptions
#include "status-code/include/system_code_from_exception.hpp"
OUTCOME_V2_NAMESPACE_BEGIN
namespace awaitables
{
  namespace detail
  {
    inline bool error_is_set(SYSTEM_ERROR2_NAMESPACE::system_code &sc) noexcept { return sc.failure(); }
    inline SYSTEM_ERROR2_NAMESPACE::system_code error_from_exception(std::exception_ptr &&ep = std::current_exception(), SYSTEM_ERROR2_NAMESPACE::system_code not_matched = SYSTEM_ERROR2_NAMESPACE::generic_code(SYSTEM_ERROR2_NAMESPACE::errc::resource_unavailable_try_again)) noexcept
    {
      return SYSTEM_ERROR2_NAMESPACE::system_code_from_exception(static_cast<std::exception_ptr &&>(ep), static_cast<SYSTEM_ERROR2_NAMESPACE::system_code &&>(not_matched));
    }
  }  // namespace detail
}  // namespace awaitables
OUTCOME_V2_NAMESPACE_END
#endif

#include "../detail/coroutine_support.ipp"

#undef OUTCOME_COROUTINE_SUPPORT_NAMESPACE_BEGIN
#undef OUTCOME_COROUTINE_SUPPORT_NAMESPACE_EXPORT_BEGIN
#undef OUTCOME_COROUTINE_SUPPORT_NAMESPACE_END

#endif
