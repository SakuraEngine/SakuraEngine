/* Configure QuickCppLib
(C) 2016-2021 Niall Douglas <http://www.nedproductions.biz/> (8 commits)


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

#ifndef QUICKCPPLIB_CONFIG_HPP
#define QUICKCPPLIB_CONFIG_HPP

#include "cpp_feature.h"

#ifndef QUICKCPPLIB_DISABLE_ABI_PERMUTATION
#include "revision.hpp"
#endif

#define QUICKCPPLIB_VERSION_GLUE2(a, b) a##b
#define QUICKCPPLIB_VERSION_GLUE(a, b) QUICKCPPLIB_VERSION_GLUE2(a, b)

// clang-format off
#ifdef DOXYGEN_IS_IN_THE_HOUSE
//! \brief The QuickCppLib namespace
namespace quickcpplib
{
  //! \brief Per commit unique namespace to prevent different git submodule versions clashing
  namespace _xxx
  {
  }
}
#define QUICKCPPLIB_NAMESPACE quickcpplib::_xxx
#define QUICKCPPLIB_NAMESPACE_BEGIN namespace quickcpplib { namespace _xxx {
#define QUICKCPPLIB_NAMESPACE_END } }
#elif defined(QUICKCPPLIB_DISABLE_ABI_PERMUTATION)
#define QUICKCPPLIB_NAMESPACE quickcpplib
#define QUICKCPPLIB_NAMESPACE_BEGIN namespace quickcpplib {
#define QUICKCPPLIB_NAMESPACE_END }
#else
#define QUICKCPPLIB_NAMESPACE quickcpplib::QUICKCPPLIB_VERSION_GLUE(_, QUICKCPPLIB_PREVIOUS_COMMIT_UNIQUE)
#define QUICKCPPLIB_NAMESPACE_BEGIN namespace quickcpplib { namespace QUICKCPPLIB_VERSION_GLUE(_, QUICKCPPLIB_PREVIOUS_COMMIT_UNIQUE) {
#define QUICKCPPLIB_NAMESPACE_END } }
#endif
// clang-format on

#ifdef _MSC_VER
#define QUICKCPPLIB_BIND_MESSAGE_PRAGMA2(x) __pragma(message(x))
#define QUICKCPPLIB_BIND_MESSAGE_PRAGMA(x) QUICKCPPLIB_BIND_MESSAGE_PRAGMA2(x)
#define QUICKCPPLIB_BIND_MESSAGE_PREFIX(type) __FILE__ "(" QUICKCPPLIB_BIND_STRINGIZE2(__LINE__) "): " type ": "
#define QUICKCPPLIB_BIND_MESSAGE_(type, prefix, msg) QUICKCPPLIB_BIND_MESSAGE_PRAGMA(prefix msg)
#else
#define QUICKCPPLIB_BIND_MESSAGE_PRAGMA2(x) _Pragma(#x)
#define QUICKCPPLIB_BIND_MESSAGE_PRAGMA(type, x) QUICKCPPLIB_BIND_MESSAGE_PRAGMA2(type x)
#define QUICKCPPLIB_BIND_MESSAGE_(type, prefix, msg) QUICKCPPLIB_BIND_MESSAGE_PRAGMA(type, msg)
#endif
//! Have the compiler output a message
#define QUICKCPPLIB_MESSAGE(msg) QUICKCPPLIB_BIND_MESSAGE_(message, QUICKCPPLIB_BIND_MESSAGE_PREFIX("message"), msg)
//! Have the compiler output a note
#define QUICKCPPLIB_NOTE(msg) QUICKCPPLIB_BIND_MESSAGE_(message, QUICKCPPLIB_BIND_MESSAGE_PREFIX("note"), msg)
//! Have the compiler output a warning
#define QUICKCPPLIB_WARNING(msg) QUICKCPPLIB_BIND_MESSAGE_(GCC warning, QUICKCPPLIB_BIND_MESSAGE_PREFIX("warning"), msg)
//! Have the compiler output an error
#define QUICKCPPLIB_ERROR(msg) QUICKCPPLIB_BIND_MESSAGE_(GCC error, QUICKCPPLIB_BIND_MESSAGE_PREFIX("error"), msg)


#ifdef QUICKCPPLIB_ENABLE_VALGRIND
#include "./valgrind/drd.h"
#define QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(p) ANNOTATE_RWLOCK_CREATE(p)
#define QUICKCPPLIB_ANNOTATE_RWLOCK_DESTROY(p) ANNOTATE_RWLOCK_DESTROY(p)
#define QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(p, s) ANNOTATE_RWLOCK_ACQUIRED(p, s)
#define QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(p, s) ANNOTATE_RWLOCK_RELEASED(p, s)
#define QUICKCPPLIB_ANNOTATE_IGNORE_READS_BEGIN() ANNOTATE_IGNORE_READS_BEGIN()
#define QUICKCPPLIB_ANNOTATE_IGNORE_READS_END() ANNOTATE_IGNORE_READS_END()
#define QUICKCPPLIB_ANNOTATE_IGNORE_WRITES_BEGIN() ANNOTATE_IGNORE_WRITES_BEGIN()
#define QUICKCPPLIB_ANNOTATE_IGNORE_WRITES_END() ANNOTATE_IGNORE_WRITES_END()
#define QUICKCPPLIB_DRD_IGNORE_VAR(x) DRD_IGNORE_VAR(x)
#define QUICKCPPLIB_DRD_STOP_IGNORING_VAR(x) DRD_STOP_IGNORING_VAR(x)
#define QUICKCPPLIB_RUNNING_ON_VALGRIND RUNNING_ON_VALGRIND
#else
#define QUICKCPPLIB_ANNOTATE_RWLOCK_CREATE(p)
#define QUICKCPPLIB_ANNOTATE_RWLOCK_DESTROY(p)
#define QUICKCPPLIB_ANNOTATE_RWLOCK_ACQUIRED(p, s)
#define QUICKCPPLIB_ANNOTATE_RWLOCK_RELEASED(p, s)
#define QUICKCPPLIB_ANNOTATE_IGNORE_READS_BEGIN()
#define QUICKCPPLIB_ANNOTATE_IGNORE_READS_END()
#define QUICKCPPLIB_ANNOTATE_IGNORE_WRITES_BEGIN()
#define QUICKCPPLIB_ANNOTATE_IGNORE_WRITES_END()
#define QUICKCPPLIB_DRD_IGNORE_VAR(x)
#define QUICKCPPLIB_DRD_STOP_IGNORING_VAR(x)
#define QUICKCPPLIB_RUNNING_ON_VALGRIND (0)
#endif

#ifndef QUICKCPPLIB_IN_ADDRESS_SANITIZER
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define QUICKCPPLIB_IN_ADDRESS_SANITIZER 1
#endif
#elif defined(__SANITIZE_ADDRESS__)
#define QUICKCPPLIB_IN_ADDRESS_SANITIZER 1
#endif
#endif
#ifndef QUICKCPPLIB_IN_ADDRESS_SANITIZER
#define QUICKCPPLIB_IN_ADDRESS_SANITIZER 0
#endif

#ifndef QUICKCPPLIB_IN_THREAD_SANITIZER
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define QUICKCPPLIB_IN_THREAD_SANITIZER 1
#endif
#elif defined(__SANITIZE_THREAD__)
#define QUICKCPPLIB_IN_THREAD_SANITIZER 1
#endif
#endif
#ifndef QUICKCPPLIB_IN_THREAD_SANITIZER
#define QUICKCPPLIB_IN_THREAD_SANITIZER 0
#endif

#ifndef QUICKCPPLIB_IN_UNDEFINED_SANITIZER
#if defined(__has_feature)
#if __has_feature(undefined_behavior_sanitizer)
#define QUICKCPPLIB_IN_UNDEFINED_SANITIZER 1
#endif
#elif defined(__SANITIZE_UNDEFINED__) || (__GNUC__ <= 9 && defined(__SANITIZE_ADDRESS__))
#define QUICKCPPLIB_IN_UNDEFINED_SANITIZER 1
#endif
#endif
#ifndef QUICKCPPLIB_IN_UNDEFINED_SANITIZER
#define QUICKCPPLIB_IN_UNDEFINED_SANITIZER 0
#endif

#if QUICKCPPLIB_IN_THREAD_SANITIZER
#define QUICKCPPLIB_DISABLE_THREAD_SANITIZE __attribute__((no_sanitize_thread))
#else
#define QUICKCPPLIB_DISABLE_THREAD_SANITIZE
#endif
#if QUICKCPPLIB_IN_THREAD_SANITIZER
#define QUICKCPPLIB_DISABLE_THREAD_SANITIZE __attribute__((no_sanitize_thread))
#else
#define QUICKCPPLIB_DISABLE_THREAD_SANITIZE
#endif
#if QUICKCPPLIB_IN_UNDEFINED_SANITIZER
#define QUICKCPPLIB_DISABLE_UNDEFINED_SANITIZE __attribute__((no_sanitize_undefined))
#else
#define QUICKCPPLIB_DISABLE_UNDEFINED_SANITIZE
#endif

#ifndef QUICKCPPLIB_SMT_PAUSE
#if !defined(__clang__) && defined(_MSC_VER) && _MSC_VER >= 1310 && (defined(_M_IX86) || defined(_M_X64))
extern "C" void _mm_pause();
#pragma intrinsic(_mm_pause)
#define QUICKCPPLIB_SMT_PAUSE _mm_pause();
#elif !defined(__c2__) && defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define QUICKCPPLIB_SMT_PAUSE __asm__ __volatile__("rep; nop" : : : "memory");
#endif
#endif

#ifndef QUICKCPPLIB_FORCEINLINE
#if defined(_MSC_VER)
#define QUICKCPPLIB_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define QUICKCPPLIB_FORCEINLINE __attribute__((always_inline))
#else
#define QUICKCPPLIB_FORCEINLINE
#endif
#endif

#ifndef QUICKCPPLIB_NOINLINE
#if defined(_MSC_VER)
#define QUICKCPPLIB_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define QUICKCPPLIB_NOINLINE __attribute__((noinline))
#else
#define QUICKCPPLIB_NOINLINE
#endif
#endif

#ifdef __has_cpp_attribute
#define QUICKCPPLIB_HAS_CPP_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
#define QUICKCPPLIB_HAS_CPP_ATTRIBUTE(attr) (0)
#endif

#if !defined(QUICKCPPLIB_NORETURN)
#if QUICKCPPLIB_HAS_CPP_ATTRIBUTE(noreturn)
#define QUICKCPPLIB_NORETURN [[noreturn]]
#elif defined(_MSC_VER)
#define QUICKCPPLIB_NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#define QUICKCPPLIB_NORETURN __attribute__((__noreturn__))
#else
#define QUICKCPPLIB_NORETURN
#endif
#endif

#ifndef QUICKCPPLIB_NODISCARD
#if defined(STANDARDESE_IS_IN_THE_HOUSE) || (_HAS_CXX17 && _MSC_VER >= 1911 /* VS2017.3 */)
#define QUICKCPPLIB_NODISCARD [[nodiscard]]
#endif
#endif
#ifndef QUICKCPPLIB_NODISCARD
#if QUICKCPPLIB_HAS_CPP_ATTRIBUTE(nodiscard)
#define QUICKCPPLIB_NODISCARD [[nodiscard]]
#elif defined(__clang__)  // deliberately not GCC
#define QUICKCPPLIB_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
// _Must_inspect_result_ expands into this
#define QUICKCPPLIB_NODISCARD                                                                                                                                                                                                                                                                                                  \
  __declspec("SAL_name"                                                                                                                                                                                                                                                                                                        \
             "("                                                                                                                                                                                                                                                                                                               \
             "\"_Must_inspect_result_\""                                                                                                                                                                                                                                                                                       \
             ","                                                                                                                                                                                                                                                                                                               \
             "\"\""                                                                                                                                                                                                                                                                                                            \
             ","                                                                                                                                                                                                                                                                                                               \
             "\"2\""                                                                                                                                                                                                                                                                                                           \
             ")") __declspec("SAL_begin") __declspec("SAL_post") __declspec("SAL_mustInspect") __declspec("SAL_post") __declspec("SAL_checkReturn") __declspec("SAL_end")
