/* Tries to convert an exception ptr into its equivalent error code
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (11 commits)
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

#ifndef OUTCOME_UTILS_HPP
#define OUTCOME_UTILS_HPP

#include "config.hpp"

#include <exception>
#include <string>
#include <system_error>

OUTCOME_V2_NAMESPACE_BEGIN

#ifdef __cpp_exceptions
/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
inline std::error_code error_from_exception(std::exception_ptr &&ep = std::current_exception(), std::error_code not_matched = std::make_error_code(std::errc::resource_unavailable_try_again)) noexcept
{
  if(!ep)
  {
    return {};
  }
  try
  {
    std::rethrow_exception(ep);
  }
  catch(const std::invalid_argument & /*unused*/)
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::invalid_argument);
  }
  catch(const std::domain_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::argument_out_of_domain);
  }
  catch(const std::length_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::argument_list_too_long);
  }
  catch(const std::out_of_range & /*unused*/)
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::result_out_of_range);
  }
  catch(const std::logic_error & /*unused*/) /* base class for this group */
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::invalid_argument);
  }
  catch(const std::system_error &e) /* also catches ios::failure */
  {
    ep = std::exception_ptr();
    return e.code();
  }
  catch(const std::overflow_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::value_too_large);
  }
  catch(const std::range_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::result_out_of_range);
  }
  catch(const std::runtime_error & /*unused*/) /* base class for this group */
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::resource_unavailable_try_again);
  }
  catch(const std::bad_alloc & /*unused*/)
  {
    ep = std::exception_ptr();
    return std::make_error_code(std::errc::not_enough_memory);
  }
  catch(...)
  {
  }
  return not_matched;
}

/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
inline void try_throw_std_exception_from_error(std::error_code ec, const std::string &msg = std::string{})
{
  if(!ec || (ec.category() != std::generic_category()
#ifndef _WIN32
             && ec.category() != std::system_category()
#endif
             ))
  {
    return;
  }
  switch(ec.value())
  {
  case EINVAL:
    throw msg.empty() ? std::invalid_argument("invalid argument") : std::invalid_argument(msg);
  case EDOM:
    throw msg.empty() ? std::domain_error("domain error") : std::domain_error(msg);
  case E2BIG:
    throw msg.empty() ? std::length_error("length error") : std::length_error(msg);
  case ERANGE:
    throw msg.empty() ? std::out_of_range("out of range") : std::out_of_range(msg);
  case EOVERFLOW:
    throw msg.empty() ? std::overflow_error("overflow error") : std::overflow_error(msg);
  case ENOMEM:
    throw std::bad_alloc();
  }
}
#endif

OUTCOME_V2_NAMESPACE_END

#endif
