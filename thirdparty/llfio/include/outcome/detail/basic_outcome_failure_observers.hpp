/* Failure observers for outcome type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
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

#ifndef OUTCOME_BASIC_OUTCOME_FAILURE_OBSERVERS_HPP
#define OUTCOME_BASIC_OUTCOME_FAILURE_OBSERVERS_HPP

#include "basic_result_storage.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  namespace adl
  {
    struct search_detail_adl
    {
    };
    // Do NOT use template requirements here!
    template <class S, typename = decltype(basic_outcome_failure_exception_from_error(std::declval<S>()))>
    inline auto _delayed_lookup_basic_outcome_failure_exception_from_error(const S &ec, search_detail_adl /*unused*/)
    {
      // ADL discovered
      return basic_outcome_failure_exception_from_error(ec);
    }
  }                                        // namespace adl
#if defined(_MSC_VER) && _MSC_VER <= 1923  // VS2019
  // VS2017 and VS2019 with /permissive- chokes on the correct form due to over eager early instantiation.
  template <class S, class P> inline void _delayed_lookup_basic_outcome_failure_exception_from_error(...) { static_assert(sizeof(S) == 0, "No specialisation for these error and exception types available!"); }
#else
  template <class S, class P> inline void _delayed_lookup_basic_outcome_failure_exception_from_error(...) = delete;  // NOLINT No specialisation for these error and exception types available!
#endif

  template <class exception_type> inline exception_type current_exception_or_fatal(std::exception_ptr e) { std::rethrow_exception(e); }
  template <> inline std::exception_ptr current_exception_or_fatal<std::exception_ptr>(std::exception_ptr e) { return e; }

  template <class Base, class R, class S, class P, class NoValuePolicy> class basic_outcome_failure_observers : public Base
  {
  public:
    using exception_type = P;
    using Base::Base;

    exception_type failure() const noexcept
    {
#ifdef __cpp_exceptions
      try
#endif
      {
        if(this->_state._status.have_exception())
        {
          return this->assume_exception();
        }
        if(this->_state._status.have_error())
        {
          return _delayed_lookup_basic_outcome_failure_exception_from_error(this->assume_error(), adl::search_detail_adl());
        }
        return exception_type();
      }
#ifdef __cpp_exceptions
      catch(...)
      {
        // Return the failure if exception_type is std::exception_ptr,
        // otherwise terminate same as throwing an exception inside noexcept
        return current_exception_or_fatal<exception_type>(std::current_exception());
      }
#endif
    }
  };

}  // namespace detail

OUTCOME_V2_NAMESPACE_END

#endif
