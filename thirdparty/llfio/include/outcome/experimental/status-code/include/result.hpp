/* A partial result based on std::variant and proposed std::error
(C) 2020 Niall Douglas <http://www.nedproductions.biz/> (11 commits)
File Created: Jan 2020

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

#ifndef SYSTEM_ERROR2_RESULT_HPP
#define SYSTEM_ERROR2_RESULT_HPP

#include "error.hpp"

#if __cplusplus >= 201703L || _HAS_CXX17
#if __has_include(<variant>)

#include <exception>
#include <variant>

SYSTEM_ERROR2_NAMESPACE_BEGIN

template <class T> inline constexpr std::in_place_type_t<T> in_place_type{};

template <class T> class result;

//! \brief A trait for detecting result types
template <class T> struct is_result : public std::false_type
{
};
template <class T> struct is_result<result<T>> : public std::true_type
{
};

/*! \brief Exception type representing the failure to retrieve an error.
 */
class bad_result_access : public std::exception
{
public:
  bad_result_access() = default;
  //! Return an explanatory string
  virtual const char *what() const noexcept override { return "bad result access"; }  // NOLINT
};

namespace detail
{
  struct void_
  {
  };
  template <class T> using devoid = std::conditional_t<std::is_void_v<T>, void_, T>;
}  // namespace detail

