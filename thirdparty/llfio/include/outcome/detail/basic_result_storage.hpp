/* Storage for a very simple basic_result type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
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

#ifndef OUTCOME_BASIC_RESULT_STORAGE_HPP
#define OUTCOME_BASIC_RESULT_STORAGE_HPP

#include "../success_failure.hpp"
#include "../trait.hpp"
#include "value_storage.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  template <class R, class EC, class NoValuePolicy> class basic_result_storage;
}  // namespace detail

namespace hooks
{
  template <class R, class S, class NoValuePolicy> constexpr inline uint16_t spare_storage(const detail::basic_result_storage<R, S, NoValuePolicy> *r) noexcept;
  template <class R, class S, class NoValuePolicy>
  constexpr inline void set_spare_storage(detail::basic_result_storage<R, S, NoValuePolicy> *r, uint16_t v) noexcept;
}  // namespace hooks

namespace policy
{
  struct base;
}  // namespace policy

namespace detail
{
  template <class R, class EC, class NoValuePolicy>  //
  class basic_result_storage
  {
    static_assert(trait::type_can_be_used_in_basic_result<R>, "The type R cannot be used in a basic_result");
    static_assert(trait::type_can_be_used_in_basic_result<EC>, "The type S cannot be used in a basic_result");

    friend struct policy::base;
    template <class T, class U, class V>  //
    friend class basic_result_storage;
    template <class T, class U, class V> friend class basic_result_final;
    template <class T, class U, class V>
    friend constexpr inline uint16_t hooks::spare_storage(const detail::basic_result_storage<T, U, V> *r) noexcept;  // NOLINT
    template <class T, class U, class V>
    friend constexpr inline void hooks::set_spare_storage(detail::basic_result_storage<T, U, V> *r, uint16_t v) noexcept;  // NOLINT

    struct disable_in_place_value_type
    {
    };
    struct disable_in_place_error_type
    {
    };

  protected:
    using _value_type = std::conditional_t<std::is_same<R, EC>::value, disable_in_place_value_type, R>;
    using _error_type = std::conditional_t<std::is_same<R, EC>::value, disable_in_place_error_type, EC>;

    using _state_type = value_storage_select_impl<_value_type, _error_type>;

#ifdef STANDARDESE_IS_IN_THE_HOUSE
    value_storage_trivial<_value_type, _error_type> _state;
#else
    _state_type _state;
#endif

  public:
    // Used by iostream support to access state
    _state_type &_iostreams_state() { return _state; }
    const _state_type &_iostreams_state() const { return _state; }

  protected:
    basic_result_storage() = default;
    basic_result_storage(const basic_result_storage &) = default;             // NOLINT
    basic_result_storage(basic_result_storage &&) = default;                  // NOLINT
    basic_result_storage &operator=(const basic_result_storage &) = default;  // NOLINT
    basic_result_storage &operator=(basic_result_storage &&) = default;       // NOLINT
    ~basic_result_storage() = default;

    template <class... Args>
    constexpr explicit basic_result_storage(in_place_type_t<_value_type> _,
                                            Args &&... args) noexcept(detail::is_nothrow_constructible<_value_type, Args...>)
        : _state{_, static_cast<Args &&>(args)...}
    {
    }
    template <class U, class... Args>
    constexpr basic_result_storage(in_place_type_t<_value_type> _, std::initializer_list<U> il,
                                   Args &&... args) noexcept(detail::is_nothrow_constructible<_value_type, std::initializer_list<U>, Args...>)
        : _state{_, il, static_cast<Args &&>(args)...}
    {
    }
    template <class... Args>
    constexpr explicit basic_result_storage(in_place_type_t<_error_type> _,
                                            Args &&... args) noexcept(detail::is_nothrow_constructible<_error_type, Args...>)
        : _state{_, static_cast<Args &&>(args)...}
    {
    }
    template <class U, class... Args>
    constexpr basic_result_storage(in_place_type_t<_error_type> _, std::initializer_list<U> il,
                                   Args &&... args) noexcept(detail::is_nothrow_constructible<_error_type, std::initializer_list<U>, Args...>)
        : _state{_, il, static_cast<Args &&>(args)...}
    {
    }

    struct compatible_conversion_tag
    {
    };
    template <class T, class U, class V>
    constexpr basic_result_storage(compatible_conversion_tag /*unused*/, const basic_result_storage<T, U, V> &o) noexcept(
    detail::is_nothrow_constructible<_value_type, T> &&detail::is_nothrow_constructible<_error_type, U>)
        : _state(o._state)
    {
    }
    template <class T, class U, class V>
    constexpr basic_result_storage(compatible_conversion_tag /*unused*/, basic_result_storage<T, U, V> &&o) noexcept(
    detail::is_nothrow_constructible<_value_type, T> &&detail::is_nothrow_constructible<_error_type, U>)
        : _state(static_cast<decltype(o._state) &&>(o._state))
    {
    }

    struct make_error_code_compatible_conversion_tag
    {
    };
    template <class T, class U, class V>
    constexpr basic_result_storage(make_error_code_compatible_conversion_tag /*unused*/, const basic_result_storage<T, U, V> &o) noexcept(
    detail::is_nothrow_constructible<_value_type, T> &&noexcept(make_error_code(std::declval<U>())))
        : _state(o._state._status.have_value() ? _state_type(in_place_type<_value_type>, o._state._value) :
                                                 _state_type(in_place_type<_error_type>, make_error_code(o._state._error)))
    {
    }
    template <class T, class U, class V>
    constexpr basic_result_storage(make_error_code_compatible_conversion_tag /*unused*/, basic_result_storage<T, U, V> &&o) noexcept(
    detail::is_nothrow_constructible<_value_type, T> &&noexcept(make_error_code(std::declval<U>())))
        : _state(o._state._status.have_value() ? _state_type(in_place_type<_value_type>, static_cast<T &&>(o._state._value)) :
                                                 _state_type(in_place_type<_error_type>, make_error_code(static_cast<U &&>(o._state._error))))
    {
    }

    struct make_exception_ptr_compatible_conversion_tag
    {
    };
    template <class T, class U, class V>
    constexpr basic_result_storage(make_exception_ptr_compatible_conversion_tag /*unused*/, const basic_result_storage<T, U, V> &o) noexcept(
    detail::is_nothrow_constructible<_value_type, T> &&noexcept(make_exception_ptr(std::declval<U>())))
        : _state(o._state._status.have_value() ? _state_type(in_place_type<_value_type>, o._state._value) :
                                                 _state_type(in_place_type<_error_type>, make_exception_ptr(o._state._error)))
    {
    }
    template <class T, class U, class V>
    constexpr basic_result_storage(make_exception_ptr_compatible_conversion_tag /*unused*/, basic_result_storage<T, U, V> &&o) noexcept(
    detail::is_nothrow_constructible<_value_type, T> &&noexcept(make_exception_ptr(std::declval<U>())))
        : _state(o._state._status.have_value() ? _state_type(in_place_type<_value_type>, static_cast<T &&>(o._state._value)) :
                                                 _state_type(in_place_type<_error_type>, make_exception_ptr(static_cast<U &&>(o._state._error))))
    {
    }
  };

}  // namespace detail
OUTCOME_V2_NAMESPACE_END

#endif
