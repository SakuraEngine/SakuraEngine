/* Exception types throwable
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (9 commits)
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

#ifndef OUTCOME_BAD_ACCESS_HPP
#define OUTCOME_BAD_ACCESS_HPP

#include "config.hpp"

#include <stdexcept>

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL 
type definition  bad_result_access. Potential doc page: `bad_result_access`
*/
class OUTCOME_SYMBOL_VISIBLE bad_result_access : public std::logic_error
{
public:
  explicit bad_result_access(const char *what)
      : std::logic_error(what)
  {
  }
};

/*! AWAITING HUGO JSON CONVERSION TOOL 
type definition template <class S> bad_result_access_with. Potential doc page: `bad_result_access_with<EC>`
*/
template <class S> class OUTCOME_SYMBOL_VISIBLE bad_result_access_with : public bad_result_access
{
  S _error;

public:
  explicit bad_result_access_with(S v)
      : bad_result_access("no value")
      , _error(std::move(v))
  {
  }
  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  const S &error() const & { return _error; }
  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  S &error() & { return _error; }
  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  const S &&error() const && { return _error; }
  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  S &&error() && { return _error; }
};

/*! AWAITING HUGO JSON CONVERSION TOOL 
type definition  bad_outcome_access. Potential doc page: `bad_outcome_access`
*/
class OUTCOME_SYMBOL_VISIBLE bad_outcome_access : public std::logic_error
{
public:
  explicit bad_outcome_access(const char *what)
      : std::logic_error(what)
  {
  }
};

OUTCOME_V2_NAMESPACE_END

#endif
