/* A less simple result type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
File Created: June 2017


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

#ifndef OUTCOME_STD_OUTCOME_HPP
#define OUTCOME_STD_OUTCOME_HPP

#include "basic_outcome.hpp"
#include "std_result.hpp"

#ifndef STD_BASIC_OUTCOME_FAILURE_EXCEPTION_FROM_ERROR
#define STD_BASIC_OUTCOME_FAILURE_EXCEPTION_FROM_ERROR
namespace std  // NOLINT
{
  inline exception_ptr basic_outcome_failure_exception_from_error(const error_code &ec) { return make_exception_ptr(system_error(ec)); }
}  // namespace std
#endif

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
template <class R, class S = std::error_code, class P = std::exception_ptr, class NoValuePolicy = policy::default_policy<R, S, P>>  //
using std_outcome = basic_outcome<R, S, P, NoValuePolicy>;

OUTCOME_V2_NAMESPACE_END

#include "policy/outcome_error_code_throw_as_system_error.hpp"
#include "policy/outcome_exception_ptr_rethrow.hpp"

#endif
