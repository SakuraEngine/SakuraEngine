/* NT kernel error code category
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
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

#ifndef NTKERNEL_CATEGORY_CONFIG_HPP
#define NTKERNEL_CATEGORY_CONFIG_HPP

#if NTKERNEL_ERROR_CATEGORY_STATIC
#define NTKERNEL_ERROR_CATEGORY_API extern
#else
#ifdef _WIN32
#ifdef NTKERNEL_CATEGORY_SOURCE
#define NTKERNEL_ERROR_CATEGORY_API extern __declspec(dllexport)
#else
#define NTKERNEL_ERROR_CATEGORY_API extern
#endif
#else
#ifdef NTKERNEL_CATEGORY_SOURCE
#define NTKERNEL_ERROR_CATEGORY_API extern __attribute__((visibility("default")))
#else
#define NTKERNEL_ERROR_CATEGORY_API extern
#endif
#endif
#endif

#if NTKERNEL_ERROR_CATEGORY_INLINE
#define NTKERNEL_ERROR_CATEGORY_INLINE_API inline
#ifdef _MSC_VER
#pragma message("WARNING: Defining custom error code category ntkernel_category() via header only form is unreliable! Semantic comparisons will break! Define NTKERNEL_ERROR_CATEGORY_INLINE to 0 and only ever link in ntkernel_category() from a prebuilt shared library to avoid this problem.")
#else
#warning WARNING: Defining custom error code category ntkernel_category() via header only form is unreliable! Semantic comparisons will break! Define NTKERNEL_ERROR_CATEGORY_INLINE to 0 and only ever link in ntkernel_category() from a prebuilt shared library to avoid this problem.
#endif
#else
#define NTKERNEL_ERROR_CATEGORY_INLINE_API
#endif

#endif
