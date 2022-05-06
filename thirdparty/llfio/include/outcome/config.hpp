/* Configure Outcome with QuickCppLib
(C) 2015-2021 Niall Douglas <http://www.nedproductions.biz/> (24 commits)
File Created: August 2015


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

#ifndef OUTCOME_V2_CONFIG_HPP
#define OUTCOME_V2_CONFIG_HPP

#include "detail/version.hpp"

// Pull in detection of __MINGW64_VERSION_MAJOR
#if defined(__MINGW32__) && !defined(DOXYGEN_IS_IN_THE_HOUSE)
#include <_mingw.h>
#endif

#include "quickcpplib/config.hpp"

#ifndef __cpp_variadic_templates
#error Outcome needs variadic template support in the compiler
#endif
#if __cpp_constexpr < 201304 && _MSC_FULL_VER < 191100000
#error Outcome needs constexpr (C++ 14) support in the compiler
#endif
#ifndef __cpp_variable_templates
#error Outcome needs variable template support in the compiler
#endif
#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 6
#error Due to a bug in nested template variables parsing, Outcome does not work on GCCs earlier than v6.
#endif

#ifdef DOXYGEN_IS_IN_THE_HOUSE
#define OUTCOME_FORCEINLINE
#define OUTCOME_NODISCARD [[nodiscard]]
#define OUTCOME_TEMPLATE(...) template <__VA_ARGS__
#define OUTCOME_TREQUIRES(...) , __VA_ARGS__ >
#define OUTCOME_TEXPR(...) typename = decltype(__VA_ARGS__)
#define OUTCOME_TPRED(...) typename = std::enable_if_t<__VA_ARGS__>
#define OUTCOME_REQUIRES(...) requires __VA_ARGS__
/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
#endif

#ifndef OUTCOME_SYMBOL_VISIBLE
#define OUTCOME_SYMBOL_VISIBLE QUICKCPPLIB_SYMBOL_VISIBLE
#endif
#ifndef OUTCOME_FORCEINLINE
#define OUTCOME_FORCEINLINE QUICKCPPLIB_FORCEINLINE
#endif
#ifndef OUTCOME_NODISCARD
#define OUTCOME_NODISCARD QUICKCPPLIB_NODISCARD
#endif
#ifndef OUTCOME_THREAD_LOCAL
#define OUTCOME_THREAD_LOCAL QUICKCPPLIB_THREAD_LOCAL
#endif
#ifndef OUTCOME_TEMPLATE
#define OUTCOME_TEMPLATE(...) QUICKCPPLIB_TEMPLATE(__VA_ARGS__)
#endif
#ifndef OUTCOME_TREQUIRES
#define OUTCOME_TREQUIRES(...) QUICKCPPLIB_TREQUIRES(__VA_ARGS__)
#endif
#ifndef OUTCOME_TEXPR
#define OUTCOME_TEXPR(...) QUICKCPPLIB_TEXPR(__VA_ARGS__)
#endif
#ifndef OUTCOME_TPRED
#define OUTCOME_TPRED(...) QUICKCPPLIB_TPRED(__VA_ARGS__)
#endif
#ifndef OUTCOME_REQUIRES
#define OUTCOME_REQUIRES(...) QUICKCPPLIB_REQUIRES(__VA_ARGS__)
#endif

#include "quickcpplib/import.h"

#ifndef OUTCOME_ENABLE_LEGACY_SUPPORT_FOR
#define OUTCOME_ENABLE_LEGACY_SUPPORT_FOR 220  // the v2.2 Outcome release
#endif

#if defined(OUTCOME_UNSTABLE_VERSION)
#include "detail/revision.hpp"
#define OUTCOME_V2 (QUICKCPPLIB_BIND_NAMESPACE_VERSION(outcome_v2, OUTCOME_PREVIOUS_COMMIT_UNIQUE))
#ifdef _DEBUG
#define OUTCOME_V2_CXX_MODULE_NAME QUICKCPPLIB_BIND_NAMESPACE((QUICKCPPLIB_BIND_NAMESPACE_VERSION(outcome_v2d, OUTCOME_PREVIOUS_COMMIT_UNIQUE)))
#else
#define OUTCOME_V2_CXX_MODULE_NAME QUICKCPPLIB_BIND_NAMESPACE((QUICKCPPLIB_BIND_NAMESPACE_VERSION(outcome_v2, OUTCOME_PREVIOUS_COMMIT_UNIQUE)))
#endif
#else
#define OUTCOME_V2 (QUICKCPPLIB_BIND_NAMESPACE_VERSION(outcome_v2))
#ifdef _DEBUG
#define OUTCOME_V2_CXX_MODULE_NAME QUICKCPPLIB_BIND_NAMESPACE((QUICKCPPLIB_BIND_NAMESPACE_VERSION(outcome_v2d)))
#else
#define OUTCOME_V2_CXX_MODULE_NAME QUICKCPPLIB_BIND_NAMESPACE((QUICKCPPLIB_BIND_NAMESPACE_VERSION(outcome_v2)))
#endif
#endif

#if defined(GENERATING_OUTCOME_MODULE_INTERFACE)
#define OUTCOME_V2_NAMESPACE QUICKCPPLIB_BIND_NAMESPACE(OUTCOME_V2)
#define OUTCOME_V2_NAMESPACE_BEGIN QUICKCPPLIB_BIND_NAMESPACE_BEGIN(OUTCOME_V2)
#define OUTCOME_V2_NAMESPACE_EXPORT_BEGIN QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN(OUTCOME_V2)
#define OUTCOME_V2_NAMESPACE_END QUICKCPPLIB_BIND_NAMESPACE_END(OUTCOME_V2)
#else
#define OUTCOME_V2_NAMESPACE QUICKCPPLIB_BIND_NAMESPACE(OUTCOME_V2)
#define OUTCOME_V2_NAMESPACE_BEGIN QUICKCPPLIB_BIND_NAMESPACE_BEGIN(OUTCOME_V2)
#define OUTCOME_V2_NAMESPACE_EXPORT_BEGIN QUICKCPPLIB_BIND_NAMESPACE_BEGIN(OUTCOME_V2)
#define OUTCOME_V2_NAMESPACE_END QUICKCPPLIB_BIND_NAMESPACE_END(OUTCOME_V2)
#endif

#include <cstdint>  // for uint32_t etc
#include <initializer_list>
#include <iosfwd>  // for future serialisation
#include <new>     // for placement in moves etc
#include <type_traits>

#ifndef OUTCOME_USE_STD_IN_PLACE_TYPE
#if defined(_MSC_VER) && _HAS_CXX17
#define OUTCOME_USE_STD_IN_PLACE_TYPE 1  // MSVC always has std::in_place_type
#elif __cplusplus >= 201700
// libstdc++ before GCC 6 doesn't have it, despite claiming C++ 17 support
#ifdef __has_include
#if !__has_include(<variant>)
#define OUTCOME_USE_STD_IN_PLACE_TYPE 0  // must have it if <variant> is present
#endif
#endif

#ifndef OUTCOME_USE_STD_IN_PLACE_TYPE
#define OUTCOME_USE_STD_IN_PLACE_TYPE 1
#endif
#else
#define OUTCOME_USE_STD_IN_PLACE_TYPE 0
#endif
#endif

#if OUTCOME_USE_STD_IN_PLACE_TYPE
#include <utility>  // for in_place_type_t

OUTCOME_V2_NAMESPACE_BEGIN
template <class T> using in_place_type_t = std::in_place_type_t<T>;
using std::in_place_type;
OUTCOME_V2_NAMESPACE_END
#else
OUTCOME_V2_NAMESPACE_BEGIN
/*! AWAITING HUGO JSON CONVERSION TOOL 
type definition template <class T> in_place_type_t. Potential doc page: `in_place_type_t<T>`
*/
template <class T> struct in_place_type_t
{
  explicit in_place_type_t() = default;
};
/*! AWAITING HUGO JSON CONVERSION TOOL 
SIGNATURE NOT RECOGNISED
*/
template <class T> constexpr in_place_type_t<T> in_place_type{};
OUTCOME_V2_NAMESPACE_END
#endif