#endif
#endif
#ifndef QUICKCPPLIB_NODISCARD
#define QUICKCPPLIB_NODISCARD
#endif

#ifndef QUICKCPPLIB_SYMBOL_VISIBLE
#if defined(_MSC_VER)
#define QUICKCPPLIB_SYMBOL_VISIBLE
#elif defined(__GNUC__)
#define QUICKCPPLIB_SYMBOL_VISIBLE __attribute__((visibility("default")))
#else
#define QUICKCPPLIB_SYMBOL_VISIBLE
#endif
#endif

#ifndef QUICKCPPLIB_SYMBOL_EXPORT
#if defined(_MSC_VER)
#define QUICKCPPLIB_SYMBOL_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define QUICKCPPLIB_SYMBOL_EXPORT __attribute__((visibility("default")))
#else
#define QUICKCPPLIB_SYMBOL_EXPORT
#endif
#endif

#ifndef QUICKCPPLIB_SYMBOL_IMPORT
#if defined(_MSC_VER)
#define QUICKCPPLIB_SYMBOL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#define QUICKCPPLIB_SYMBOL_IMPORT
#else
#define QUICKCPPLIB_SYMBOL_IMPORT
#endif
#endif

#ifndef QUICKCPPLIB_THREAD_LOCAL
#if _MSC_VER >= 1800
#define QUICKCPPLIB_THREAD_LOCAL_IS_CXX11 1
#elif __cplusplus >= 201103L
#if __GNUC__ >= 5 && !defined(__clang__)
#define QUICKCPPLIB_THREAD_LOCAL_IS_CXX11 1
#elif defined(__has_feature)
#if __has_feature(cxx_thread_local)
#define QUICKCPPLIB_THREAD_LOCAL_IS_CXX11 1
#endif
#endif
#endif
#ifdef QUICKCPPLIB_THREAD_LOCAL_IS_CXX11
#define QUICKCPPLIB_THREAD_LOCAL thread_local
#endif
#ifndef QUICKCPPLIB_THREAD_LOCAL
#if defined(_MSC_VER)
#define QUICKCPPLIB_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__)
#define QUICKCPPLIB_THREAD_LOCAL __thread
#else
#error Unknown compiler, cannot set QUICKCPPLIB_THREAD_LOCAL
#endif
#endif
#endif

