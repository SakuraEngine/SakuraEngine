/* A C++ 11 tribool
(C) 2015-2017 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
File Created: June 2015


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

#ifndef QUICKCPPLIB_TRIBOOL_H
#define QUICKCPPLIB_TRIBOOL_H

/*! \file tribool.hpp
\brief Provides a constexpr C++ 11 tribool
*/

#include "config.hpp"

#include <istream>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace tribool
{
  /*! \defgroup tribool Constexpr C++ 11 tribool

  This tries to be mostly compatible with Boost.Tribool, except it's written in C++ 11 and is 100%
  constexpr throughout by being a strongly typed enum. Note that `other` state is aliased to
  `indeterminate` for Boost.Tribool compatibility. It is also aliased to unknown.

  Unlike Boost.Tribool this deliberately does not provide automatic conversion to bool. It was
  always questionable if code coped well with `operator!` != `!operator bool` anyway. Use the
  free functions true_(), false_(), other(), indeterminate() and unknown() to test the contents
  of this tribool.

  For sorting, false < unknown < true. Same for max/min. STL functions operators << and >>
  for iostream are defined.
  */
  //! \brief A constexpr C++ 11 tribool \ingroup tribool
  enum class tribool : signed char
  {
    false_ = -1,        //!< False
    true_ = 1,          //!< True
    other = 0,          //!< Other/Indeterminate/Unknown
    indeterminate = 0,  //!< Other/Indeterminate/Unknown
    unknown = 0         //!< Other/Indeterminate/Unknown
  };
  //! \brief Explicit construction from some signed integer. <0 false, >0 true, 0 is other \ingroup tribool
  constexpr inline tribool make_tribool(int v) noexcept { return v > 0 ? tribool::true_ : v < 0 ? tribool::false_ : tribool::other; }
  //! \brief If tribool is true return false, if tribool is false return true, else return other \ingroup tribool
  constexpr inline tribool operator~(tribool v) noexcept { return static_cast<tribool>(-static_cast<signed char>(v)); }
  //! \brief If a is true and b is true, return true, if either is false return false, else return other \ingroup tribool
  constexpr inline tribool operator&(tribool a, tribool b) noexcept { return (a == tribool::true_ && b == tribool::true_) ? tribool::true_ : (a == tribool::false_ || b == tribool::false_) ? tribool::false_ : tribool::other; }
  //! \brief If a is true or b is true, return true, if either is other return other, else return false \ingroup tribool
  constexpr inline tribool operator|(tribool a, tribool b) noexcept { return (a == tribool::true_ || b == tribool::true_) ? tribool::true_ : (a == tribool::other || b == tribool::other) ? tribool::other : tribool::false_; }

  //  //! \brief If tribool is false return true, else return false \ingroup tribool
  //  constexpr inline bool operator !(tribool v) noexcept { return a==tribool::false_; }
  //! \brief If a is true and b is true, return true \ingroup tribool
  constexpr inline bool operator&&(tribool a, tribool b) noexcept { return (a == tribool::true_ && b == tribool::true_); }
  //! \brief If a is true or b is true, return true \ingroup tribool
  constexpr inline bool operator||(tribool a, tribool b) noexcept { return (a == tribool::true_ || b == tribool::true_); }

  //! \brief Return true if tribool is true. \ingroup tribool
  constexpr inline bool true_(tribool a) noexcept { return a == tribool::true_; }
  //! \brief Return true if tribool is true. \ingroup tribool
  constexpr inline bool false_(tribool a) noexcept { return a == tribool::false_; }
  //! \brief Return true if tribool is other/indeterminate/unknown. \ingroup tribool
  constexpr inline bool other(tribool a) noexcept { return a == tribool::indeterminate; }
  //! \brief Return true if tribool is other/indeterminate/unknown. \ingroup tribool
  constexpr inline bool indeterminate(tribool a) noexcept { return a == tribool::indeterminate; }
  //! \brief Return true if tribool is other/indeterminate/unknown. \ingroup tribool
  constexpr inline bool unknown(tribool a) noexcept { return a == tribool::indeterminate; }
}
QUICKCPPLIB_NAMESPACE_END

namespace std
{
  inline istream &operator>>(istream &s, QUICKCPPLIB_NAMESPACE::tribool::tribool &a)
  {
    char c;
    s >> c;
    a = (c == '1') ? QUICKCPPLIB_NAMESPACE::tribool::tribool::true_ : (c == '0') ? QUICKCPPLIB_NAMESPACE::tribool::tribool::false_ : QUICKCPPLIB_NAMESPACE::tribool::tribool::other;
    return s;
  }
  inline ostream &operator<<(ostream &s, QUICKCPPLIB_NAMESPACE::tribool::tribool a)
  {
    char c = (a == QUICKCPPLIB_NAMESPACE::tribool::tribool::true_) ? '1' : (a == QUICKCPPLIB_NAMESPACE::tribool::tribool::false_) ? '0' : '?';
    return s << c;
  }
}

#endif