#ifndef OUTCOME_TRIVIAL_ABI
#if defined(STANDARDESE_IS_IN_THE_HOUSE) || __clang_major__ >= 7
//! Defined to be `[[clang::trivial_abi]]` when on a new enough clang compiler. Usually automatic, can be overriden.
#define OUTCOME_TRIVIAL_ABI [[clang::trivial_abi]]
#else
#define OUTCOME_TRIVIAL_ABI
#endif
#endif

OUTCOME_V2_NAMESPACE_BEGIN
namespace detail
{
  // Test if type is an in_place_type_t
  template <class T> struct is_in_place_type_t
  {
    static constexpr bool value = false;
  };
  template <class U> struct is_in_place_type_t<in_place_type_t<U>>
  {
    static constexpr bool value = true;
  };

  // Replace void with constructible void_type
  struct empty_type
  {
  };
  struct void_type
  {
    // We always compare true to another instance of me
    constexpr bool operator==(void_type /*unused*/) const noexcept { return true; }
    constexpr bool operator!=(void_type /*unused*/) const noexcept { return false; }
  };
  template <class T> using devoid = std::conditional_t<std::is_void<T>::value, void_type, T>;

  template <class Output, class Input> using rebind_type5 = Output;
  template <class Output, class Input>
  using rebind_type4 = std::conditional_t<                                   //
  std::is_volatile<Input>::value,                                            //
  std::add_volatile_t<rebind_type5<Output, std::remove_volatile_t<Input>>>,  //
  rebind_type5<Output, Input>>;
  template <class Output, class Input>
  using rebind_type3 = std::conditional_t<                             //
  std::is_const<Input>::value,                                         //
  std::add_const_t<rebind_type4<Output, std::remove_const_t<Input>>>,  //
  rebind_type4<Output, Input>>;
  template <class Output, class Input>
  using rebind_type2 = std::conditional_t<                                            //
  std::is_lvalue_reference<Input>::value,                                             //
  std::add_lvalue_reference_t<rebind_type3<Output, std::remove_reference_t<Input>>>,  //
  rebind_type3<Output, Input>>;
  template <class Output, class Input>
  using rebind_type = std::conditional_t<                                             //
  std::is_rvalue_reference<Input>::value,                                             //
  std::add_rvalue_reference_t<rebind_type2<Output, std::remove_reference_t<Input>>>,  //
  rebind_type2<Output, Input>>;

