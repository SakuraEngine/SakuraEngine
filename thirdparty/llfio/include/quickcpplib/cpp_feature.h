/* Provides SG-10 feature checking for all C++ compilers
(C) 2014-2017 Niall Douglas <http://www.nedproductions.biz/> (13 commits)
File Created: Nov 2014


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

#ifndef QUICKCPPLIB_HAS_FEATURE_H
#define QUICKCPPLIB_HAS_FEATURE_H

#if __cplusplus >= 201103L

// Some of these macros ended up getting removed by ISO standards,
// they are prefixed with ////
////#if !defined(__cpp_alignas)
////#define __cpp_alignas 190000
////#endif
////#if !defined(__cpp_default_function_template_args)
////#define __cpp_default_function_template_args 190000
////#endif
////#if !defined(__cpp_defaulted_functions)
////#define __cpp_defaulted_functions 190000
////#endif
////#if !defined(__cpp_deleted_functions)
////#define __cpp_deleted_functions 190000
////#endif
////#if !defined(__cpp_generalized_initializers)
////#define __cpp_generalized_initializers 190000
////#endif
////#if !defined(__cpp_implicit_moves)
////#define __cpp_implicit_moves 190000
////#endif
////#if !defined(__cpp_inline_namespaces)
////#define __cpp_inline_namespaces 190000
////#endif
////#if !defined(__cpp_local_type_template_args)
////#define __cpp_local_type_template_args 190000
////#endif
////#if !defined(__cpp_noexcept)
////#define __cpp_noexcept 190000
////#endif
////#if !defined(__cpp_nonstatic_member_init)
////#define __cpp_nonstatic_member_init 190000
////#endif
////#if !defined(__cpp_nullptr)
////#define __cpp_nullptr 190000
////#endif
////#if !defined(__cpp_override_control)
////#define __cpp_override_control 190000
////#endif
////#if !defined(__cpp_thread_local)
////#define __cpp_thread_local 190000
////#endif
////#if !defined(__cpp_auto_type)
////#define __cpp_auto_type 190000
////#endif
////#if !defined(__cpp_strong_enums)
////#define __cpp_strong_enums 190000
////#endif
////#if !defined(__cpp_trailing_return)
////#define __cpp_trailing_return 190000
////#endif
////#if !defined(__cpp_unrestricted_unions)
////#define __cpp_unrestricted_unions 190000
////#endif

#if !defined(__cpp_alias_templates)
#define __cpp_alias_templates 190000
#endif

#if !defined(__cpp_attributes)
#define __cpp_attributes 190000
#endif

#if !defined(__cpp_constexpr)
#if __cplusplus >= 201402L
#define __cpp_constexpr 201304  // relaxed constexpr
#else
#define __cpp_constexpr 190000
#endif
#endif

#if !defined(__cpp_decltype)
#define __cpp_decltype 190000
#endif

#if !defined(__cpp_delegating_constructors)
#define __cpp_delegating_constructors 190000
#endif

#if !defined(__cpp_explicit_conversion)   //// renamed from __cpp_explicit_conversions
#define __cpp_explicit_conversion 190000
#endif

#if !defined(__cpp_inheriting_constructors)
#define __cpp_inheriting_constructors 190000
#endif

#if !defined(__cpp_initializer_lists)   //// NEW
#define __cpp_initializer_lists 190000
#endif

#if !defined(__cpp_lambdas)
#define __cpp_lambdas 190000
#endif

#if !defined(__cpp_nsdmi)
#define __cpp_nsdmi 190000  //// NEW
#endif

#if !defined(__cpp_range_based_for)   //// renamed from __cpp_range_for
#define __cpp_range_based_for 190000
#endif

#if !defined(__cpp_raw_strings)
#define __cpp_raw_strings 190000
#endif

#if !defined(__cpp_ref_qualifiers)   //// renamed from __cpp_reference_qualified_functions
#define __cpp_ref_qualifiers 190000
#endif

#if !defined(__cpp_rvalue_references)
#define __cpp_rvalue_references 190000
#endif

#if !defined(__cpp_static_assert)
#define __cpp_static_assert 190000
#endif

#if !defined(__cpp_unicode_characters)   //// NEW
#define __cpp_unicode_characters 190000
#endif

#if !defined(__cpp_unicode_literals)
#define __cpp_unicode_literals 190000
#endif

#if !defined(__cpp_user_defined_literals)
#define __cpp_user_defined_literals 190000
#endif

#if !defined(__cpp_variadic_templates)
#define __cpp_variadic_templates 190000
#endif

#endif

#if __cplusplus >= 201402L

// Some of these macros ended up getting removed by ISO standards,
// they are prefixed with ////
////#if !defined(__cpp_contextual_conversions)
////#define __cpp_contextual_conversions 190000
////#endif
////#if !defined(__cpp_digit_separators)
////#define __cpp_digit_separators 190000
////#endif
////#if !defined(__cpp_relaxed_constexpr)
////#define __cpp_relaxed_constexpr 190000
////#endif
////#if !defined(__cpp_runtime_arrays)
////# define __cpp_runtime_arrays 190000
////#endif


#if !defined(__cpp_aggregate_nsdmi)
#define __cpp_aggregate_nsdmi 190000
#endif

#if !defined(__cpp_binary_literals)
#define __cpp_binary_literals 190000
#endif

#if !defined(__cpp_decltype_auto)
#define __cpp_decltype_auto 190000
#endif

#if !defined(__cpp_generic_lambdas)
#define __cpp_generic_lambdas 190000
#endif

#if !defined(__cpp_init_captures)
#define __cpp_init_captures 190000
#endif

#if !defined(__cpp_return_type_deduction)
#define __cpp_return_type_deduction 190000
#endif

#if !defined(__cpp_sized_deallocation)
#define __cpp_sized_deallocation 190000
#endif

#if !defined(__cpp_variable_templates)
#define __cpp_variable_templates 190000
#endif

#endif


// VS2010: _MSC_VER=1600
// VS2012: _MSC_VER=1700
// VS2013: _MSC_VER=1800
// VS2015: _MSC_VER=1900
// VS2017: _MSC_VER=1910
#if defined(_MSC_VER) && !defined(__clang__)

#if !defined(__cpp_exceptions) && defined(_CPPUNWIND)
#define __cpp_exceptions 190000
#endif

#if !defined(__cpp_rtti) && defined(_CPPRTTI)
#define __cpp_rtti 190000
#endif


// C++ 11

#if !defined(__cpp_alias_templates) && _MSC_VER >= 1800
#define __cpp_alias_templates 190000
#endif

#if !defined(__cpp_attributes)
#define __cpp_attributes 190000
#endif

#if !defined(__cpp_constexpr) && _MSC_FULL_VER >= 190023506 /* VS2015 */
#define __cpp_constexpr 190000
#endif

