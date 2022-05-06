/* Convenience macros for importing local namespace binds
(C) 2014-2017 Niall Douglas <http://www.nedproductions.biz/> (9 commits)
File Created: Aug 2014


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

#ifndef QUICKCPPLIB_BIND_IMPORT_HPP
#define QUICKCPPLIB_BIND_IMPORT_HPP

/* 2014-10-9 ned: I lost today figuring out the below. I really hate the C preprocessor now.
 *
 * Anyway, infinity = 8. It's easy to expand below if needed.
 */
#include "detail/preprocessor_macro_overload.h"

#define QUICKCPPLIB_BIND_STRINGIZE(a) #a
#define QUICKCPPLIB_BIND_STRINGIZE2(a) QUICKCPPLIB_BIND_STRINGIZE(a)
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION8(a, b, c, d, e, f, g, h) a##_##b##_##c##_##d##_##e##_##f##_##g##_##h
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION7(a, b, c, d, e, f, g) a##_##b##_##c##_##d##_##e##_##f##_##g
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION6(a, b, c, d, e, f) a##_##b##_##c##_##d##_##e##_##f
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION5(a, b, c, d, e) a##_##b##_##c##_##d##_##e
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION4(a, b, c, d) a##_##b##_##c##_##d
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION3(a, b, c) a##_##b##_##c
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION2(a, b) a##_##b
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION1(a) a
//! Concatenates each parameter with _
#define QUICKCPPLIB_BIND_NAMESPACE_VERSION(...) QUICKCPPLIB_CALL_OVERLOAD(QUICKCPPLIB_BIND_NAMESPACE_VERSION, __VA_ARGS__)

#define QUICKCPPLIB_BIND_NAMESPACE_SELECT_2(name, modifier) name
#define QUICKCPPLIB_BIND_NAMESPACE_SELECT2(name, modifier) ::name
#define QUICKCPPLIB_BIND_NAMESPACE_SELECT_1(name) name
#define QUICKCPPLIB_BIND_NAMESPACE_SELECT1(name) ::name
#define QUICKCPPLIB_BIND_NAMESPACE_SELECT_(...) QUICKCPPLIB_CALL_OVERLOAD_(QUICKCPPLIB_BIND_NAMESPACE_SELECT_, __VA_ARGS__)
#define QUICKCPPLIB_BIND_NAMESPACE_SELECT(...) QUICKCPPLIB_CALL_OVERLOAD_(QUICKCPPLIB_BIND_NAMESPACE_SELECT, __VA_ARGS__)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND8(a, b, c, d, e, f, g, h)                                                                                                                                                                                                                                                             \
  QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a QUICKCPPLIB_BIND_NAMESPACE_SELECT b QUICKCPPLIB_BIND_NAMESPACE_SELECT c QUICKCPPLIB_BIND_NAMESPACE_SELECT d QUICKCPPLIB_BIND_NAMESPACE_SELECT e QUICKCPPLIB_BIND_NAMESPACE_SELECT f QUICKCPPLIB_BIND_NAMESPACE_SELECT g QUICKCPPLIB_BIND_NAMESPACE_SELECT h
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND7(a, b, c, d, e, f, g) QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a QUICKCPPLIB_BIND_NAMESPACE_SELECT b QUICKCPPLIB_BIND_NAMESPACE_SELECT c QUICKCPPLIB_BIND_NAMESPACE_SELECT d QUICKCPPLIB_BIND_NAMESPACE_SELECT e QUICKCPPLIB_BIND_NAMESPACE_SELECT f QUICKCPPLIB_BIND_NAMESPACE_SELECT g
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND6(a, b, c, d, e, f) QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a QUICKCPPLIB_BIND_NAMESPACE_SELECT b QUICKCPPLIB_BIND_NAMESPACE_SELECT c QUICKCPPLIB_BIND_NAMESPACE_SELECT d QUICKCPPLIB_BIND_NAMESPACE_SELECT e QUICKCPPLIB_BIND_NAMESPACE_SELECT f
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND5(a, b, c, d, e) QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a QUICKCPPLIB_BIND_NAMESPACE_SELECT b QUICKCPPLIB_BIND_NAMESPACE_SELECT c QUICKCPPLIB_BIND_NAMESPACE_SELECT d QUICKCPPLIB_BIND_NAMESPACE_SELECT e
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND4(a, b, c, d) QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a QUICKCPPLIB_BIND_NAMESPACE_SELECT b QUICKCPPLIB_BIND_NAMESPACE_SELECT c QUICKCPPLIB_BIND_NAMESPACE_SELECT d
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND3(a, b, c) QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a QUICKCPPLIB_BIND_NAMESPACE_SELECT b QUICKCPPLIB_BIND_NAMESPACE_SELECT c
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND2(a, b) QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a QUICKCPPLIB_BIND_NAMESPACE_SELECT b
#define QUICKCPPLIB_BIND_NAMESPACE_EXPAND1(a) QUICKCPPLIB_BIND_NAMESPACE_SELECT_ a
//! Expands into a::b::c:: ...
#define QUICKCPPLIB_BIND_NAMESPACE(...) QUICKCPPLIB_CALL_OVERLOAD(QUICKCPPLIB_BIND_NAMESPACE_EXPAND, __VA_ARGS__)