/*! \class result
\brief A imperfect `result<T>` type with its error type hardcoded to `error`, only available on C++ 17 or later.

Note that the proper `result<T>` type does not have the possibility of
valueless by exception state. This implementation is therefore imperfect.
*/
template <class T> class result : protected std::variant<SYSTEM_ERROR2_NAMESPACE::error, detail::devoid<T>>
{
  using _base = std::variant<SYSTEM_ERROR2_NAMESPACE::error, detail::devoid<T>>;
  static_assert(!std::is_reference_v<T>, "Type cannot be a reference");
  static_assert(!std::is_array_v<T>, "Type cannot be an array");
  static_assert(!std::is_same_v<T, SYSTEM_ERROR2_NAMESPACE::error>, "Type cannot be a std::error");
  // not success nor failure types

  struct _implicit_converting_constructor_tag
  {
  };
  struct _explicit_converting_constructor_tag
  {
  };
  struct _implicit_constructor_tag
  {
  };
  struct _implicit_in_place_value_constructor_tag
  {
  };
  struct _implicit_in_place_error_constructor_tag
  {
  };

public:
  //! The value type
  using value_type = T;
  //! The error type
  using error_type = SYSTEM_ERROR2_NAMESPACE::error;
  //! The value type, if it is available, else a usefully named unusable internal type
  using value_type_if_enabled = detail::devoid<T>;
  //! Used to rebind result types
  template <class U> using rebind = result<U>;

protected:
  constexpr void _check() const
  {
    if(_base::index() == 0)
    {
      std::get_if<0>(this)->throw_exception();
    }
  }
  constexpr
#ifdef _MSC_VER
  __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
        __attribute__((noreturn))
#endif
  void _ub()
  {
    assert(false);  // NOLINT
#if defined(__GNUC__) || defined(__clang__)
    __builtin_unreachable();
#elif defined(_MSC_VER)
    __assume(0);
#endif
  }

public:
  constexpr _base &_internal() noexcept { return *this; }
  constexpr const _base &_internal() const noexcept { return *this; }

  //! Default constructor is disabled
  result() = delete;
  //! Copy constructor
  result(const result &) = delete;
  //! Move constructor
  result(result &&) = default;
  //! Copy assignment
  result &operator=(const result &) = delete;
  //! Move assignment
  result &operator=(result &&) = default;
  //! Destructor
  ~result() = default;

  //! Implicit result converting move constructor
  SYSTEM_ERROR2_TEMPLATE(class U)
  SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(std::is_convertible_v<U, T>)) constexpr result(result<U> &&o, _implicit_converting_constructor_tag = {}) noexcept(std::is_nothrow_constructible_v<T, U>)
      : _base(std::move(o))
  {
  }
  //! Implicit result converting copy constructor
  SYSTEM_ERROR2_TEMPLATE(class U)
  SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(std::is_convertible_v<U, T>)) constexpr result(const result<U> &o, _implicit_converting_constructor_tag = {}) noexcept(std::is_nothrow_constructible_v<T, U>)
      : _base(o)
  {
  }
  //! Explicit result converting move constructor
  SYSTEM_ERROR2_TEMPLATE(class U)
  SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(std::is_constructible_v<T, U>)) constexpr explicit result(result<U> &&o, _explicit_converting_constructor_tag = {}) noexcept(std::is_nothrow_constructible_v<T, U>)
      : _base(std::move(o))
  {
  }
  //! Explicit result converting copy constructor
  SYSTEM_ERROR2_TEMPLATE(class U)
  SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(std::is_constructible_v<T, U>)) constexpr explicit result(const result<U> &o, _explicit_converting_constructor_tag = {}) noexcept(std::is_nothrow_constructible_v<T, U>)
      : _base(o)
  {
  }

  //! Anything which `std::variant<error, T>` will construct from, we shall implicitly construct from
  using _base::_base;

  //! Special case `in_place_type_t<void>`
  constexpr explicit result(std::in_place_type_t<void> /*unused*/) noexcept
      : _base(in_place_type<detail::void_>)
  {
  }

  //! Implicit in-place converting error constructor
  SYSTEM_ERROR2_TEMPLATE(class Arg1, class Arg2, class... Args, long = 5)                                                                                              //
  SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(!(std::is_constructible_v<value_type, Arg1, Arg2, Args...> && std::is_constructible_v<error_type, Arg1, Arg2, Args...>)  //
                                              &&std::is_constructible_v<error_type, Arg1, Arg2, Args...>))
  constexpr result(Arg1 &&arg1, Arg2 &&arg2, Args &&...args) noexcept(std::is_nothrow_constructible_v<error_type, Arg1, Arg2, Args...>)
      : _base(std::in_place_index<0>, std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...)
  {
  }

  //! Implicit in-place converting value constructor
  SYSTEM_ERROR2_TEMPLATE(class Arg1, class Arg2, class... Args, int = 5)                                                                                               //
  SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(!(std::is_constructible_v<value_type, Arg1, Arg2, Args...> && std::is_constructible_v<error_type, Arg1, Arg2, Args...>)  //
                                              &&std::is_constructible_v<value_type, Arg1, Arg2, Args...>))
  constexpr result(Arg1 &&arg1, Arg2 &&arg2, Args &&...args) noexcept(std::is_nothrow_constructible_v<value_type, Arg1, Arg2, Args...>)
      : _base(std::in_place_index<1>, std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Args>(args)...)
  {
  }

  //! Implicit construction from any type where an ADL discovered `make_status_code(T, Args ...)` returns a `status_code`.
  SYSTEM_ERROR2_TEMPLATE(class U, class... Args,                                                                            //
                         class MakeStatusCodeResult = typename detail::safe_get_make_status_code_result<U, Args...>::type)  // Safe ADL lookup of make_status_code(), returns void if not found
  SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(!std::is_same<typename std::decay<U>::type, result>::value                    // not copy/move of self
                                              && !std::is_same<typename std::decay<U>::type, value_type>::value             // not copy/move of value type
                                              && is_status_code<MakeStatusCodeResult>::value                                // ADL makes a status code
                                              && std::is_constructible<error_type, MakeStatusCodeResult>::value))           // ADLed status code is compatible
  constexpr result(U &&v, Args &&...args) noexcept(noexcept(make_status_code(std::declval<U>(), std::declval<Args>()...)))  // NOLINT
      : _base(std::in_place_index<0>, make_status_code(static_cast<U &&>(v), static_cast<Args &&>(args)...))
  {
  }

  //! Swap with another result
  constexpr void swap(result &o) noexcept(std::is_nothrow_swappable_v<_base>) { _base::swap(o); }

  //! Clone the result
  constexpr result clone() const { return has_value() ? result(value()) : result(error().clone()); }

  //! True if result has a value
  constexpr bool has_value() const noexcept { return _base::index() == 1; }
  //! True if result has a value
  explicit operator bool() const noexcept { return has_value(); }
  //! True if result has an error
  constexpr bool has_error() const noexcept { return _base::index() == 0; }

  //! Accesses the value if one exists, else calls `.error().throw_exception()`.
  constexpr value_type_if_enabled &value() &
  {
    _check();
    return std::get<1>(*this);
  }
  //! Accesses the value if one exists, else calls `.error().throw_exception()`.
  constexpr const value_type_if_enabled &value() const &
  {
    _check();
    return std::get<1>(*this);
  }
  //! Accesses the value if one exists, else calls `.error().throw_exception()`.
  constexpr value_type_if_enabled &&value() &&
  {
    _check();
    return std::get<1>(std::move(*this));
  }
  //! Accesses the value if one exists, else calls `.error().throw_exception()`.
  constexpr const value_type_if_enabled &&value() const &&
  {
    _check();
    return std::get<1>(std::move(*this));
  }

  //! Accesses the error if one exists, else throws `bad_result_access`.
  constexpr error_type &error() &
  {
    if(!has_error())
    {
#ifdef __cpp_exceptions
      throw bad_result_access();
#else
      abort();
#endif
    }
    return *std::get_if<0>(this);
  }
  //! Accesses the error if one exists, else throws `bad_result_access`.
  constexpr const error_type &error() const &
  {
    if(!has_error())
    {
#ifdef __cpp_exceptions
      throw bad_result_access();
#else
      abort();
#endif
    }
    return *std::get_if<0>(this);
  }
  //! Accesses the error if one exists, else throws `bad_result_access`.
  constexpr error_type &&error() &&
  {
    if(!has_error())
    {
#ifdef __cpp_exceptions
      throw bad_result_access();
#else
      abort();
#endif
    }
    return std::move(*std::get_if<0>(this));
  }
  //! Accesses the error if one exists, else throws `bad_result_access`.
  constexpr const error_type &&error() const &&
  {
    if(!has_error())
    {
#ifdef __cpp_exceptions
      throw bad_result_access();
#else
      abort();
#endif
    }
    return std::move(*std::get_if<0>(this));
  }

  //! Accesses the value, being UB if none exists
  constexpr value_type_if_enabled &assume_value() &noexcept
  {
    if(!has_value())
    {
      _ub();
    }
    return *std::get_if<1>(this);
  }
  //! Accesses the error, being UB if none exists
  constexpr const value_type_if_enabled &assume_value() const &noexcept
  {
    if(!has_value())
    {
      _ub();
    }
    return *std::get_if<1>(this);
  }
  //! Accesses the error, being UB if none exists
  constexpr value_type_if_enabled &&assume_value() &&noexcept
  {
    if(!has_value())
    {
      _ub();
    }
    return std::move(*std::get_if<1>(this));
  }
  //! Accesses the error, being UB if none exists
  constexpr const value_type_if_enabled &&assume_value() const &&noexcept
  {
    if(!has_value())
    {
      _ub();
    }
    return std::move(*std::get_if<1>(this));
  }

  //! Accesses the error, being UB if none exists
  constexpr error_type &assume_error() &noexcept
  {
    if(!has_error())
    {
      _ub();
    }
    return *std::get_if<0>(this);
  }
  //! Accesses the error, being UB if none exists
  constexpr const error_type &assume_error() const &noexcept
  {
    if(!has_error())
    {
      _ub();
    }
    return *std::get_if<0>(this);
  }
  //! Accesses the error, being UB if none exists
  constexpr error_type &&assume_error() &&noexcept
  {
    if(!has_error())
    {
      _ub();
    }
    return std::move(*std::get_if<0>(this));
  }
  //! Accesses the error, being UB if none exists
  constexpr const error_type &&assume_error() const &&noexcept
  {
    if(!has_error())
    {
      _ub();
    }
    return std::move(*std::get_if<0>(this));
  }
};

//! True if the two results compare equal.
template <class T, class U, typename = decltype(std::declval<T>() == std::declval<U>())> constexpr inline bool operator==(const result<T> &a, const result<U> &b) noexcept
{
  const auto &x = a._internal();
  return x == b;
}
//! True if the two results compare unequal.
template <class T, class U, typename = decltype(std::declval<T>() != std::declval<U>())> constexpr inline bool operator!=(const result<T> &a, const result<U> &b) noexcept
{
  const auto &x = a._internal();
  return x != b;
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
#endif
#endif
