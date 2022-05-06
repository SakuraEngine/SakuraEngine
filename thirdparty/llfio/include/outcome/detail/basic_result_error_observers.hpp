/* Error observers for a very simple basic_result type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (2 commits)
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

#ifndef OUTCOME_BASIC_RESULT_ERROR_OBSERVERS_HPP
#define OUTCOME_BASIC_RESULT_ERROR_OBSERVERS_HPP

#include "basic_result_storage.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  template <class Base, class EC, class NoValuePolicy> class basic_result_error_observers : public Base
  {
  public:
    using error_type = EC;
    using Base::Base;

    constexpr error_type &assume_error() & noexcept
    {
      NoValuePolicy::narrow_error_check(static_cast<basic_result_error_observers &>(*this));
      return this->_state._error;
    }
    constexpr const error_type &assume_error() const &noexcept
    {
      NoValuePolicy::narrow_error_check(static_cast<const basic_result_error_observers &>(*this));
      return this->_state._error;
    }
    constexpr error_type &&assume_error() && noexcept
    {
      NoValuePolicy::narrow_error_check(static_cast<basic_result_error_observers &&>(*this));
      return static_cast<error_type &&>(this->_state._error);
    }
    constexpr const error_type &&assume_error() const &&noexcept
    {
      NoValuePolicy::narrow_error_check(static_cast<const basic_result_error_observers &&>(*this));
      return static_cast<const error_type &&>(this->_state._error);
    }

    constexpr error_type &error() &
    {
      NoValuePolicy::wide_error_check(static_cast<basic_result_error_observers &>(*this));
      return this->_state._error;
    }
    constexpr const error_type &error() const &
    {
      NoValuePolicy::wide_error_check(static_cast<const basic_result_error_observers &>(*this));
      return this->_state._error;
    }
    constexpr error_type &&error() &&
    {
      NoValuePolicy::wide_error_check(static_cast<basic_result_error_observers &&>(*this));
      return static_cast<error_type &&>(this->_state._error);
    }
    constexpr const error_type &&error() const &&
    {
      NoValuePolicy::wide_error_check(static_cast<const basic_result_error_observers &&>(*this));
      return static_cast<const error_type &&>(this->_state._error);
    }
  };
  template <class Base, class NoValuePolicy> class basic_result_error_observers<Base, void, NoValuePolicy> : public Base
  {
  public:
    using Base::Base;
    constexpr void assume_error() const noexcept { NoValuePolicy::narrow_error_check(*this); }
    constexpr void error() const { NoValuePolicy::wide_error_check(*this); }
  };
}  // namespace detail
OUTCOME_V2_NAMESPACE_END

#endif
