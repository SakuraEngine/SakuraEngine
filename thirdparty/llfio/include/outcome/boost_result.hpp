/* A very simple result type
(C) 2017-2020 Niall Douglas <http://www.nedproductions.biz/> (10 commits)
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

#ifndef OUTCOME_BOOST_RESULT_HPP
#define OUTCOME_BOOST_RESULT_HPP

#include "config.hpp"

#include "boost/system/system_error.hpp"
#include "boost/exception_ptr.hpp"
#include "boost/version.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
namespace policy
{
  namespace detail
  {
    /* Pass through `make_error_code` function for `boost::system::error_code`.
     */
    inline boost::system::error_code make_error_code(boost::system::error_code v) { return v; }

    /* Pass through `make_exception_ptr` function for `boost::exception_ptr`.
    */
    inline boost::exception_ptr make_exception_ptr(boost::exception_ptr v) { return v; }
  }  // namespace detail
}  // namespace policy
OUTCOME_V2_NAMESPACE_END

#include "std_result.hpp"


// ADL injection of outcome_throw_as_system_error_with_payload
namespace boost
{
  namespace system
  {
    inline void outcome_throw_as_system_error_with_payload(const error_code &error) { OUTCOME_THROW_EXCEPTION(system_error(error)); }
    namespace errc
    {
      OUTCOME_TEMPLATE(class Error)
      OUTCOME_TREQUIRES(OUTCOME_TPRED(is_error_code_enum<std::decay_t<Error>>::value || is_error_condition_enum<std::decay_t<Error>>::value))
      inline void outcome_throw_as_system_error_with_payload(Error &&error) { OUTCOME_THROW_EXCEPTION(system_error(make_error_code(error))); }
    }  // namespace errc
  }    // namespace system
}  // namespace boost

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  // Customise _set_error_is_errno
  template <class State> constexpr inline void _set_error_is_errno(State &state, const boost::system::error_code &error)
  {
    if(error.category() == boost::system::generic_category()
#ifndef _WIN32
       || error.category() == boost::system::system_category()
#endif
    )
    {
      state._status.set_have_error_is_errno(true);
    }
  }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const boost::system::error_condition &error)
  {
    if(error.category() == boost::system::generic_category()
#ifndef _WIN32
       || error.category() == boost::system::system_category()
#endif
    )
    {
      state._status.set_have_error_is_errno(true);
    }
  }
  template <class State> constexpr inline void _set_error_is_errno(State &state, const boost::system::errc::errc_t & /*unused*/)
  {
    state._status.set_have_error_is_errno(true);
  }

}  // namespace detail

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
namespace trait
{
  namespace detail
  {
    // Shortcut these for lower build impact
    template <> struct _is_error_code_available<boost::system::error_code>
    {
      static constexpr bool value = true;
      using type = boost::system::error_code;
    };
    template <> struct _is_exception_ptr_available<boost::exception_ptr>
    {
      static constexpr bool value = true;
      using type = boost::exception_ptr;
    };
  }  // namespace detail

  // boost::system::error_code is an error type
  template <> struct is_error_type<boost::system::error_code>
  {
    static constexpr bool value = true;
  };
  // boost::system::error_code::errc_t is an error type
  template <> struct is_error_type<boost::system::errc::errc_t>
  {
    static constexpr bool value = true;
  };
  // boost::exception_ptr is an error types
  template <> struct is_error_type<boost::exception_ptr>
  {
    static constexpr bool value = true;
  };
  // For boost::system::error_code, boost::system::is_error_condition_enum<> is the trait we want.
  template <class Enum> struct is_error_type_enum<boost::system::error_code, Enum>
  {
    static constexpr bool value = boost::system::is_error_condition_enum<Enum>::value;
  };

}  // namespace trait


/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class R, class S = boost::system::error_code, class NoValuePolicy = policy::default_policy<R, S, void>>  //
using boost_result = basic_result<R, S, NoValuePolicy>;

/*! AWAITING HUGO JSON CONVERSION TOOL
type alias template <class R, class S = boost::system::error_code> boost_unchecked. Potential doc page: `boost_unchecked<T, E = boost::system::error_code>`
*/
template <class R, class S = boost::system::error_code> using boost_unchecked = boost_result<R, S, policy::all_narrow>;

/*! AWAITING HUGO JSON CONVERSION TOOL
type alias template <class R, class S = boost::system::error_code> boost_checked. Potential doc page: `boost_checked<T, E = boost::system::error_code>`
*/
template <class R, class S = boost::system::error_code> using boost_checked = boost_result<R, S, policy::throw_bad_result_access<S, void>>;

OUTCOME_V2_NAMESPACE_END

#endif
