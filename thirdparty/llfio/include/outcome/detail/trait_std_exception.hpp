/* Traits for Outcome
(C) 2018-2019 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: March 2018


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

#ifndef OUTCOME_TRAIT_STD_EXCEPTION_HPP
#define OUTCOME_TRAIT_STD_EXCEPTION_HPP

#include "../config.hpp"

#include <exception>

OUTCOME_V2_NAMESPACE_BEGIN

namespace policy
{
  namespace detail
  {
    /* Pass through `make_exception_ptr` function for `std::exception_ptr`.
    */
    inline std::exception_ptr make_exception_ptr(std::exception_ptr v) { return v; }

    // Try ADL, if not use fall backs above
    template <class T> constexpr inline decltype(auto) exception_ptr(T &&v) { return make_exception_ptr(std::forward<T>(v)); }
  }  // namespace detail

  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  template <class T> constexpr inline decltype(auto) exception_ptr(T &&v) { return detail::exception_ptr(std::forward<T>(v)); }

  namespace detail
  {
    template <bool has_error_payload> struct _rethrow_exception
    {
      template <class Exception> explicit _rethrow_exception(Exception && /*unused*/)  // NOLINT
      {
      }
    };
    template <> struct _rethrow_exception<true>
    {
      template <class Exception> explicit _rethrow_exception(Exception &&excpt)  // NOLINT
      {
        // ADL
        rethrow_exception(policy::exception_ptr(std::forward<Exception>(excpt)));
      }
    };
  }  // namespace detail
}  // namespace policy

namespace trait
{
  namespace detail
  {
    // Shortcut this for lower build impact
    template <> struct _is_exception_ptr_available<std::exception_ptr>
    {
      static constexpr bool value = true;
      using type = std::exception_ptr;
    };
  }  // namespace detail

  // std::exception_ptr is an error type
  template <> struct is_error_type<std::exception_ptr>
  {
    static constexpr bool value = true;
  };

}  // namespace trait

OUTCOME_V2_NAMESPACE_END

#endif
