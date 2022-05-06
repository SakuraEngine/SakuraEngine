/* scope support
(C) 2019 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Apr 2019


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

#ifndef QUICKCPPLIB_SCOPE_HPP
#define QUICKCPPLIB_SCOPE_HPP

#include "config.hpp"

#ifndef QUICKCPPLIB_USE_STD_SCOPE
#if(__cplusplus >= 202000 || _HAS_CXX20) && __has_include(<scope>)
#define QUICKCPPLIB_USE_STD_SCOPE 1
#else
#define QUICKCPPLIB_USE_STD_SCOPE 0
#endif
#endif

#if QUICKCPPLIB_USE_STD_SCOPE
#include <scope>
#else
#include <type_traits>
#include <utility>
#endif
#include <exception>

QUICKCPPLIB_NAMESPACE_BEGIN
namespace scope
{
#if QUICKCPPLIB_USE_STD_SCOPE
  template <class T> using scope_exit = std::scope_exit<T>;
  template <class T> using scope_fail = std::scope_fail<T>;
  template <class T> using scope_success = std::scope_success<T>;
#else
  namespace detail
  {
    template <class T, bool v = noexcept(std::declval<T>()())> constexpr inline bool _is_nothrow_invocable(int) noexcept { return v; }
    template <class T> constexpr inline bool _is_nothrow_invocable(...) noexcept { return false; }
    template <class T> constexpr inline bool is_nothrow_invocable() noexcept { return _is_nothrow_invocable<typename std::decay<T>::type>(5); }

    enum class scope_impl_kind
    {
      exit,
      fail,
      success
    };
    template <class EF, scope_impl_kind kind> class scope_impl
    {
      EF _f;
      bool _released{false};
#if __cplusplus >= 201700 || _HAS_CXX17
      int _uncaught_exceptions;
#endif

    public:
      scope_impl() = delete;
      scope_impl(const scope_impl &) = delete;
      scope_impl &operator=(const scope_impl &) = delete;
      scope_impl &operator=(scope_impl &&) = delete;

      constexpr scope_impl(scope_impl &&o) noexcept(std::is_nothrow_move_constructible<EF>::value)
          : _f(static_cast<EF &&>(o._f))
          , _released(o._released)
#if __cplusplus >= 201700 || _HAS_CXX17
          , _uncaught_exceptions(o._uncaught_exceptions)
#endif
      {
        o._released = true;
      }

      template <class T, typename = decltype(std::declval<T>()()),
                typename std::enable_if<!std::is_same<scope_impl, typename std::decay<T>::type>::value      //
                                        && std::is_constructible<EF, T>::value                              //
                                        && (scope_impl_kind::success == kind || is_nothrow_invocable<T>())  //
                                        ,
                                        bool>::type = true>
      explicit constexpr scope_impl(T &&f) noexcept(std::is_nothrow_constructible<EF, T>::value)
          : _f(static_cast<T &&>(f))
#if __cplusplus >= 201700 || _HAS_CXX17
          , _uncaught_exceptions(std::uncaught_exceptions())
#endif
      {
      }
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)  // conditional expression is constant
#endif
      ~scope_impl()
      {
        if(!_released)
        {
          if(scope_impl_kind::exit == kind)
          {
            _f();
            return;
          }
#if __cplusplus >= 201700 || _HAS_CXX17
          bool unwind_is_due_to_throw = (std::uncaught_exceptions() > _uncaught_exceptions);
#else
          bool unwind_is_due_to_throw = std::uncaught_exception();
#endif
          if(scope_impl_kind::fail == kind && unwind_is_due_to_throw)
          {
            _f();
            return;
          }
          if(scope_impl_kind::success == kind && !unwind_is_due_to_throw)
          {
            _f();
            return;
          }
        }
      }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
      constexpr void release() noexcept { _released = true; }
    };
  }  // namespace detail

  template <class T> using scope_exit = detail::scope_impl<T, detail::scope_impl_kind::exit>;
  template <class T> using scope_fail = detail::scope_impl<T, detail::scope_impl_kind::fail>;
  template <class T> using scope_success = detail::scope_impl<T, detail::scope_impl_kind::success>;

  template <class T  //
#ifndef _MSC_VER
            ,
            typename = decltype(std::declval<T>()()),                               // MSVC chokes on these constraints here yet is
            typename std::enable_if<detail::is_nothrow_invocable<T>(), bool>::type  // perfectly fine with them on the
                                                                                    // constructor above :(
            = true
#endif
            >
  constexpr inline auto make_scope_exit(T &&v)
  {
    return scope_exit<typename std::decay<T>::type>(static_cast<T &&>(v));
  }
  template <class T  //
#ifndef _MSC_VER
            ,
            typename = decltype(std::declval<T>()()),                               //
            typename std::enable_if<detail::is_nothrow_invocable<T>(), bool>::type  //
            = true
#endif
            >
  constexpr inline auto make_scope_fail(T &&v)
  {
    return scope_fail<typename std::decay<T>::type>(static_cast<T &&>(v));
  }
  template <class T  //
#ifndef _MSC_VER
            ,
            typename = decltype(std::declval<T>()())
#endif
            >
  constexpr inline auto make_scope_success(T &&v)
  {
    return scope_success<typename std::decay<T>::type>(static_cast<T &&>(v));
  }
#endif

}  // namespace scope
QUICKCPPLIB_NAMESPACE_END

#endif
