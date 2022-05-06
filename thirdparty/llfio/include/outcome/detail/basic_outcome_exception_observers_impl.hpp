/* Exception observers for outcome type
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

#ifndef OUTCOME_BASIC_OUTCOME_EXCEPTION_OBSERVERS_IMPL_HPP
#define OUTCOME_BASIC_OUTCOME_EXCEPTION_OBSERVERS_IMPL_HPP

#include "basic_outcome_exception_observers.hpp"

#include "../policy/base.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace policy
{
  template <class R, class S, class P, class NoValuePolicy, class Impl> inline constexpr auto &&base::_exception(Impl &&self) noexcept
  {
    // Impl will be some internal implementation class which has no knowledge of the _ptr stored
    // beneath it. So statically cast, preserving rvalue and constness, to the derived class.
    using Outcome = OUTCOME_V2_NAMESPACE::detail::rebind_type<basic_outcome<R, S, P, NoValuePolicy>, decltype(self)>;
#if defined(_MSC_VER) && _MSC_VER < 1920
    // VS2017 tries a copy construction in the correct implementation despite that Outcome is always a rvalue or lvalue ref! :(
    basic_outcome<R, S, P, NoValuePolicy> &_self = (basic_outcome<R, S, P, NoValuePolicy> &) (self);  // NOLINT
#else
    Outcome _self = static_cast<Outcome>(self);  // NOLINT
#endif
    return static_cast<Outcome>(_self)._ptr;
  }
}  // namespace policy

namespace detail
{
  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::assume_exception() & noexcept
  {
    NoValuePolicy::narrow_exception_check(*this);
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(*this);
  }
  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr const typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::assume_exception() const &noexcept
  {
    NoValuePolicy::narrow_exception_check(*this);
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(*this);
  }
  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &&basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::assume_exception() && noexcept
  {
    NoValuePolicy::narrow_exception_check(std::move(*this));
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(std::move(*this));
  }
  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr const typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &&basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::assume_exception() const &&noexcept
  {
    NoValuePolicy::narrow_exception_check(std::move(*this));
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(std::move(*this));
  }

  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception() &
  {
    NoValuePolicy::wide_exception_check(*this);
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(*this);
  }
  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr const typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception() const &
  {
    NoValuePolicy::wide_exception_check(*this);
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(*this);
  }
  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &&basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception() &&
  {
    NoValuePolicy::wide_exception_check(std::move(*this));
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(std::move(*this));
  }
  template <class Base, class R, class S, class P, class NoValuePolicy> inline constexpr const typename basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception_type &&basic_outcome_exception_observers<Base, R, S, P, NoValuePolicy>::exception() const &&
  {
    NoValuePolicy::wide_exception_check(std::move(*this));
    return NoValuePolicy::template _exception<R, S, P, NoValuePolicy>(std::move(*this));
  }
}  // namespace detail

OUTCOME_V2_NAMESPACE_END

#endif