#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT2(name, modifier)                                                                                                                                                                                                                                                     \
  modifier namespace name                                                                                                                                                                                                                                                                                                      \
  {
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT1(name)                                                                                                                                                                                                                                                               \
  namespace name                                                                                                                                                                                                                                                                                                               \
  {
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT(...) QUICKCPPLIB_CALL_OVERLOAD_(QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT, __VA_ARGS__)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND8(a, b, c, d, e, f, g, h) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND7(b, c, d, e, f, g, h)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND7(a, b, c, d, e, f, g) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND6(b, c, d, e, f, g)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND6(a, b, c, d, e, f) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND5(b, c, d, e, f)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND5(a, b, c, d, e) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND4(b, c, d, e)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND4(a, b, c, d) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND3(b, c, d)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND3(a, b, c) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND2(b, c)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND2(a, b) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND1(b)
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND1(a) QUICKCPPLIB_BIND_NAMESPACE_BEGIN_NAMESPACE_SELECT a

//! Expands into namespace a { namespace b { namespace c ...
#define QUICKCPPLIB_BIND_NAMESPACE_BEGIN(...) QUICKCPPLIB_CALL_OVERLOAD(QUICKCPPLIB_BIND_NAMESPACE_BEGIN_EXPAND, __VA_ARGS__)

#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT2(name, modifier)                                                                                                                                                                                                                                              \
  modifier namespace name                                                                                                                                                                                                                                                                                                      \
  {
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT1(name)                                                                                                                                                                                                                                                        \
  export namespace name                                                                                                                                                                                                                                                                                                        \
  {
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT(...) QUICKCPPLIB_CALL_OVERLOAD_(QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT, __VA_ARGS__)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND8(a, b, c, d, e, f, g, h) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND7(b, c, d, e, f, g, h)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND7(a, b, c, d, e, f, g) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND6(b, c, d, e, f, g)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND6(a, b, c, d, e, f) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND5(b, c, d, e, f)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND5(a, b, c, d, e) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND4(b, c, d, e)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND4(a, b, c, d) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND3(b, c, d)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND3(a, b, c) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND2(b, c)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND2(a, b) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND1(b)
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND1(a) QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_NAMESPACE_SELECT a

//! Expands into export namespace a { namespace b { namespace c ...
#define QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN(...) QUICKCPPLIB_CALL_OVERLOAD(QUICKCPPLIB_BIND_NAMESPACE_EXPORT_BEGIN_EXPAND, __VA_ARGS__)

#define QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT2(name, modifier) }
#define QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT1(name) }
#define QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT(...) QUICKCPPLIB_CALL_OVERLOAD_(QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT, __VA_ARGS__)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND8(a, b, c, d, e, f, g, h) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND7(b, c, d, e, f, g, h)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND7(a, b, c, d, e, f, g) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND6(b, c, d, e, f, g)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND6(a, b, c, d, e, f) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND5(b, c, d, e, f)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND5(a, b, c, d, e) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND4(b, c, d, e)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND4(a, b, c, d) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND3(b, c, d)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND3(a, b, c) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND2(b, c)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND2(a, b) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND1(b)
#define QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND1(a) QUICKCPPLIB_BIND_NAMESPACE_END_NAMESPACE_SELECT a

//! Expands into } } ...
#define QUICKCPPLIB_BIND_NAMESPACE_END(...) QUICKCPPLIB_CALL_OVERLOAD(QUICKCPPLIB_BIND_NAMESPACE_END_EXPAND, __VA_ARGS__)

//! Expands into a static const char string array used to mark BindLib compatible namespaces
#define QUICKCPPLIB_BIND_DECLARE(decl, desc) static const char *quickcpplib_out[] = {#decl, desc};

#endif