#if !defined(__cpp_decltype) && _MSC_VER >= 1600
#define __cpp_decltype 190000
#endif

#if !defined(__cpp_delegating_constructors) && _MSC_VER >= 1800
#define __cpp_delegating_constructors 190000
#endif

#if !defined(__cpp_explicit_conversion) && _MSC_VER >= 1800
#define __cpp_explicit_conversion 190000
#endif

#if !defined(__cpp_inheriting_constructors) && _MSC_VER >= 1900
#define __cpp_inheriting_constructors 190000
#endif

#if !defined(__cpp_initializer_lists) && _MSC_VER >= 1900
#define __cpp_initializer_lists 190000
#endif

#if !defined(__cpp_lambdas) && _MSC_VER >= 1600
#define __cpp_lambdas 190000
#endif

#if !defined(__cpp_nsdmi) && _MSC_VER >= 1900
#define __cpp_nsdmi 190000
#endif

#if !defined(__cpp_range_based_for) && _MSC_VER >= 1700
#define __cpp_range_based_for 190000
#endif

#if !defined(__cpp_raw_strings) && _MSC_VER >= 1800
#define __cpp_raw_strings 190000
#endif

#if !defined(__cpp_ref_qualifiers) && _MSC_VER >= 1900
#define __cpp_ref_qualifiers 190000
#endif

