//
// byte-lite, a C++17-like byte type for C++98 and later.
// For more information see https://github.com/martinmoene/byte-lite
//
// Copyright 2017-2019 Martin Moene
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef NONSTD_BYTE_LITE_HPP
#define NONSTD_BYTE_LITE_HPP

#define byte_lite_MAJOR  0
#define byte_lite_MINOR  3
#define byte_lite_PATCH  0

#define byte_lite_VERSION  byte_STRINGIFY(byte_lite_MAJOR) "." byte_STRINGIFY(byte_lite_MINOR) "." byte_STRINGIFY(byte_lite_PATCH)

#define byte_STRINGIFY(  x )  byte_STRINGIFY_( x )
#define byte_STRINGIFY_( x )  #x

// byte-lite configuration:

#define byte_BYTE_DEFAULT  0
#define byte_BYTE_NONSTD   1
#define byte_BYTE_STD      2

#if !defined( byte_CONFIG_SELECT_BYTE )
# define byte_CONFIG_SELECT_BYTE  ( byte_HAVE_STD_BYTE ? byte_BYTE_STD : byte_BYTE_NONSTD )
#endif

// C++ language version detection (C++20 is speculative):
// Note: VC14.0/1900 (VS2015) lacks too much from C++14.

#ifndef   byte_CPLUSPLUS
# if defined(_MSVC_LANG ) && !defined(__clang__)
#  define byte_CPLUSPLUS  (_MSC_VER == 1900 ? 201103L : _MSVC_LANG )
# else
#  define byte_CPLUSPLUS  __cplusplus
# endif
#endif

#define byte_CPP98_OR_GREATER  ( byte_CPLUSPLUS >= 199711L )
#define byte_CPP11_OR_GREATER  ( byte_CPLUSPLUS >= 201103L )
#define byte_CPP11_OR_GREATER_ ( byte_CPLUSPLUS >= 201103L )
#define byte_CPP14_OR_GREATER  ( byte_CPLUSPLUS >= 201402L )
#define byte_CPP17_OR_GREATER  ( byte_CPLUSPLUS >= 201703L )
#define byte_CPP20_OR_GREATER  ( byte_CPLUSPLUS >= 202000L )

// use C++17 std::byte if available and requested:

#if byte_CPP17_OR_GREATER
# define byte_HAVE_STD_BYTE  1
#else
# define byte_HAVE_STD_BYTE  0
#endif

#define byte_USES_STD_BYTE  ( (byte_CONFIG_SELECT_BYTE == byte_BYTE_STD) || ((byte_CONFIG_SELECT_BYTE == byte_BYTE_DEFAULT) && byte_HAVE_STD_BYTE) )

//
// Using std::byte:
//

#if byte_USES_STD_BYTE

#include <cstddef>
#include <type_traits>

namespace nonstd {

using std::byte;
using std::to_integer;

// Provide compatibility with nonstd::byte:

template
<
    class IntegerType
    , class = typename std::enable_if<std::is_integral<IntegerType>::value>::type
>
inline constexpr byte to_byte( IntegerType v ) noexcept
{
    return static_cast<byte>( v );
}

inline constexpr unsigned char to_uchar( byte b ) noexcept
{
    return to_integer<unsigned char>( b );
}

} // namespace nonstd

#else // byte_USES_STD_BYTE

// half-open range [lo..hi):
#define byte_BETWEEN( v, lo, hi ) ( (lo) <= (v) && (v) < (hi) )

// Compiler versions:
//
// MSVC++  6.0  _MSC_VER == 1200  byte_COMPILER_MSVC_VERSION ==  60  (Visual Studio 6.0)
// MSVC++  7.0  _MSC_VER == 1300  byte_COMPILER_MSVC_VERSION ==  70  (Visual Studio .NET 2002)
// MSVC++  7.1  _MSC_VER == 1310  byte_COMPILER_MSVC_VERSION ==  71  (Visual Studio .NET 2003)
// MSVC++  8.0  _MSC_VER == 1400  byte_COMPILER_MSVC_VERSION ==  80  (Visual Studio 2005)
// MSVC++  9.0  _MSC_VER == 1500  byte_COMPILER_MSVC_VERSION ==  90  (Visual Studio 2008)
// MSVC++ 10.0  _MSC_VER == 1600  byte_COMPILER_MSVC_VERSION == 100  (Visual Studio 2010)
// MSVC++ 11.0  _MSC_VER == 1700  byte_COMPILER_MSVC_VERSION == 110  (Visual Studio 2012)
// MSVC++ 12.0  _MSC_VER == 1800  byte_COMPILER_MSVC_VERSION == 120  (Visual Studio 2013)
// MSVC++ 14.0  _MSC_VER == 1900  byte_COMPILER_MSVC_VERSION == 140  (Visual Studio 2015)
// MSVC++ 14.1  _MSC_VER >= 1910  byte_COMPILER_MSVC_VERSION == 141  (Visual Studio 2017)
// MSVC++ 14.2  _MSC_VER >= 1920  byte_COMPILER_MSVC_VERSION == 142  (Visual Studio 2019)

