/* Policies for result and outcome
(C) 2018-2019 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: Sep 2018


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

#ifndef OUTCOME_POLICY_FAIL_TO_COMPILE_OBSERVERS_HPP
#define OUTCOME_POLICY_FAIL_TO_COMPILE_OBSERVERS_HPP

#include "base.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

#define OUTCOME_FAIL_TO_COMPILE_OBSERVERS_MESSAGE                                                                                                                                                                                                                                                                              \
  "Attempt to wide observe value, error or "                                                                                                                                                                                                                                                                                   \
  "exception for a basic_result/basic_outcome given an EC or EP type which is not void, and for whom "                                                                                                                                                                                                                         \
  "trait::is_error_code_available<EC>, trait::is_exception_ptr_available<EC>, and trait::is_exception_ptr_available<EP> "                                                                                                                                                                                                      \
  "are all false. Please specify a NoValuePolicy to tell basic_result/basic_outcome what to do, or else use "                                                                                                                                                                                                                  \
  "a more specific convenience type alias such as unchecked<T, E> to indicate you want the wide "                                                                                                                                                                                                                              \
  "observers to be narrow, or checked<T, E> to indicate you always want an exception throw etc."

namespace policy
{
  struct fail_to_compile_observers : base
  {
    template <class Impl> static constexpr void wide_value_check(Impl && /* unused */) { static_assert(!std::is_same<Impl, Impl>::value, OUTCOME_FAIL_TO_COMPILE_OBSERVERS_MESSAGE); }
    template <class Impl> static constexpr void wide_error_check(Impl && /* unused */) { static_assert(!std::is_same<Impl, Impl>::value, OUTCOME_FAIL_TO_COMPILE_OBSERVERS_MESSAGE); }
    template <class Impl> static constexpr void wide_exception_check(Impl && /* unused */) { static_assert(!std::is_same<Impl, Impl>::value, OUTCOME_FAIL_TO_COMPILE_OBSERVERS_MESSAGE); }
  };
}  // namespace policy

#undef OUTCOME_FAIL_TO_COMPILE_OBSERVERS_MESSAGE

OUTCOME_V2_NAMESPACE_END

#endif
