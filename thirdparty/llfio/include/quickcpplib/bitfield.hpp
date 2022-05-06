/* Yet another C++ 11 constexpr bitfield
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Aug 2016


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

#ifndef QUICKCPPLIB_BITFIELD_HPP
#define QUICKCPPLIB_BITFIELD_HPP

#include "config.hpp"

#include <type_traits>  // for std::underlying_type

QUICKCPPLIB_NAMESPACE_BEGIN

//! \brief The namespace for the bitfield types
namespace bitfield
{
  /*!
  \brief Constexpr typesafe bitwise flags support

  Usage:
  \code
  QUICKCPPLIB_BITFIELD_BEGIN(flag)
  {
    flag1 = 1 << 0,
    flag2 = 1 << 1,
    ...
  }
  QUICKCPPLIB_BITFIELD_END(flag)
  ...
  flag myflags = flag::flag1|flag::flag2;
  if(myflags & flag::flag1) ...
  if(!myflags) ...
  // This intentionally fails to compile:
  // if(myflags && flag::flag2)
  //
  // That is because of operator precedence difficulties in say:
  // if(myflags && flag::flag1 && myflags && flag::flag2)
  //
  // This works fine though:
  if((myflags & flag::flag1) || (myflags & flag::flag2)) ...
  \endcode
  */
  template <class Enum> struct bitfield : public Enum
  {
    //! The C style enum type which represents flags in this bitfield
    using enum_type = typename Enum::enum_type;
    //! The type which the C style enum implicitly converts to
    using underlying_type = std::underlying_type_t<enum_type>;

  private:
    underlying_type _value;

  public:
    //! Default construct to all bits zero
    constexpr bitfield() noexcept : _value(0) {}
    //! Implicit construction from the C style enum
    constexpr bitfield(enum_type v) noexcept : _value(v) {}
    //! Implicit construction from the underlying type of the C enum
    constexpr bitfield(underlying_type v) noexcept : _value(v) {}

    //! Permit explicit casting to the underlying type
    explicit constexpr operator underlying_type() const noexcept { return _value; }
    //! Test for non-zeroness
    explicit constexpr operator bool() const noexcept { return !!_value; }
    //! Test for zeroness
    constexpr bool operator!() const noexcept { return !_value; }

    //! Test for equality
    constexpr bool operator==(bitfield o) const noexcept { return _value == o._value; }
    //! Test for equality
    constexpr bool operator==(enum_type o) const noexcept { return _value == o; }
    //! Test for inequality
    constexpr bool operator!=(bitfield o) const noexcept { return _value != o._value; }
    //! Test for inequality
    constexpr bool operator!=(enum_type o) const noexcept { return _value != o; }

    //! Performs a bitwise NOT
    constexpr bitfield operator~() const noexcept { return bitfield(~_value); }
    //! Performs a bitwise AND
    constexpr bitfield operator&(bitfield o) const noexcept { return bitfield(_value & o._value); }
    //! Performs a bitwise AND
    constexpr bitfield operator&(enum_type o) const noexcept { return bitfield(_value & o); }
    //! Performs a bitwise AND
    constexpr bitfield &operator&=(bitfield o) noexcept
    {
      _value &= o._value;
      return *this;
    }
    //! Performs a bitwise AND
    constexpr bitfield &operator&=(enum_type o) noexcept
    {
      _value &= o;
      return *this;
    }
    //! Trap incorrect use of logical AND
    template <class T> bool operator&&(T) noexcept = delete;
    //! Trap incorrect use of logical OR
    // template <class T> bool operator||(T) noexcept = delete;
    //! Performs a bitwise OR
    constexpr bitfield operator|(bitfield o) const noexcept { return bitfield(_value | o._value); }
    //! Performs a bitwise OR
    constexpr bitfield operator|(enum_type o) const noexcept { return bitfield(_value | o); }
    //! Performs a bitwise OR
    constexpr bitfield &operator|=(bitfield o) noexcept
    {
      _value |= o._value;
      return *this;
    }
    //! Performs a bitwise OR
    constexpr bitfield &operator|=(enum_type o) noexcept
    {
      _value |= o;
      return *this;
    }
    //! Performs a bitwise XOR
    constexpr bitfield operator^(bitfield o) const noexcept { return bitfield(_value ^ o._value); }
    //! Performs a bitwise XOR
    constexpr bitfield operator^(enum_type o) const noexcept { return bitfield(_value ^ o); }
    //! Performs a bitwise XOR
    constexpr bitfield &operator^=(bitfield o) noexcept
    {
      _value ^= o._value;
      return *this;
    }
    //! Performs a bitwise XOR
    constexpr bitfield &operator^=(enum_type o) noexcept
    {
      _value ^= o;
      return *this;
    }
  };

#ifdef DOXYGEN_IS_IN_THE_HOUSE
//! Begins a typesafe bitfield
#define QUICKCPPLIB_BITFIELD_BEGIN(type) enum bitfield__##type : unsigned

//! Begins a typesafe bitfield
#define QUICKCPPLIB_BITFIELD_BEGIN_T(type, UT) enum bitfield__##type : UT

//! Ends a typesafe bitfield
#define QUICKCPPLIB_BITFIELD_END(type) ;
#else
//! Begins a typesafe bitfield with underlying representation `unsigned`
#define QUICKCPPLIB_BITFIELD_BEGIN(type)                                                                                                                                                                                                                                                                                       \
  \
struct type##_base                                                                                                                                                                                                                                                                                                             \
  \
{                                                                                                                                                                                                                                                                                                                         \
  enum enum_type : unsigned

//! Begins a typesafe bitfield with underlying representation `unsigned`
#define QUICKCPPLIB_BITFIELD_BEGIN_T(type, UT)                                                                                                                                                                                                                                                                                       \
  \
struct type##_base                                                                                                                                                                                                                                                                                                             \
  \
{                                                                                                                                                                                                                                                                                                                         \
  enum enum_type : UT

//! Ends a typesafe bitfield
#define QUICKCPPLIB_BITFIELD_END(type)                                                                                                                                                                                                                                                                                         \
  \
;                                                                                                                                                                                                                                                                                                                         \
  }                                                                                                                                                                                                                                                                                                                            \
  ;                                                                                                                                                                                                                                                                                                                            \
  \
using type = QUICKCPPLIB_NAMESPACE::bitfield::bitfield<type##_base>;
}
#endif

  QUICKCPPLIB_NAMESPACE_END
#endif
