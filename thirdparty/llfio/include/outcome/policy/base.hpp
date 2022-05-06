/* Policies for result and outcome
(C) 2017-2020 Niall Douglas <http://www.nedproductions.biz/> (6 commits) and Andrzej Krzemieński <akrzemi1@gmail.com> (1 commit)
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

#ifndef OUTCOME_POLICY_BASE_HPP
#define OUTCOME_POLICY_BASE_HPP

#include "../detail/value_storage.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
namespace hooks
{
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class U> constexpr inline void hook_result_construction(T * /*unused*/, U && /*unused*/) noexcept {}
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class U> constexpr inline void hook_result_copy_construction(T * /*unused*/, U && /*unused*/) noexcept {}
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class U> constexpr inline void hook_result_move_construction(T * /*unused*/, U && /*unused*/) noexcept {}
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class U, class... Args>
  constexpr inline void hook_result_in_place_construction(T * /*unused*/, in_place_type_t<U> /*unused*/, Args &&... /*unused*/) noexcept
  {
  }
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class... U> constexpr inline void hook_outcome_construction(T * /*unused*/, U &&... /*unused*/) noexcept {}
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class U> constexpr inline void hook_outcome_copy_construction(T * /*unused*/, U && /*unused*/) noexcept {}
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class U> constexpr inline void hook_outcome_move_construction(T * /*unused*/, U && /*unused*/) noexcept {}
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class T, class U, class... Args>
  constexpr inline void hook_outcome_in_place_construction(T * /*unused*/, in_place_type_t<U> /*unused*/, Args &&... /*unused*/) noexcept
  {
  }
}  // namespace hooks
#endif

namespace policy
{
  namespace detail
  {
    using OUTCOME_V2_NAMESPACE::detail::make_ub;
  }
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  struct base
  {
    template <class... Args> static constexpr void _silence_unused(Args &&... /*unused*/) noexcept {}
  protected:
    template <class Impl> static constexpr void _make_ub(Impl &&self) noexcept { return detail::make_ub(static_cast<Impl &&>(self)); }
    template <class Impl> static constexpr bool _has_value(Impl &&self) noexcept { return self._state._status.have_value(); }
    template <class Impl> static constexpr bool _has_error(Impl &&self) noexcept { return self._state._status.have_error(); }
    template <class Impl> static constexpr bool _has_exception(Impl &&self) noexcept { return self._state._status.have_exception(); }
    template <class Impl> static constexpr bool _has_error_is_errno(Impl &&self) noexcept { return self._state._status.have_error_is_errno(); }

    template <class Impl> static constexpr void _set_has_value(Impl &&self, bool v) noexcept { self._state._status.set_have_value(v); }
    template <class Impl> static constexpr void _set_has_error(Impl &&self, bool v) noexcept { self._state._status.set_have_error(v); }
    template <class Impl> static constexpr void _set_has_exception(Impl &&self, bool v) noexcept { self._state._status.set_have_exception(v); }
    template <class Impl> static constexpr void _set_has_error_is_errno(Impl &&self, bool v) noexcept { self._state._status.set_have_error_is_errno(v); }

    template <class Impl> static constexpr auto &&_value(Impl &&self) noexcept { return static_cast<Impl &&>(self)._state._value; }
    template <class Impl> static constexpr auto &&_error(Impl &&self) noexcept { return static_cast<Impl &&>(self)._state._error; }

  public:
    template <class R, class S, class P, class NoValuePolicy, class Impl> static inline constexpr auto &&_exception(Impl &&self) noexcept;

    template <class T, class U> static constexpr inline void on_result_construction(T *inst, U &&v) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_result_construction(inst, static_cast<U &&>(v));
#else
      (void) inst;
      (void) v;
#endif
    }
    template <class T, class U> static constexpr inline void on_result_copy_construction(T *inst, U &&v) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_result_copy_construction(inst, static_cast<U &&>(v));
#else
      (void) inst;
      (void) v;
#endif
    }
    template <class T, class U> static constexpr inline void on_result_move_construction(T *inst, U &&v) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_result_move_construction(inst, static_cast<U &&>(v));
#else
      (void) inst;
      (void) v;
#endif
    }
    template <class T, class U, class... Args>
    static constexpr inline void on_result_in_place_construction(T *inst, in_place_type_t<U> _, Args &&... args) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_result_in_place_construction(inst, _, static_cast<Args &&>(args)...);
#else
      (void) inst;
      (void) _;
      _silence_unused(static_cast<Args &&>(args)...);
#endif
    }

    template <class T, class... U> static constexpr inline void on_outcome_construction(T *inst, U &&... args) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_outcome_construction(inst, static_cast<U &&>(args)...);
#else
      (void) inst;
      _silence_unused(static_cast<U &&>(args)...);
#endif
    }
    template <class T, class U> static constexpr inline void on_outcome_copy_construction(T *inst, U &&v) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_outcome_copy_construction(inst, static_cast<U &&>(v));
#else
      (void) inst;
      (void) v;
#endif
    }
    template <class T, class U> static constexpr inline void on_outcome_move_construction(T *inst, U &&v) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_outcome_move_construction(inst, static_cast<U &&>(v));
#else
      (void) inst;
      (void) v;
#endif
    }
    template <class T, class U, class... Args>
    static constexpr inline void on_outcome_in_place_construction(T *inst, in_place_type_t<U> _, Args &&... args) noexcept
    {
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
      using namespace hooks;
      hook_outcome_in_place_construction(inst, _, static_cast<Args &&>(args)...);
#else
      (void) inst;
      (void) _;
      _silence_unused(static_cast<Args &&>(args)...);
#endif
    }

    template <class Impl> static constexpr void narrow_value_check(Impl &&self) noexcept
    {
      if(!_has_value(self))
      {
        _make_ub(self);
      }
    }
    template <class Impl> static constexpr void narrow_error_check(Impl &&self) noexcept
    {
      if(!_has_error(self))
      {
        _make_ub(self);
      }
    }
    template <class Impl> static constexpr void narrow_exception_check(Impl &&self) noexcept
    {
      if(!_has_exception(self))
      {
        _make_ub(self);
      }
    }
  };
}  // namespace policy

OUTCOME_V2_NAMESPACE_END

#endif
