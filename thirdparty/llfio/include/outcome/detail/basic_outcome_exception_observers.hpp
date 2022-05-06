/* Exception observers for outcome type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
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

#ifndef OUTCOME_BASIC_OUTCOME_EXCEPTION_OBSERVERS_HPP
#define OUTCOME_BASIC_OUTCOME_EXCEPTION_OBSERVERS_HPP

#include "basic_result_storage.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  template <class Base, class R, class S, class P, class NoValuePolicy> class basic_outcome_exception_observers : public Base
  {
  public:
    using exception_type = P;
    using Base::Base;

    constexpr inline exception_type &assume_exception() & noexcept;
    constexpr inline const exception_type &assume_exception() const &noexcept;
    constexpr inline exception_type &&assume_exception() && noexcept;
    constexpr inline const exception_type &&assume_exception() const &&noexcept;

    constexpr inline exception_type &exception() &;
    constexpr inline const exception_type &exception() const &;
    constexpr inline exception_type &&exception() &&;
    constexpr inline const exception_type &&exception() const &&;
  };

  // Exception observers not present
  template <class Base, class R, class S, class NoValuePolicy> class basic_outcome_exception_observers<Base, R, S, void, NoValuePolicy> : public Base
  {
  public:
    using Base::Base;
    constexpr void assume_exception() const noexcept { NoValuePolicy::narrow_exception_check(this); }
    constexpr void exception() const { NoValuePolicy::wide_exception_check(this); }
  };

}  // namespace detail

OUTCOME_V2_NAMESPACE_END

#endif