#include "detail/preprocessor_macro_overload.h"
#if defined(__cpp_concepts) && !defined(QUICKCPPLIB_DISABLE_CONCEPTS_SUPPORT)
#define QUICKCPPLIB_TREQUIRES_EXPAND8(a, b, c, d, e, f, g, h) a &&QUICKCPPLIB_TREQUIRES_EXPAND7(b, c, d, e, f, g, h)
#define QUICKCPPLIB_TREQUIRES_EXPAND7(a, b, c, d, e, f, g) a &&QUICKCPPLIB_TREQUIRES_EXPAND6(b, c, d, e, f, g)
#define QUICKCPPLIB_TREQUIRES_EXPAND6(a, b, c, d, e, f) a &&QUICKCPPLIB_TREQUIRES_EXPAND5(b, c, d, e, f)
#define QUICKCPPLIB_TREQUIRES_EXPAND5(a, b, c, d, e) a &&QUICKCPPLIB_TREQUIRES_EXPAND4(b, c, d, e)
#define QUICKCPPLIB_TREQUIRES_EXPAND4(a, b, c, d) a &&QUICKCPPLIB_TREQUIRES_EXPAND3(b, c, d)
#define QUICKCPPLIB_TREQUIRES_EXPAND3(a, b, c) a &&QUICKCPPLIB_TREQUIRES_EXPAND2(b, c)
#define QUICKCPPLIB_TREQUIRES_EXPAND2(a, b) a &&QUICKCPPLIB_TREQUIRES_EXPAND1(b)
#define QUICKCPPLIB_TREQUIRES_EXPAND1(a) a