#if !defined(__cpp_rvalue_references) && _MSC_VER >= 1600
#define __cpp_rvalue_references 190000
#endif

#if !defined(__cpp_static_assert) && _MSC_VER >= 1600
#define __cpp_static_assert 190000
#endif

//#if !defined(__cpp_unicode_literals)
//# define __cpp_unicode_literals 190000
//#endif

#if !defined(__cpp_user_defined_literals) && _MSC_VER >= 1900
#define __cpp_user_defined_literals 190000
#endif

#if !defined(__cpp_variadic_templates) && _MSC_VER >= 1800
#define __cpp_variadic_templates 190000
#endif


// C++ 14

//#if !defined(__cpp_aggregate_nsdmi)
//#define __cpp_aggregate_nsdmi 190000
//#endif

#if !defined(__cpp_binary_literals) && _MSC_VER >= 1900
#define __cpp_binary_literals 190000
#endif

#if !defined(__cpp_decltype_auto) && _MSC_VER >= 1900
#define __cpp_decltype_auto 190000
#endif

#if !defined(__cpp_generic_lambdas) && _MSC_VER >= 1900
#define __cpp_generic_lambdas 190000
#endif

#if !defined(__cpp_init_captures) && _MSC_VER >= 1900
#define __cpp_init_captures 190000
#endif

#if !defined(__cpp_return_type_deduction) && _MSC_VER >= 1900
#define __cpp_return_type_deduction 190000
#endif

#if !defined(__cpp_sized_deallocation) && _MSC_VER >= 1900
#define __cpp_sized_deallocation 190000
#endif

#if !defined(__cpp_variable_templates) && _MSC_FULL_VER >= 190023506
#define __cpp_variable_templates 190000
#endif

#endif  // _MSC_VER


// Much to my surprise, GCC's support of these is actually incomplete, so fill in the gaps
#if(defined(__GNUC__) && !defined(__clang__))

#define QUICKCPPLIB_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if !defined(__cpp_exceptions) && defined(__EXCEPTIONS)
#define __cpp_exceptions 190000
#endif

#if !defined(__cpp_rtti) && defined(__GXX_RTTI)
#define __cpp_rtti 190000
#endif


// C++ 11
#if defined(__GXX_EXPERIMENTAL_CXX0X__)

#if !defined(__cpp_alias_templates) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_alias_templates 190000
#endif

#if !defined(__cpp_attributes) && (QUICKCPPLIB_GCC >= 40800)
#define __cpp_attributes 190000
#endif

#if !defined(__cpp_constexpr) && (QUICKCPPLIB_GCC >= 40600)
#define __cpp_constexpr 190000
#endif

#if !defined(__cpp_decltype) && (QUICKCPPLIB_GCC >= 40300)
#define __cpp_decltype 190000
#endif

#if !defined(__cpp_delegating_constructors) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_delegating_constructors 190000
#endif

#if !defined(__cpp_explicit_conversion) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_explicit_conversion 190000
#endif

#if !defined(__cpp_inheriting_constructors) && (QUICKCPPLIB_GCC >= 40800)
#define __cpp_inheriting_constructors 190000
#endif

#if !defined(__cpp_initializer_lists) && (QUICKCPPLIB_GCC >= 40800)
#define __cpp_initializer_lists 190000
#endif

#if !defined(__cpp_lambdas) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_lambdas 190000
#endif

#if !defined(__cpp_nsdmi) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_nsdmi 190000
#endif

#if !defined(__cpp_range_based_for) && (QUICKCPPLIB_GCC >= 40600)
#define __cpp_range_based_for 190000
#endif

#if !defined(__cpp_raw_strings) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_raw_strings 190000
#endif

#if !defined(__cpp_ref_qualifiers) && (QUICKCPPLIB_GCC >= 40801)
#define __cpp_ref_qualifiers 190000
#endif

