/* Erasure cast extending bit cast.
(C) 2018 - 2019 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: May 2019


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

#ifndef QUICKCPPLIB_ERASURE_CAST_HPP
#define QUICKCPPLIB_ERASURE_CAST_HPP

#include "bit_cast.hpp"

QUICKCPPLIB_NAMESPACE_BEGIN

namespace erasure_cast
{
  using bit_cast::bit_cast;
  namespace traits = bit_cast::traits;
  namespace detail
  {
    using namespace bit_cast::detail;
    /* A partially compliant implementation of C++20's std::bit_cast function contributed
    by Jesse Towner.
    erasure_cast performs a bit_cast with additional rules to handle types
    of differing sizes. For integral & enum types, it may perform a narrowing
    or widing conversion with static_cast if necessary, before doing the final
    conversion with bit_cast. When casting to or from non-integral, non-enum
    types it may insert the value into another object with extra padding bytes
    to satisfy bit_cast's preconditions that both types have the same size. */

    template <class To, class From> using is_erasure_castable = std::integral_constant<bool, traits::is_move_relocating<To>::value && traits::is_move_relocating<From>::value>;

    template <class T, bool = std::is_enum<T>::value> struct identity_or_underlying_type
    {
      using type = T;
    };
    template <class T> struct identity_or_underlying_type<T, true>
    {
      using type = typename std::underlying_type<T>::type;
    };

    template <class OfSize, class OfSign>
    using erasure_integer_type = typename std::conditional<std::is_signed<typename identity_or_underlying_type<OfSign>::type>::value, typename std::make_signed<typename identity_or_underlying_type<OfSize>::type>::type, typename std::make_unsigned<typename identity_or_underlying_type<OfSize>::type>::type>::type;

    template <class ErasedType, std::size_t N> struct padded_erasure_object
    {
      static_assert(traits::is_move_relocating<ErasedType>::value, "ErasedType must be TriviallyCopyable or MoveRelocating");
      static_assert(alignof(ErasedType) <= sizeof(ErasedType), "ErasedType must not be over-aligned");
      ErasedType value;
      char padding[N];
      constexpr explicit padded_erasure_object(const ErasedType &v) noexcept
          : value(v)
          , padding{}
      {
      }
    };

    struct bit_cast_equivalence_overload
    {
    };
    struct static_cast_dest_smaller_overload
    {
    };
    struct static_cast_dest_larger_overload
    {
    };
    struct union_cast_dest_smaller_overload
    {
    };
    struct union_cast_dest_larger_overload
    {
    };
  }  // namespace detail

  /*! \brief Erasure cast implementation chosen if types are move relocating or trivally copyable,
  have identical size, and are bit castable. Constexpr. Forwards to `bit_cast()` directly.
  */
  QUICKCPPLIB_TEMPLATE(class To, class From)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_erasure_castable<To, From>::value  //
                                          && (sizeof(To) == sizeof(From))))
  constexpr inline To erasure_cast(const From &from, detail::bit_cast_equivalence_overload = {}) noexcept { return bit_cast<To>(from); }

  /*! \brief Erasure cast implementation chosen if types are move relocating or trivally copyable,
  are statically castable, and destination type is smaller than source type. Constexpr.
  */
  QUICKCPPLIB_TEMPLATE(class To, class From)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_erasure_castable<To, From>::value   //
                                          &&detail::is_static_castable<To, From>::value  //
                                          && (sizeof(To) < sizeof(From))))
  constexpr inline To erasure_cast(const From &from, detail::static_cast_dest_smaller_overload = {}) noexcept { return static_cast<To>(bit_cast<detail::erasure_integer_type<From, To>>(from)); }

  /*! \brief Erasure cast implementation chosen if types are move relocating or trivally copyable,
  are statically castable, and destination type is larger than source type. Constexpr.
  */
  QUICKCPPLIB_TEMPLATE(class To, class From)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_erasure_castable<To, From>::value   //
                                          &&detail::is_static_castable<To, From>::value  //
                                          && (sizeof(To) > sizeof(From))))
  constexpr inline To erasure_cast(const From &from, detail::static_cast_dest_larger_overload = {}) noexcept { return bit_cast<To>(static_cast<detail::erasure_integer_type<To, From>>(from)); }

  /*! \brief Erasure cast implementation chosen if types are move relocating or trivally copyable,
  are union castable, and destination type is smaller than source type. May be constexpr if
  underlying bit cast is constexpr.
  */
  QUICKCPPLIB_TEMPLATE(class To, class From)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_erasure_castable<To, From>::value     //
                                          && !detail::is_static_castable<To, From>::value  //
                                          && (sizeof(To) < sizeof(From))))
  constexpr inline To erasure_cast(const From &from, detail::union_cast_dest_smaller_overload = {}) noexcept { return bit_cast<detail::padded_erasure_object<To, sizeof(From) - sizeof(To)>>(from).value; }

  /*! \brief Erasure cast implementation chosen if types are move relocating or trivally copyable,
  are union castable, and destination type is larger than source type. May be constexpr if
  underlying bit cast is constexpr.
  */
  QUICKCPPLIB_TEMPLATE(class To, class From)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_erasure_castable<To, From>::value     //
                                          && !detail::is_static_castable<To, From>::value  //
                                          && (sizeof(To) > sizeof(From))))
  constexpr inline To erasure_cast(const From &from, detail::union_cast_dest_larger_overload = {}) noexcept { return bit_cast<To>(detail::padded_erasure_object<From, sizeof(To) - sizeof(From)>{from}); }
}  // namespace erasure_cast

QUICKCPPLIB_NAMESPACE_END

#endif
