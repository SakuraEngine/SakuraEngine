/* Proposed SG14 status_code
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Feb 2018


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

#ifndef SYSTEM_ERROR2_IOSTREAM_SUPPORT_HPP
#define SYSTEM_ERROR2_IOSTREAM_SUPPORT_HPP

#include "error.hpp"

#include <ostream>

SYSTEM_ERROR2_NAMESPACE_BEGIN

/*! Print the status code to a `std::ostream &`.
Requires that `DomainType::value_type` implements an `operator<<` overload for `std::ostream`.
*/
SYSTEM_ERROR2_TEMPLATE(class DomainType)  //
SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(std::is_same<std::ostream, typename std::decay<decltype(std::declval<std::ostream>() << std::declval<typename status_code<DomainType>::value_type>())>::type>::value)) inline std::ostream &operator<<(std::ostream &s, const status_code<DomainType> &v)
{
  if(v.empty())
  {
    return s << "(empty)";
  }
  return s << v.domain().name().c_str() << ": " << v.value();
}

/*! Print a status code domain's `string_ref` to a `std::ostream &`.
 */
inline std::ostream &operator<<(std::ostream &s, const status_code_domain::string_ref &v)
{
  return s << v.c_str();
}

/*! Print the erased status code to a `std::ostream &`.
 */
template <class ErasedType> inline std::ostream &operator<<(std::ostream &s, const status_code<erased<ErasedType>> &v)
{
  if(v.empty())
  {
    return s << "(empty)";
  }
  return s << v.domain().name() << ": " << v.message();
}

/*! Print the generic code to a `std::ostream &`.
 */
inline std::ostream &operator<<(std::ostream &s, const generic_code &v)
{
  if(v.empty())
  {
    return s << "(empty)";
  }
  return s << v.domain().name() << ": " << v.message();
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
