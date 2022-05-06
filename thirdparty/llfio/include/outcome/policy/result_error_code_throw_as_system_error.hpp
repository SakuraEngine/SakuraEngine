/* Policies for result and outcome
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (8 commits)
File Created: Oct 2017


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

#ifndef OUTCOME_POLICY_RESULT_ERROR_CODE_THROW_AS_SYSTEM_ERROR_HPP
#define OUTCOME_POLICY_RESULT_ERROR_CODE_THROW_AS_SYSTEM_ERROR_HPP

#include "../bad_access.hpp"
#include "base.hpp"

#include <system_error>

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace policy
{
  template <class T, class EC, class E> struct error_code_throw_as_system_error;
  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  template <class T, class EC> struct error_code_throw_as_system_error<T, EC, void> : base
  {
    template <class Impl> static constexpr void wide_value_check(Impl &&self)
    {
      if(!base::_has_value(std::forward<Impl>(self)))
      {
        if(base::_has_error(std::forward<Impl>(self)))
        {
          // ADL discovered
          outcome_throw_as_system_error_with_payload(base::_error(std::forward<Impl>(self)));
        }
        OUTCOME_THROW_EXCEPTION(bad_result_access("no value"));  // NOLINT
      }
    }
    template <class Impl> static constexpr void wide_error_check(Impl &&self)
    {
      if(!base::_has_error(std::forward<Impl>(self)))
      {
        OUTCOME_THROW_EXCEPTION(bad_result_access("no error"));  // NOLINT
      }
    }
  };
}  // namespace policy

OUTCOME_V2_NAMESPACE_END

#endif
