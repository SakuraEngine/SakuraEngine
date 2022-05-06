/* Type sugar for success and failure
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (25 commits)
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

#ifndef OUTCOME_SUCCESS_FAILURE_HPP
#define OUTCOME_SUCCESS_FAILURE_HPP

#include "config.hpp"

OUTCOME_V2_NAMESPACE_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL
type definition template <class T> success_type. Potential doc page: `success_type<T>`
*/
template <class T> struct OUTCOME_NODISCARD success_type
{
  using value_type = T;

private:
  value_type _value;
  uint16_t _spare_storage{0};

public:
  success_type() = default;
  success_type(const success_type &) = default;
  success_type(success_type &&) = default;  // NOLINT
  success_type &operator=(const success_type &) = default;
  success_type &operator=(success_type &&) = default;  // NOLINT
  ~success_type() = default;
  OUTCOME_TEMPLATE(class U)
  OUTCOME_TREQUIRES(OUTCOME_TPRED(!std::is_same<success_type, std::decay_t<U>>::value))
  constexpr explicit success_type(U &&v, uint16_t spare_storage = 0)
      : _value(static_cast<U &&>(v))  // NOLINT
      , _spare_storage(spare_storage)
  {
  }

  constexpr value_type &value() & { return _value; }
  constexpr const value_type &value() const & { return _value; }
  constexpr value_type &&value() && { return static_cast<value_type &&>(_value); }
  constexpr const value_type &&value() const && { return static_cast<value_type &&>(_value); }

  constexpr uint16_t spare_storage() const { return _spare_storage; }
};
template <> struct OUTCOME_NODISCARD success_type<void>
{
  using value_type = void;

  constexpr uint16_t spare_storage() const { return 0; }
};
/*! Returns type sugar for implicitly constructing a `basic_result<T>` with a successful state,
default constructing `T` if necessary.
*/
inline constexpr success_type<void> success() noexcept
{
  return success_type<void>{};
}
/*! Returns type sugar for implicitly constructing a `basic_result<T>` with a successful state.
\effects Copies or moves the successful state supplied into the returned type sugar.
*/
template <class T> inline constexpr success_type<std::decay_t<T>> success(T &&v, uint16_t spare_storage = 0)
{
  return success_type<std::decay_t<T>>{static_cast<T &&>(v), spare_storage};
}

