/* Traits for Outcome
(C) 2018-2019 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
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

#ifndef OUTCOME_TRAIT_STD_ERROR_CODE_HPP
#define OUTCOME_TRAIT_STD_ERROR_CODE_HPP

#include "../config.hpp"

#include <system_error>

OUTCOME_V2_NAMESPACE_BEGIN

namespace detail
{
  // Customise _set_error_is_errno
  template <class State> constexpr inline void _set_error_is_errno(State &state, const std::error_code &error)
  {
    if(error.category() == std::generic_category()
#ifndef _WIN32
       || error.category() == std::system_category()
#endif
    )
    {
      state._status.set_have_error_is_errno(true);
    }
  }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const std::error_condition &error)
  {
    if(error.category() == std::generic_category()
#ifndef _WIN32
       || error.category() == std::system_category()
#endif
    )
    {
      state._status.set_have_error_is_errno(true);
    }
  }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const std::errc & /*unused*/) {
      state._status.set_have_error_is_errno(true);
   }

}  // namespace detail

namespace policy
{
  namespace detail
  {
    /* Pass through `make_error_code` function for `std::error_code`.
     */
    inline std::error_code make_error_code(std::error_code v) { return v; }

    // Try ADL, if not use fall backs above
    template <class T> constexpr inline decltype(auto) error_code(T &&v) { return make_error_code(std::forward<T>(v)); }

    struct std_enum_overload_tag
    {
    };
  }  // namespace detail

  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  template <class T> constexpr inline decltype(auto) error_code(T &&v) { return detail::error_code(std::forward<T>(v)); }

  /*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
  // inline void outcome_throw_as_system_error_with_payload(...) = delete;  // To use the error_code_throw_as_system_error policy with a custom Error type, you must define a outcome_throw_as_system_error_with_payload() free function to say how to handle the payload
  inline void outcome_throw_as_system_error_with_payload(const std::error_code &error) { OUTCOME_THROW_EXCEPTION(std::system_error(error)); }  // NOLINT
  OUTCOME_TEMPLATE(class Error)
  OUTCOME_TREQUIRES(OUTCOME_TPRED(std::is_error_code_enum<std::decay_t<Error>>::value || std::is_error_condition_enum<std::decay_t<Error>>::value))
  inline void outcome_throw_as_system_error_with_payload(Error &&error, detail::std_enum_overload_tag /*unused*/ = detail::std_enum_overload_tag()) { OUTCOME_THROW_EXCEPTION(std::system_error(make_error_code(error))); }  // NOLINT
}  // namespace policy

namespace trait
{
  namespace detail
  {
    template <> struct _is_error_code_available<std::error_code>
    {
      // Shortcut this for lower build impact
      static constexpr bool value = true;
      using type = std::error_code;
    };
  }  // namespace detail

  // std::error_code is an error type
  template <> struct is_error_type<std::error_code>
  {
    static constexpr bool value = true;
  };
  // For std::error_code, std::is_error_condition_enum<> is the trait we want.
  template <class Enum> struct is_error_type_enum<std::error_code, Enum>
  {
    static constexpr bool value = std::is_error_condition_enum<Enum>::value;
  };

}  // namespace trait

OUTCOME_V2_NAMESPACE_END

#endif
