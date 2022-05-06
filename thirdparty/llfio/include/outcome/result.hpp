/* A very simple result type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (99 commits)
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

#ifndef OUTCOME_RESULT_HPP
#define OUTCOME_RESULT_HPP

#include "std_result.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
template <class R, class S = std::error_code, class NoValuePolicy = policy::default_policy<R, S, void>>  //
using result = std_result<R, S, NoValuePolicy>;

/*! AWAITING HUGO JSON CONVERSION TOOL 
type alias template <class R, class S = std::error_code> unchecked. Potential doc page: `unchecked<T, E = varies>`
*/
template <class R, class S = std::error_code> using unchecked = result<R, S, policy::all_narrow>;

/*! AWAITING HUGO JSON CONVERSION TOOL 
type alias template <class R, class S = std::error_code> checked. Potential doc page: `checked<T, E = varies>`
*/
template <class R, class S = std::error_code> using checked = result<R, S, policy::throw_bad_result_access<S, void>>;

OUTCOME_V2_NAMESPACE_END

#endif