//! Expands into a && b && c && ...
#define QUICKCPPLIB_TREQUIRES(...) requires QUICKCPPLIB_CALL_OVERLOAD(QUICKCPPLIB_TREQUIRES_EXPAND, __VA_ARGS__)

#define QUICKCPPLIB_TEMPLATE(...) template <__VA_ARGS__>
#define QUICKCPPLIB_TEXPR(...)                                                                                                                                                                                                                                                                                                 \
  requires { (__VA_ARGS__); }
#define QUICKCPPLIB_TPRED(...) (__VA_ARGS__)
#if !defined(_MSC_VER) || _MSC_FULL_VER >= 192400000  // VS 2019 16.3 is broken here
#define QUICKCPPLIB_REQUIRES(...) requires(__VA_ARGS__)
#else
#define QUICKCPPLIB_REQUIRES(...)
#endif
#else
#define QUICKCPPLIB_TEMPLATE(...) template <__VA_ARGS__
#define QUICKCPPLIB_TREQUIRES(...) , __VA_ARGS__ >
#define QUICKCPPLIB_TEXPR(...) typename = decltype(__VA_ARGS__)
#ifdef _MSC_VER
// MSVC gives an error if every specialisation of a template is always ill-formed, so
// the more powerful SFINAE form below causes pukeage :(
#define QUICKCPPLIB_TPRED(...) typename = typename std::enable_if<(__VA_ARGS__)>::type
#else
#define QUICKCPPLIB_TPRED(...) typename std::enable_if<(__VA_ARGS__), bool>::type = true
#endif
#define QUICKCPPLIB_REQUIRES(...)
#endif


#endif
