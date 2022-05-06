/* A very simple result type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (8 commits)
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

#ifndef OUTCOME_STD_RESULT_HPP
#define OUTCOME_STD_RESULT_HPP

#include "basic_result.hpp"
#include "detail/trait_std_error_code.hpp"
#include "detail/trait_std_exception.hpp"

#include "policy/fail_to_compile_observers.hpp"
#include "policy/result_error_code_throw_as_system_error.hpp"
#include "policy/result_exception_ptr_rethrow.hpp"
#include "policy/throw_bad_result_access.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
namespace policy
{
  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  template <class T, class EC, class E>
  using default_policy = std::conditional_t<  //
  std::is_void<EC>::value && std::is_void<E>::value,
  terminate,                                                                                                                     //
  std::conditional_t<                                                                                                            //
  trait::is_error_code_available<EC>::value, error_code_throw_as_system_error<T, EC, E>,                                         //
  std::conditional_t<                                                                                                            //
  trait::is_exception_ptr_available<EC>::value || trait::is_exception_ptr_available<E>::value, exception_ptr_rethrow<T, EC, E>,  //
  fail_to_compile_observers                                                                                                      //
  >>>;
}  // namespace policy

/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
template <class R, class S = std::error_code, class NoValuePolicy = policy::default_policy<R, S, void>>  //
using std_result = basic_result<R, S, NoValuePolicy>;

/*! AWAITING HUGO JSON CONVERSION TOOL 
type alias template <class R, class S = std::error_code> std_unchecked. Potential doc page: `std_unchecked<T, E = std::error_code>`
*/
template <class R, class S = std::error_code> using std_unchecked = std_result<R, S, policy::all_narrow>;

/*! AWAITING HUGO JSON CONVERSION TOOL 
type alias template <class R, class S = std::error_code> std_checked. Potential doc page: `std_checked<T, E = std::error_code>`
*/
template <class R, class S = std::error_code> using std_checked = std_result<R, S, policy::throw_bad_result_access<S, void>>;

OUTCOME_V2_NAMESPACE_END

#endif