  // static_assert(std::is_same_v<rebind_type<int, volatile const double &&>, volatile const int &&>, "");


  /* True if type is the same or constructible. Works around a bug where clang + libstdc++
  pukes on std::is_constructible<filesystem::path, void> (this bug is fixed upstream).
  */
  template <class T, class U> struct _is_explicitly_constructible
  {
    static constexpr bool value = std::is_constructible<T, U>::value;
  };
  template <class T> struct _is_explicitly_constructible<T, void>
  {
    static constexpr bool value = false;
  };
  template <> struct _is_explicitly_constructible<void, void>
  {
    static constexpr bool value = false;
  };
  template <class T, class U> static constexpr bool is_explicitly_constructible = _is_explicitly_constructible<T, U>::value;

  template <class T, class U> struct _is_implicitly_constructible
  {
    static constexpr bool value = std::is_convertible<U, T>::value;
  };
  template <class T> struct _is_implicitly_constructible<T, void>
  {
    static constexpr bool value = false;
  };
  template <> struct _is_implicitly_constructible<void, void>
  {
    static constexpr bool value = false;
  };
  template <class T, class U> static constexpr bool is_implicitly_constructible = _is_implicitly_constructible<T, U>::value;

  template <class T, class... Args> struct _is_nothrow_constructible
  {
    static constexpr bool value = std::is_nothrow_constructible<T, Args...>::value;
  };
  template <class T> struct _is_nothrow_constructible<T, void>
  {
    static constexpr bool value = false;
  };
  template <> struct _is_nothrow_constructible<void, void>
  {
    static constexpr bool value = false;
  };
  template <class T, class... Args> static constexpr bool is_nothrow_constructible = _is_nothrow_constructible<T, Args...>::value;