#if defined(_MSC_VER ) && !defined(__clang__)
# define byte_COMPILER_MSVC_VER      (_MSC_VER )
# define byte_COMPILER_MSVC_VERSION  (_MSC_VER / 10 - 10 * ( 5 + (_MSC_VER < 1900 ) ) )
#else
# define byte_COMPILER_MSVC_VER      0
# define byte_COMPILER_MSVC_VERSION  0
#endif

#define byte_COMPILER_VERSION( major, minor, patch ) ( 10 * ( 10 * (major) + (minor) ) + (patch) )

#if defined(__clang__)
# define byte_COMPILER_CLANG_VERSION byte_COMPILER_VERSION( __clang_major__, __clang_minor__, __clang_patchlevel__ )
#else
# define byte_COMPILER_CLANG_VERSION 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
# define byte_COMPILER_GNUC_VERSION byte_COMPILER_VERSION( __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__ )
#else
# define byte_COMPILER_GNUC_VERSION 0
#endif

#if byte_BETWEEN( byte_COMPILER_MSVC_VER, 1300, 1900 )
# pragma warning( push )
# pragma warning( disable: 4345 )   // initialization behavior changed
#endif

// Compiler non-strict aliasing:

#if defined(__clang__) || defined(__GNUC__)
# define byte_may_alias  __attribute__((__may_alias__))
#else
# define byte_may_alias
#endif

// Presence of language and library features:

#ifdef _HAS_CPP0X
# define byte_HAS_CPP0X  _HAS_CPP0X
#else
# define byte_HAS_CPP0X  0
#endif

// Unless defined otherwise below, consider VC14 as C++11 for variant-lite:

#if byte_COMPILER_MSVC_VER >= 1900
# undef  byte_CPP11_OR_GREATER
# define byte_CPP11_OR_GREATER  1
#endif

#define byte_CPP11_90   (byte_CPP11_OR_GREATER_ || byte_COMPILER_MSVC_VER >= 1500)
#define byte_CPP11_100  (byte_CPP11_OR_GREATER_ || byte_COMPILER_MSVC_VER >= 1600)
#define byte_CPP11_110  (byte_CPP11_OR_GREATER_ || byte_COMPILER_MSVC_VER >= 1700)
#define byte_CPP11_120  (byte_CPP11_OR_GREATER_ || byte_COMPILER_MSVC_VER >= 1800)
#define byte_CPP11_140  (byte_CPP11_OR_GREATER_ || byte_COMPILER_MSVC_VER >= 1900)
#define byte_CPP11_141  (byte_CPP11_OR_GREATER_ || byte_COMPILER_MSVC_VER >= 1910)

#define byte_CPP14_000  (byte_CPP14_OR_GREATER)
#define byte_CPP17_000  (byte_CPP17_OR_GREATER)

// Presence of C++11 language features:

#define byte_HAVE_CONSTEXPR_11          byte_CPP11_140
#define byte_HAVE_DEFAULT_FUNCTION_TEMPLATE_ARG  byte_CPP11_120
#define byte_HAVE_NOEXCEPT              byte_CPP11_140

// Presence of C++14 language features:

#define byte_HAVE_CONSTEXPR_14          byte_CPP14_000

// Presence of C++17 language features:

#define byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE  byte_CPP17_000

// Presence of C++ library features:

#define byte_HAVE_TYPE_TRAITS           byte_CPP11_90

// C++ feature usage:

#if byte_HAVE_CONSTEXPR_11
# define byte_constexpr constexpr
#else
# define byte_constexpr /*constexpr*/
#endif

#if byte_HAVE_CONSTEXPR_14
# define byte_constexpr14 constexpr
#else
# define byte_constexpr14 /*constexpr*/
#endif

#if byte_HAVE_NOEXCEPT
# define byte_noexcept noexcept
#else
# define byte_noexcept /*noexcept*/
#endif

// additional includes:

#if byte_HAVE_TYPE_TRAITS
# include <type_traits>
#endif

// conditionally enabling:

#if byte_HAVE_DEFAULT_FUNCTION_TEMPLATE_ARG
# define byte_ENABLE_IF_INTEGRAL_T(T)  \
    , class = typename std::enable_if<std::is_integral<T>::value>::type
#else
# define byte_ENABLE_IF_INTEGRAL_T(T)
#endif

#if byte_HAVE_DEFAULT_FUNCTION_TEMPLATE_ARG
# define byte_DEFAULT_TEMPLATE_ARG(T)  \
    = T
#else
# define byte_DEFAULT_TEMPLATE_ARG(T)
#endif