// __cpp_rvalue_reference deviation
#if !defined(__cpp_rvalue_references) && defined(__cpp_rvalue_reference)
#define __cpp_rvalue_references __cpp_rvalue_reference
#endif

#if !defined(__cpp_static_assert) && (QUICKCPPLIB_GCC >= 40300)
#define __cpp_static_assert 190000
#endif

#if !defined(__cpp_unicode_characters) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_unicode_characters 190000
#endif

#if !defined(__cpp_unicode_literals) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_unicode_literals 190000
#endif

#if !defined(__cpp_user_defined_literals) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_user_defined_literals 190000
#endif

#if !defined(__cpp_variadic_templates) && (QUICKCPPLIB_GCC >= 40400)
#define __cpp_variadic_templates 190000
#endif


// C++ 14
// Every C++ 14 supporting GCC does the right thing here

#endif  // __GXX_EXPERIMENTAL_CXX0X__

#endif  // GCC


// clang deviates in some places from the present SG-10 draft, plus older
// clangs are quite incomplete
#if defined(__clang__)

#define QUICKCPPLIB_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

#if !defined(__cpp_exceptions) && (defined(__EXCEPTIONS) || defined(_CPPUNWIND))
#define __cpp_exceptions 190000
#endif

#if !defined(__cpp_rtti) && (defined(__GXX_RTTI) || defined(_CPPRTTI))
#define __cpp_rtti 190000
#endif


// C++ 11
#if defined(__GXX_EXPERIMENTAL_CXX0X__)

#if !defined(__cpp_alias_templates) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_alias_templates 190000
#endif

#if !defined(__cpp_attributes) && (QUICKCPPLIB_CLANG >= 30300)
#define __cpp_attributes 190000
#endif

#if !defined(__cpp_constexpr) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_constexpr 190000
#endif

#if !defined(__cpp_decltype) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_decltype 190000
#endif

#if !defined(__cpp_delegating_constructors) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_delegating_constructors 190000
#endif

#if !defined(__cpp_explicit_conversion) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_explicit_conversion 190000
#endif

#if !defined(__cpp_inheriting_constructors) && (QUICKCPPLIB_CLANG >= 30300)
#define __cpp_inheriting_constructors 190000
#endif

#if !defined(__cpp_initializer_lists) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_initializer_lists 190000
#endif

#if !defined(__cpp_lambdas) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_lambdas 190000
#endif

#if !defined(__cpp_nsdmi) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_nsdmi 190000
#endif

#if !defined(__cpp_range_based_for) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_range_based_for 190000
#endif

// __cpp_raw_string_literals deviation
#if !defined(__cpp_raw_strings) && defined(__cpp_raw_string_literals)
#define __cpp_raw_strings __cpp_raw_string_literals
#endif
#if !defined(__cpp_raw_strings) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_raw_strings 190000
#endif

#if !defined(__cpp_ref_qualifiers) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_ref_qualifiers 190000
#endif

// __cpp_rvalue_reference deviation
#if !defined(__cpp_rvalue_references) && defined(__cpp_rvalue_reference)
#define __cpp_rvalue_references __cpp_rvalue_reference
#endif
#if !defined(__cpp_rvalue_references) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_rvalue_references 190000
#endif

#if !defined(__cpp_static_assert) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_static_assert 190000
#endif

#if !defined(__cpp_unicode_characters) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_unicode_characters 190000
#endif

#if !defined(__cpp_unicode_literals) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_unicode_literals 190000
#endif

// __cpp_user_literals deviation
#if !defined(__cpp_user_defined_literals) && defined(__cpp_user_literals)
#define __cpp_user_defined_literals __cpp_user_literals
#endif
#if !defined(__cpp_user_defined_literals) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_user_defined_literals 190000
#endif

#if !defined(__cpp_variadic_templates) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_variadic_templates 190000
#endif


// C++ 14
// Every C++ 14 supporting clang does the right thing here

#endif  // __GXX_EXPERIMENTAL_CXX0X__

#endif  // clang

#endif