  template <class T, class... Args> struct _is_constructible
  {
    static constexpr bool value = std::is_constructible<T, Args...>::value;
  };
  template <class T> struct _is_constructible<T, void>
  {
    static constexpr bool value = false;
  };
  template <> struct _is_constructible<void, void>
  {
    static constexpr bool value = false;
  };
  template <class T, class... Args> static constexpr bool is_constructible = _is_constructible<T, Args...>::value;

#ifndef OUTCOME_USE_STD_IS_NOTHROW_SWAPPABLE
#if defined(_MSC_VER) && _HAS_CXX17
#define OUTCOME_USE_STD_IS_NOTHROW_SWAPPABLE 1  // MSVC always has std::is_nothrow_swappable
#elif __cplusplus >= 201700
// libstdc++ before GCC 6 doesn't have it, despite claiming C++ 17 support
#ifdef __has_include
#if !__has_include(<variant>)
#define OUTCOME_USE_STD_IS_NOTHROW_SWAPPABLE 0
#endif
#endif

#ifndef OUTCOME_USE_STD_IS_NOTHROW_SWAPPABLE
#define OUTCOME_USE_STD_IS_NOTHROW_SWAPPABLE 1
#endif
#else
#define OUTCOME_USE_STD_IS_NOTHROW_SWAPPABLE 0
#endif
#endif

// True if type is nothrow swappable
#if !defined(STANDARDESE_IS_IN_THE_HOUSE) && OUTCOME_USE_STD_IS_NOTHROW_SWAPPABLE
  template <class T> using is_nothrow_swappable = std::is_nothrow_swappable<T>;
#else
  template <class T> struct is_nothrow_swappable
  {
    static constexpr bool value = std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value;
  };
#endif
}  // namespace detail
OUTCOME_V2_NAMESPACE_END


#ifndef OUTCOME_THROW_EXCEPTION
#ifdef __cpp_exceptions
#define OUTCOME_THROW_EXCEPTION(expr) throw expr
#else

#ifdef __ANDROID__
#define OUTCOME_DISABLE_EXECINFO
#endif

#ifndef OUTCOME_DISABLE_EXECINFO
#ifdef _WIN32
#include "quickcpplib/execinfo_win64.h"
#else
#include <execinfo.h>
#endif
#endif  // OUTCOME_DISABLE_EXECINFO
#include <cstdio>
#include <cstdlib>
OUTCOME_V2_NAMESPACE_BEGIN
namespace detail
{
  QUICKCPPLIB_NORETURN inline void do_fatal_exit(const char *expr)
  {
#if !defined(OUTCOME_DISABLE_EXECINFO)
    void *bt[16];
    size_t btlen = backtrace(bt, sizeof(bt) / sizeof(bt[0]));                                // NOLINT
#endif
    fprintf(stderr, "FATAL: Outcome throws exception %s with exceptions disabled\n", expr);  // NOLINT
#if !defined(OUTCOME_DISABLE_EXECINFO)
    char **bts = backtrace_symbols(bt, btlen);                                               // NOLINT
    if(bts != nullptr)
    {
      for(size_t n = 0; n < btlen; n++)
      {
        fprintf(stderr, "  %s\n", bts[n]);  // NOLINT
      }
      free(bts);  // NOLINT
    }
#endif
    abort();
  }
}  // namespace detail
OUTCOME_V2_NAMESPACE_END
#define OUTCOME_THROW_EXCEPTION(expr) OUTCOME_V2_NAMESPACE::detail::do_fatal_exit(#expr), (void) (expr)

#endif
#endif

#ifndef BOOST_OUTCOME_AUTO_TEST_CASE
#define BOOST_OUTCOME_AUTO_TEST_CASE(a, b) BOOST_AUTO_TEST_CASE(a, b)
#endif

#endif