namespace nonstd {

namespace detail {
}

#if byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE
  enum class byte_may_alias byte : unsigned char {};
#else
  struct byte_may_alias byte { typedef unsigned char type; type v; };
#endif

template<
    class IntegerType
    byte_ENABLE_IF_INTEGRAL_T( IntegerType )
>
inline byte_constexpr byte to_byte( IntegerType v ) byte_noexcept
{
#if   byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE
    return static_cast<byte>( v );
#elif byte_HAVE_CONSTEXPR_11
    return { static_cast<typename byte::type>( v ) };
#else
    byte b = { static_cast<typename byte::type>( v ) }; return b;
#endif
}

#if byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE

template<
    class IntegerType = typename std::underlying_type<byte>::type
    byte_ENABLE_IF_INTEGRAL_T( IntegerType )
>
inline byte_constexpr IntegerType to_integer( byte b ) byte_noexcept
{
    return static_cast<IntegerType>( b );
}

#elif byte_CPP11_OR_GREATER

template<
    class IntegerType
    byte_DEFAULT_TEMPLATE_ARG(typename byte::type)  byte_ENABLE_IF_INTEGRAL_T( IntegerType )
>
inline byte_constexpr IntegerType to_integer( byte b ) byte_noexcept
{
    return b.v;
}

#else // for C++98:

template<
    class IntegerType
>
inline byte_constexpr IntegerType to_integer( byte b ) byte_noexcept
{
    return b.v;
}

inline byte_constexpr unsigned char  to_integer( byte b ) byte_noexcept
{
    return to_integer<unsigned char >( b );
}

#endif

inline byte_constexpr unsigned char to_uchar( byte b ) byte_noexcept
{
    return to_integer<unsigned char>( b );
}

inline byte_constexpr unsigned char to_uchar( int i ) byte_noexcept
{
    return static_cast<unsigned char>( i );
}

#if ! byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE

inline byte_constexpr bool operator==( byte l, byte r ) byte_noexcept
{
    return l.v == r.v;
}

inline byte_constexpr bool operator!=( byte l, byte r ) byte_noexcept
{
    return !( l == r );
}

inline byte_constexpr bool operator< ( byte l, byte r ) byte_noexcept
{
    return l.v < r.v;
}

inline byte_constexpr bool operator<=( byte l, byte r ) byte_noexcept
{
    return !( r < l );
}

inline byte_constexpr bool operator> ( byte l, byte r ) byte_noexcept
{
    return ( r < l );
}

inline byte_constexpr bool operator>=( byte l, byte r ) byte_noexcept
{
    return !( l < r );
}

#endif

template<
    class IntegerType
    byte_ENABLE_IF_INTEGRAL_T( IntegerType )
>
inline byte_constexpr14 byte & operator<<=( byte & b, IntegerType shift ) byte_noexcept
{
#if byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE
    return b = to_byte( to_uchar( b ) << shift );
#else
    b.v = to_uchar( b.v << shift ); return b;
#endif
}

template<
    class IntegerType
    byte_ENABLE_IF_INTEGRAL_T( IntegerType )
>
inline byte_constexpr14 byte & operator>>=( byte & b, IntegerType shift ) byte_noexcept
{
#if byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE
    return b = to_byte( to_uchar( b ) >> shift );
#else
    b.v = to_uchar( b.v >> shift ); return b;
#endif
}

template<
    class IntegerType
    byte_ENABLE_IF_INTEGRAL_T( IntegerType )
>
inline byte_constexpr byte operator<<( byte b, IntegerType shift ) byte_noexcept
{
    return to_byte( to_uchar( b ) << shift );
}

template<
    class IntegerType
    byte_ENABLE_IF_INTEGRAL_T( IntegerType )
>
inline byte_constexpr byte operator>>( byte b, IntegerType shift ) byte_noexcept
{
    return to_byte( to_uchar( b ) >> shift );
}

inline byte_constexpr14 byte & operator|=( byte & l, byte r ) byte_noexcept
{
#if byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE
    return l = to_byte( to_uchar( l ) | to_uchar( r ) );
#else
    l.v = to_uchar( l ) | to_uchar( r ); return l;
#endif
}

inline byte_constexpr14 byte & operator&=( byte & l, byte r ) byte_noexcept
{
#if byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE
    return l = to_byte( to_uchar( l ) & to_uchar( r ) );
#else
    l.v = to_uchar( l ) & to_uchar( r ); return l;
#endif
}

inline byte_constexpr14 byte & operator^=( byte & l, byte r ) byte_noexcept
{
#if byte_HAVE_ENUM_CLASS_CONSTRUCTION_FROM_UNDERLYING_TYPE
    return l = to_byte( to_uchar( l ) ^ to_uchar (r ) );
#else
    l.v = to_uchar( l ) ^ to_uchar (r ); return l;
#endif
}

inline byte_constexpr byte operator|( byte l, byte r ) byte_noexcept
{
    return to_byte( to_uchar( l ) | to_uchar( r ) );
}

inline byte_constexpr byte operator&( byte l, byte r ) byte_noexcept
{
    return to_byte( to_uchar( l ) & to_uchar( r ) );
}

inline byte_constexpr byte operator^( byte l, byte r ) byte_noexcept
{
    return to_byte( to_uchar( l ) ^ to_uchar( r ) );
}

inline byte_constexpr byte operator~( byte b ) byte_noexcept
{
    return to_byte( ~to_uchar( b ) );
}

} // namespace nonstd

#endif // byte_USES_STD_BYTE

#endif // NONSTD_BYTE_LITE_HPP

// end of file
