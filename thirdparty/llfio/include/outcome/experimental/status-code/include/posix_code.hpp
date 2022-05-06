/* Proposed SG14 status_code
(C) 2018-2020 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Feb 2018


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

#ifndef SYSTEM_ERROR2_POSIX_CODE_HPP
#define SYSTEM_ERROR2_POSIX_CODE_HPP

#ifdef SYSTEM_ERROR2_NOT_POSIX
#error <posix_code.hpp> is not includable when SYSTEM_ERROR2_NOT_POSIX is defined!
#endif

#include "quick_status_code_from_enum.hpp"

#include <cstring>  // for strchr and strerror_r

SYSTEM_ERROR2_NAMESPACE_BEGIN

class _posix_code_domain;
//! A POSIX error code, those returned by `errno`.
using posix_code = status_code<_posix_code_domain>;
//! A specialisation of `status_error` for the POSIX error code domain.
using posix_error = status_error<_posix_code_domain>;

namespace mixins
{
  template <class Base> struct mixin<Base, _posix_code_domain> : public Base
  {
    using Base::Base;

    //! Returns a `posix_code` for the current value of `errno`.
    static posix_code current() noexcept;
  };
}  // namespace mixins

/*! The implementation of the domain for POSIX error codes, those returned by `errno`.
 */
class _posix_code_domain : public status_code_domain
{
  template <class DomainType> friend class status_code;
  template <class StatusCode> friend class detail::indirecting_domain;
  using _base = status_code_domain;

  static _base::string_ref _make_string_ref(int c) noexcept
  {
    char buffer[1024] = "";
#ifdef _WIN32
    strerror_s(buffer, sizeof(buffer), c);
#elif defined(__gnu_linux__) && !defined(__ANDROID__)  // handle glibc's weird strerror_r()
    char *s = strerror_r(c, buffer, sizeof(buffer));  // NOLINT
    if(s != nullptr)
    {
      strncpy(buffer, s, sizeof(buffer));  // NOLINT
      buffer[1023] = 0;
    }
#else
    strerror_r(c, buffer, sizeof(buffer));
#endif
    size_t length = strlen(buffer);                     // NOLINT
    auto *p = static_cast<char *>(malloc(length + 1));  // NOLINT
    if(p == nullptr)
    {
      return _base::string_ref("failed to get message from system");
    }
    memcpy(p, buffer, length + 1);  // NOLINT
    return _base::atomic_refcounted_string_ref(p, length);
  }

public:
  //! The value type of the POSIX code, which is an `int`
  using value_type = int;
  using _base::string_ref;

  //! Default constructor
  constexpr explicit _posix_code_domain(typename _base::unique_id_type id = 0xa59a56fe5f310933) noexcept
      : _base(id)
  {
  }
  _posix_code_domain(const _posix_code_domain &) = default;
  _posix_code_domain(_posix_code_domain &&) = default;
  _posix_code_domain &operator=(const _posix_code_domain &) = default;
  _posix_code_domain &operator=(_posix_code_domain &&) = default;
  ~_posix_code_domain() = default;

  //! Constexpr singleton getter. Returns constexpr posix_code_domain variable.
  static inline constexpr const _posix_code_domain &get();

  virtual string_ref name() const noexcept override { return string_ref("posix domain"); }  // NOLINT
protected:
  virtual bool _do_failure(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);                             // NOLINT
    return static_cast<const posix_code &>(code).value() != 0;  // NOLINT
  }
  virtual bool _do_equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override  // NOLINT
  {
    assert(code1.domain() == *this);                          // NOLINT
    const auto &c1 = static_cast<const posix_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const posix_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    if(code2.domain() == generic_code_domain)
    {
      const auto &c2 = static_cast<const generic_code &>(code2);  // NOLINT
      if(static_cast<int>(c2.value()) == c1.value())
      {
        return true;
      }
    }
    return false;
  }
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);                         // NOLINT
    const auto &c = static_cast<const posix_code &>(code);  // NOLINT
    return generic_code(static_cast<errc>(c.value()));
  }
  virtual string_ref _do_message(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);                         // NOLINT
    const auto &c = static_cast<const posix_code &>(code);  // NOLINT
    return _make_string_ref(c.value());
  }
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(STANDARDESE_IS_IN_THE_HOUSE)
  SYSTEM_ERROR2_NORETURN virtual void _do_throw_exception(const status_code<void> &code) const override  // NOLINT
  {
    assert(code.domain() == *this);                         // NOLINT
    const auto &c = static_cast<const posix_code &>(code);  // NOLINT
    throw status_error<_posix_code_domain>(c);
  }
#endif
};
//! A constexpr source variable for the POSIX code domain, which is that of `errno`. Returned by `_posix_code_domain::get()`.
constexpr _posix_code_domain posix_code_domain;
inline constexpr const _posix_code_domain &_posix_code_domain::get()
{
  return posix_code_domain;
}

namespace mixins
{
  template <class Base> inline posix_code mixin<Base, _posix_code_domain>::current() noexcept { return posix_code(errno); }
}  // namespace mixins

SYSTEM_ERROR2_NAMESPACE_END

#endif
