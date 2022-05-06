/* iostream specialisations for result and outcome
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (21 commits)
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

#ifndef OUTCOME_IOSTREAM_SUPPORT_HPP
#define OUTCOME_IOSTREAM_SUPPORT_HPP

#include "outcome.hpp"

#include <iostream>
#include <sstream>

OUTCOME_V2_NAMESPACE_BEGIN

namespace detail
{
  template <class T> typename std::add_lvalue_reference<T>::type lvalueref() noexcept;

  template <template <class, class> class ValueStorage, class T, class E> inline std::ostream &value_storage_out(std::ostream &s, const ValueStorage<T, E> &v)
  {
    s << static_cast<uint16_t>(v._status.status_value) << " " << v._status.spare_storage_value << " ";
    if(v._status.have_value())
    {
      s << v._value;  // NOLINT
    }
    if(v._status.have_error())
    {
      s << v._error;  // NOLINT
    }
    return s;
  }
  template <template <class, class> class ValueStorage, class E> inline std::ostream &value_storage_out(std::ostream &s, const ValueStorage<void, E> &v)
  {
    s << static_cast<uint16_t>(v._status.status_value) << " " << v._status.spare_storage_value << " ";
    if(v._status.have_error())
    {
      s << v._error;  // NOLINT
    }
    return s;
  }
  template <template <class, class> class ValueStorage, class T> inline std::ostream &value_storage_out(std::ostream &s, const ValueStorage<T, void> &v)
  {
    s << static_cast<uint16_t>(v._status.status_value) << " " << v._status.spare_storage_value << " ";
    if(v._status.have_value())
    {
      s << v._value;  // NOLINT
    }
    return s;
  }

  template <class T, class E> inline std::ostream &operator<<(std::ostream &s, const value_storage_trivial<T, E> &v) { return value_storage_out(s, v); }
  template <class T, class E> inline std::ostream &operator<<(std::ostream &s, const value_storage_nontrivial<T, E> &v) { return value_storage_out(s, v); }

  template <template <class, class> class ValueStorage, class T, class E> inline std::istream &value_storage_in(std::istream &s, ValueStorage<T, E> &v)
  {
    using type = ValueStorage<T, E>;
    v.~type();
    new(&v) type;
    uint16_t x, y;
    s >> x >> y;
    v._status.status_value = static_cast<detail::status>(x);
    v._status.spare_storage_value = y;
    if(v._status.have_value())
    {
      new(&v._value) decltype(v._value)();  // NOLINT
      s >> v._value;                        // NOLINT
    }
    if(v._status.have_error())
    {
      new(&v._error) decltype(v._error)();  // NOLINT
      s >> v._error;                        // NOLINT
    }
    return s;
  }
  template <template <class, class> class ValueStorage, class E> inline std::istream &value_storage_in(std::istream &s, ValueStorage<void, E> &v)
  {
    using type = ValueStorage<void, E>;
    v.~type();
    new(&v) type;
    uint16_t x, y;
    s >> x >> y;
    v._status.status_value = static_cast<detail::status>(x);
    v._status.spare_storage_value = y;
    if(v._status.have_error())
    {
      new(&v._error) decltype(v._error)();  // NOLINT
      s >> v._error;                        // NOLINT
    }
    return s;
  }
  template <template <class, class> class ValueStorage, class T> inline std::istream &value_storage_in(std::istream &s, ValueStorage<T, void> &v)
  {
    using type = ValueStorage<T, void>;
    v.~type();
    new(&v) type;
    uint16_t x, y;
    s >> x >> y;
    v._status.status_value = static_cast<detail::status>(x);
    v._status.spare_storage_value = y;
    if(v._status.have_value())
    {
      new(&v._value) decltype(v._value)();  // NOLINT
      s >> v._value;                        // NOLINT
    }
    return s;
  }
  template <class T, class E> inline std::istream &operator>>(std::istream &s, value_storage_trivial<T, E> &v) { return value_storage_in(s, v); }
  template <class T, class E> inline std::istream &operator>>(std::istream &s, value_storage_nontrivial<T, E> &v) { return value_storage_in(s, v); }
  OUTCOME_TEMPLATE(class T)
  OUTCOME_TREQUIRES(OUTCOME_TPRED(!std::is_constructible<std::error_code, T>::value))
  inline std::string safe_message(T && /*unused*/) { return {}; }
  inline std::string safe_message(const std::error_code &ec) { return " (" + ec.message() + ")"; }
}  // namespace detail

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
OUTCOME_TEMPLATE(class R, class S, class P)
OUTCOME_TREQUIRES(OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<R>()), OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<S>()))
inline std::istream &operator>>(std::istream &s, basic_result<R, S, P> &v)
{
  s >> v._iostreams_state();
  if(v.has_error())
  {
    s >> v.assume_error();
  }
  return s;
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
OUTCOME_TEMPLATE(class R, class S, class P)
OUTCOME_TREQUIRES(OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<R>()), OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<S>()))
inline std::ostream &operator<<(std::ostream &s, const basic_result<R, S, P> &v)
{
  s << v._iostreams_state();
  if(v.has_error())
  {
    s << v.assume_error();
  }
  return s;
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class R, class S, class P> inline std::string print(const basic_result<R, S, P> &v)
{
  std::stringstream s;
  if(v.has_value())
  {
    s << v.value();
  }
  if(v.has_error())
  {
    s << v.error() << detail::safe_message(v.error());
  }
  return s.str();
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class S, class P> inline std::string print(const basic_result<void, S, P> &v)
{
  std::stringstream s;
  if(v.has_value())
  {
    s << "(+void)";
  }
  if(v.has_error())
  {
    s << v.error() << detail::safe_message(v.error());
  }
  return s.str();
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class R, class P> inline std::string print(const basic_result<R, void, P> &v)
{
  std::stringstream s;
  if(v.has_value())
  {
    s << v.value();
  }
  if(v.has_error())
  {
    s << "(-void)";
  }
  return s.str();
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class P> inline std::string print(const basic_result<void, void, P> &v)
{
  std::stringstream s;
  if(v.has_value())
  {
    s << "(+void)";
  }
  if(v.has_error())
  {
    s << "(-void)";
  }
  return s.str();
}

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
OUTCOME_TEMPLATE(class R, class S, class P, class N)
OUTCOME_TREQUIRES(OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<R>()), OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<S>()), OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<P>()))
inline std::istream &operator>>(std::istream &s, outcome<R, S, P, N> &v)
{
  s >> v._iostreams_state();
  if(v.has_error())
  {
    s >> v.assume_error();
  }
  if(v.has_exception())
  {
    s >> v.assume_exception();
  }
  return s;
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
OUTCOME_TEMPLATE(class R, class S, class P, class N)
OUTCOME_TREQUIRES(OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<R>()), OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<S>()), OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<P>()))
inline std::ostream &operator<<(std::ostream &s, const outcome<R, S, P, N> &v)
{
  s << v._iostreams_state();
  if(v.has_error())
  {
    s << v.assume_error();
  }
  if(v.has_exception())
  {
    s << v.assume_exception();
  }
  return s;
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class R, class S, class P, class N> inline std::string print(const outcome<R, S, P, N> &v)
{
  std::stringstream s;
  int total = static_cast<int>(v.has_value()) + static_cast<int>(v.has_error()) + static_cast<int>(v.has_exception());
  if(total > 1)
  {
    s << "{ ";
  }
  s << print(static_cast<const basic_result<R, S, N> &>(static_cast<const detail::basic_result_final<R, S, N> &>(v)));  // NOLINT
  if(total > 1)
  {
    s << ", ";
  }
  if(v.has_exception())
  {
#ifdef __cpp_exceptions
    try
    {
      rethrow_exception(v.exception());
    }
    catch(const std::system_error &e)
    {
      s << "std::system_error code " << e.code() << ": " << e.what();
    }
    catch(const std::exception &e)
    {
      s << "std::exception: " << e.what();
    }
    catch(...)
#endif
    {
      s << "unknown exception";
    }
  }
  if(total > 1)
  {
    s << " }";
  }
  return s.str();
}
OUTCOME_V2_NAMESPACE_END

#endif