/*! AWAITING HUGO JSON CONVERSION TOOL
type definition template <class EC, class E = void> failure_type. Potential doc page: `failure_type<EC, EP = void>`
*/
template <class EC, class E = void> struct OUTCOME_NODISCARD failure_type
{
  using error_type = EC;
  using exception_type = E;

private:
  error_type _error;
  exception_type _exception;
  bool _have_error{false}, _have_exception{false};
  uint16_t _spare_storage{0};

  struct error_init_tag
  {
  };
  struct exception_init_tag
  {
  };

public:
  failure_type() = default;
  failure_type(const failure_type &) = default;
  failure_type(failure_type &&) = default;  // NOLINT
  failure_type &operator=(const failure_type &) = default;
  failure_type &operator=(failure_type &&) = default;  // NOLINT
  ~failure_type() = default;
  template <class U, class V>
  constexpr explicit failure_type(U &&u, V &&v, uint16_t spare_storage = 0)
      : _error(static_cast<U &&>(u))
      , _exception(static_cast<V &&>(v))
      , _have_error(true)
      , _have_exception(true)
      , _spare_storage(spare_storage)
  {
  }
  template <class U>
  constexpr explicit failure_type(in_place_type_t<error_type> /*unused*/, U &&u, uint16_t spare_storage = 0, error_init_tag /*unused*/ = error_init_tag())
      : _error(static_cast<U &&>(u))
      , _exception()
      , _have_error(true)
      , _spare_storage(spare_storage)
  {
  }
  template <class U>
  constexpr explicit failure_type(in_place_type_t<exception_type> /*unused*/, U &&u, uint16_t spare_storage = 0,
                                  exception_init_tag /*unused*/ = exception_init_tag())
      : _error()
      , _exception(static_cast<U &&>(u))
      , _have_exception(true)
      , _spare_storage(spare_storage)
  {
  }

  constexpr bool has_error() const { return _have_error; }
  constexpr bool has_exception() const { return _have_exception; }

  constexpr error_type &error() & { return _error; }
  constexpr const error_type &error() const & { return _error; }
  constexpr error_type &&error() && { return static_cast<error_type &&>(_error); }
  constexpr const error_type &&error() const && { return static_cast<error_type &&>(_error); }

  constexpr exception_type &exception() & { return _exception; }
  constexpr const exception_type &exception() const & { return _exception; }
  constexpr exception_type &&exception() && { return static_cast<exception_type &&>(_exception); }
  constexpr const exception_type &&exception() const && { return static_cast<exception_type &&>(_exception); }

  constexpr uint16_t spare_storage() const { return _spare_storage; }
};
template <class EC> struct OUTCOME_NODISCARD failure_type<EC, void>
{
  using error_type = EC;
  using exception_type = void;

private:
  error_type _error;
  uint16_t _spare_storage{0};

public:
  failure_type() = default;
  failure_type(const failure_type &) = default;
  failure_type(failure_type &&) = default;  // NOLINT
  failure_type &operator=(const failure_type &) = default;
  failure_type &operator=(failure_type &&) = default;  // NOLINT
  ~failure_type() = default;
  OUTCOME_TEMPLATE(class U)
  OUTCOME_TREQUIRES(OUTCOME_TPRED(!std::is_same<failure_type, std::decay_t<U>>::value))
  constexpr explicit failure_type(U &&u, uint16_t spare_storage = 0)
      : _error(static_cast<U &&>(u))  // NOLINT
      , _spare_storage(spare_storage)
  {
  }

  constexpr error_type &error() & { return _error; }
  constexpr const error_type &error() const & { return _error; }
  constexpr error_type &&error() && { return static_cast<error_type &&>(_error); }
  constexpr const error_type &&error() const && { return static_cast<error_type &&>(_error); }

  constexpr uint16_t spare_storage() const { return _spare_storage; }
};
template <class E> struct OUTCOME_NODISCARD failure_type<void, E>
{
  using error_type = void;
  using exception_type = E;

private:
  exception_type _exception;
  uint16_t _spare_storage{0};

public:
  failure_type() = default;
  failure_type(const failure_type &) = default;
  failure_type(failure_type &&) = default;  // NOLINT
  failure_type &operator=(const failure_type &) = default;
  failure_type &operator=(failure_type &&) = default;  // NOLINT
  ~failure_type() = default;
  OUTCOME_TEMPLATE(class V)
  OUTCOME_TREQUIRES(OUTCOME_TPRED(!std::is_same<failure_type, std::decay_t<V>>::value))
  constexpr explicit failure_type(V &&v, uint16_t spare_storage = 0)
      : _exception(static_cast<V &&>(v))  // NOLINT
      , _spare_storage(spare_storage)
  {
  }

  constexpr exception_type &exception() & { return _exception; }
  constexpr const exception_type &exception() const & { return _exception; }
  constexpr exception_type &&exception() && { return static_cast<exception_type &&>(_exception); }
  constexpr const exception_type &&exception() const && { return static_cast<exception_type &&>(_exception); }

  constexpr uint16_t spare_storage() const { return _spare_storage; }
};
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class EC> inline constexpr failure_type<std::decay_t<EC>> failure(EC &&v, uint16_t spare_storage = 0)
{
  return failure_type<std::decay_t<EC>>{static_cast<EC &&>(v), spare_storage};
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class EC, class E> inline constexpr failure_type<std::decay_t<EC>, std::decay_t<E>> failure(EC &&v, E &&w, uint16_t spare_storage = 0)
{
  return failure_type<std::decay_t<EC>, std::decay_t<E>>{static_cast<EC &&>(v), static_cast<E &&>(w), spare_storage};
}

namespace detail
{
  template <class T> struct is_success_type
  {
    static constexpr bool value = false;
  };
  template <class T> struct is_success_type<success_type<T>>
  {
    static constexpr bool value = true;
  };
  template <class T> struct is_failure_type
  {
    static constexpr bool value = false;
  };
  template <class EC, class E> struct is_failure_type<failure_type<EC, E>>
  {
    static constexpr bool value = true;
  };
}  // namespace detail

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class T> static constexpr bool is_success_type = detail::is_success_type<std::decay_t<T>>::value;

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class T> static constexpr bool is_failure_type = detail::is_failure_type<std::decay_t<T>>::value;

OUTCOME_V2_NAMESPACE_END

#endif
