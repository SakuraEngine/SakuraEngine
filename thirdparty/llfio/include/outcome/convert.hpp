/* Says how to convert value, error and exception types
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (12 commits)
File Created: Nov 2017


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

#ifndef OUTCOME_CONVERT_HPP
#define OUTCOME_CONVERT_HPP

#include "detail/basic_result_storage.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace concepts
{
#if defined(__cpp_concepts)
#if (defined(_MSC_VER) || defined(__clang__) || (defined(__GNUC__) && __cpp_concepts >= 201707) || OUTCOME_FORCE_STD_CXX_CONCEPTS) && !OUTCOME_FORCE_LEGACY_GCC_CXX_CONCEPTS
#define OUTCOME_GCC6_CONCEPT_BOOL
#else
#define OUTCOME_GCC6_CONCEPT_BOOL bool
#endif
  namespace detail
  {
    template <class T, class U> concept OUTCOME_GCC6_CONCEPT_BOOL SameHelper = std::is_same<T, U>::value;
    template <class T, class U> concept OUTCOME_GCC6_CONCEPT_BOOL same_as = detail::SameHelper<T, U> &&detail::SameHelper<U, T>;
    template <class T, class U> concept OUTCOME_GCC6_CONCEPT_BOOL convertible = std::is_convertible<T, U>::value;
    template <class T, class U> concept OUTCOME_GCC6_CONCEPT_BOOL base_of = std::is_base_of<T, U>::value;
  }  // namespace detail


  /* The `value_or_none` concept.
  \requires That `U::value_type` exists and that `std::declval<U>().has_value()` returns a `bool` and `std::declval<U>().value()` exists.
  */
  template <class U> concept OUTCOME_GCC6_CONCEPT_BOOL value_or_none = requires(U a)
  {
    {
      a.has_value()
    }
    ->detail::same_as<bool>;
    {a.value()};
  };
  /* The `value_or_error` concept.
  \requires That `U::value_type` and `U::error_type` exist;
  that `std::declval<U>().has_value()` returns a `bool`, `std::declval<U>().value()` and  `std::declval<U>().error()` exists.
  */
  template <class U> concept OUTCOME_GCC6_CONCEPT_BOOL value_or_error = requires(U a)
  {
    {
      a.has_value()
    }
    ->detail::same_as<bool>;
    {a.value()};
    {a.error()};
  };

#else
  namespace detail
  {
    struct no_match
    {
    };
    inline no_match match_value_or_none(...);
    inline no_match match_value_or_error(...);
    OUTCOME_TEMPLATE(class U)
    OUTCOME_TREQUIRES(OUTCOME_TEXPR(std::declval<U>().has_value()), OUTCOME_TEXPR(std::declval<U>().value()))
    inline U match_value_or_none(U &&);
    OUTCOME_TEMPLATE(class U)
    OUTCOME_TREQUIRES(OUTCOME_TEXPR(std::declval<U>().has_value()), OUTCOME_TEXPR(std::declval<U>().value()), OUTCOME_TEXPR(std::declval<U>().error()))
    inline U match_value_or_error(U &&);

    template <class U>
    static constexpr bool value_or_none =
    !std::is_same<no_match, decltype(match_value_or_none(std::declval<OUTCOME_V2_NAMESPACE::detail::devoid<U>>()))>::value;
    template <class U>
    static constexpr bool value_or_error =
    !std::is_same<no_match, decltype(match_value_or_error(std::declval<OUTCOME_V2_NAMESPACE::detail::devoid<U>>()))>::value;
  }  // namespace detail
  /* The `value_or_none` concept.
  \requires That `U::value_type` exists and that `std::declval<U>().has_value()` returns a `bool` and `std::declval<U>().value()` exists.
  */
  template <class U> static constexpr bool value_or_none = detail::value_or_none<U>;
  /* The `value_or_error` concept.
  \requires That `U::value_type` and `U::error_type` exist;
  that `std::declval<U>().has_value()` returns a `bool`, `std::declval<U>().value()` and  `std::declval<U>().error()` exists.
  */
  template <class U> static constexpr bool value_or_error = detail::value_or_error<U>;
#endif
}  // namespace concepts

namespace convert
{
#if OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
#if defined(__cpp_concepts)
  template <class U> concept OUTCOME_GCC6_CONCEPT_BOOL ValueOrNone = concepts::value_or_none<U>;
  template <class U> concept OUTCOME_GCC6_CONCEPT_BOOL ValueOrError = concepts::value_or_error<U>;
#else
  template <class U> static constexpr bool ValueOrNone = concepts::value_or_none<U>;
  template <class U> static constexpr bool ValueOrError = concepts::value_or_error<U>;
#endif
#endif

  namespace detail
  {
    template <class T, class X> struct make_type
    {
      template <class U> static constexpr T value(U &&v) { return T{in_place_type<typename T::value_type>, static_cast<U &&>(v).value()}; }
      template <class U> static constexpr T error(U &&v) { return T{in_place_type<typename T::error_type>, static_cast<U &&>(v).error()}; }
      static constexpr T error() { return T{in_place_type<typename T::error_type>}; }
    };
    template <class T> struct make_type<T, void>
    {
      template <class U> static constexpr T value(U && /*unused*/) { return T{in_place_type<typename T::value_type>}; }
      template <class U> static constexpr T error(U && /*unused*/) { return T{in_place_type<typename T::error_type>}; }
      static constexpr T error() { return T{in_place_type<typename T::error_type>}; }
    };
  }  // namespace detail

  /*! AWAITING HUGO JSON CONVERSION TOOL
type definition  value_or_error. Potential doc page: NOT FOUND
*/
  template <class T, class U> struct value_or_error
  {
    static constexpr bool enable_result_inputs = false;
    static constexpr bool enable_outcome_inputs = false;
    OUTCOME_TEMPLATE(class X)
    OUTCOME_TREQUIRES(
    OUTCOME_TPRED(std::is_same<U, std::decay_t<X>>::value  //
                  &&concepts::value_or_error<U>            //
                  && (std::is_void<typename std::decay_t<X>::value_type>::value ||
                      OUTCOME_V2_NAMESPACE::detail::is_explicitly_constructible<typename T::value_type, typename std::decay_t<X>::value_type>)  //
                  &&(std::is_void<typename std::decay_t<X>::error_type>::value ||
                     OUTCOME_V2_NAMESPACE::detail::is_explicitly_constructible<typename T::error_type, typename std::decay_t<X>::error_type>) ))
    constexpr T operator()(X &&v)
    {
      return v.has_value() ? detail::make_type<T, typename T::value_type>::value(static_cast<X &&>(v)) :
                             detail::make_type<T, typename U::error_type>::error(static_cast<X &&>(v));
    }
  };
}  // namespace convert

OUTCOME_V2_NAMESPACE_END

#endif
