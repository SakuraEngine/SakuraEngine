/* Proposed SG14 status_code
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: June 2018


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

#ifndef SYSTEM_ERROR2_SYSTEM_CODE_FROM_EXCEPTION_HPP
#define SYSTEM_ERROR2_SYSTEM_CODE_FROM_EXCEPTION_HPP

#include "system_code.hpp"

#include <exception>     // for exception_ptr
#include <stdexcept>     // for the exception types
#include <system_error>  // for std::system_error

SYSTEM_ERROR2_NAMESPACE_BEGIN

/*! A utility function which returns the closest matching system_code to a supplied
exception ptr.
*/
inline system_code system_code_from_exception(std::exception_ptr &&ep = std::current_exception(), system_code not_matched = generic_code(errc::resource_unavailable_try_again)) noexcept
{
  if(!ep)
  {
    return generic_code(errc::success);
  }
  try
  {
    std::rethrow_exception(ep);
  }
  catch(const std::invalid_argument & /*unused*/)
  {
    ep = std::exception_ptr();
    return generic_code(errc::invalid_argument);
  }
  catch(const std::domain_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return generic_code(errc::argument_out_of_domain);
  }
  catch(const std::length_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return generic_code(errc::argument_list_too_long);
  }
  catch(const std::out_of_range & /*unused*/)
  {
    ep = std::exception_ptr();
    return generic_code(errc::result_out_of_range);
  }
  catch(const std::logic_error & /*unused*/) /* base class for this group */
  {
    ep = std::exception_ptr();
    return generic_code(errc::invalid_argument);
  }
  catch(const std::system_error &e) /* also catches ios::failure */
  {
    ep = std::exception_ptr();
    if(e.code().category() == std::generic_category())
    {
      return generic_code(static_cast<errc>(static_cast<int>(e.code().value())));
    }
    if(e.code().category() == std::system_category())
    {
#ifdef _WIN32
      return win32_code(e.code().value());
#else
      return posix_code(e.code().value());
#endif
    }
    // Don't know this error code category, can't wrap it into std_error_code
    // as its payload won't fit into system_code, so fall through.
  }
  catch(const std::overflow_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return generic_code(errc::value_too_large);
  }
  catch(const std::range_error & /*unused*/)
  {
    ep = std::exception_ptr();
    return generic_code(errc::result_out_of_range);
  }
  catch(const std::runtime_error & /*unused*/) /* base class for this group */
  {
    ep = std::exception_ptr();
    return generic_code(errc::resource_unavailable_try_again);
  }
  catch(const std::bad_alloc & /*unused*/)
  {
    ep = std::exception_ptr();
    return generic_code(errc::not_enough_memory);
  }
  catch(...)
  {
  }
  return not_matched;
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
