/* A less simple result type
(C) 2018-2019 Niall Douglas <http://www.nedproductions.biz/> (17 commits)
File Created: Apr 2018


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

#ifndef OUTCOME_EXPERIMENTAL_STATUS_OUTCOME_HPP
#define OUTCOME_EXPERIMENTAL_STATUS_OUTCOME_HPP

#include "../basic_outcome.hpp"

#include "../detail/trait_std_exception.hpp"
#include "status_result.hpp"

// Boost.Outcome #include "boost/exception_ptr.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN
template <class DomainType> inline std::exception_ptr basic_outcome_failure_exception_from_error(const status_code<DomainType> &sc)
{
  (void) sc;
#ifdef __cpp_exceptions
  try
  {
    sc.throw_exception();
  }
  catch(...)
  {
    return std::current_exception();
  }
#endif
  return {};
}
SYSTEM_ERROR2_NAMESPACE_END

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace experimental
{
  namespace policy
  {
    template <class T, class EC, class E>
    using default_status_outcome_policy = std::conditional_t<  //
    std::is_void<EC>::value && std::is_void<E>::value,         //
    OUTCOME_V2_NAMESPACE::policy::terminate,                   //
    std::conditional_t<(is_status_code<EC>::value || is_errored_status_code<EC>::value) &&
                       (std::is_void<E>::value || OUTCOME_V2_NAMESPACE::trait::is_exception_ptr_available<E>::value),  //
                       status_code_throw<T, EC, E>,                                                                    //
                       OUTCOME_V2_NAMESPACE::policy::fail_to_compile_observers                                         //
                       >>;
  }  // namespace policy

  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class R, class S = errored_status_code<erased<typename system_code::value_type>>, class P = std::exception_ptr,
            class NoValuePolicy = policy::default_status_outcome_policy<R, S, P>>  //
  using status_outcome = basic_outcome<R, S, P, NoValuePolicy>;

  namespace policy
  {
    template <class T, class DomainType, class E> struct status_code_throw<T, status_code<DomainType>, E> : base
    {
      using _base = base;
      template <class Impl> static constexpr void wide_value_check(Impl &&self)
      {
        if(!base::_has_value(static_cast<Impl &&>(self)))
        {
          if(base::_has_exception(static_cast<Impl &&>(self)))
          {
            OUTCOME_V2_NAMESPACE::policy::detail::_rethrow_exception<trait::is_exception_ptr_available<E>::value>(
            base::_exception<T, status_code<DomainType>, E, status_code_throw>(static_cast<Impl &&>(self)));  // NOLINT
          }
          if(base::_has_error(static_cast<Impl &&>(self)))
          {
#ifdef __cpp_exceptions
            base::_error(static_cast<Impl &&>(self)).throw_exception();
#else
            OUTCOME_THROW_EXCEPTION("wide value check failed");
#endif
          }
        }
      }
      template <class Impl> static constexpr void wide_error_check(Impl &&self) { _base::narrow_error_check(static_cast<Impl &&>(self)); }
      template <class Impl> static constexpr void wide_exception_check(Impl &&self) { _base::narrow_exception_check(static_cast<Impl &&>(self)); }
    };
    template <class T, class DomainType, class E>
    struct status_code_throw<T, errored_status_code<DomainType>, E> : status_code_throw<T, status_code<DomainType>, E>
    {
      status_code_throw() = default;
      using status_code_throw<T, status_code<DomainType>, E>::status_code_throw;
    };
  }  // namespace policy

}  // namespace experimental

OUTCOME_V2_NAMESPACE_END

#endif
