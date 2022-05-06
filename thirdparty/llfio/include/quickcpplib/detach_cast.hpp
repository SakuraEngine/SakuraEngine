/* C++ 23? detach and attach cast emulation.
(C) 2019 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
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

#ifndef QUICKCPPLIB_DETACH_CAST_HPP
#define QUICKCPPLIB_DETACH_CAST_HPP

#include "bit_cast.hpp"
#include "byte.hpp"
#include "span.hpp"
#include "type_traits.hpp"

#include <cstring>  // for memcpy
#include <type_traits>

// GCC has some weird parsing bug here
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 12
#define QUICKCPPLIB_DETACH_CAST_NODISCARD
#else
#define QUICKCPPLIB_DETACH_CAST_NODISCARD QUICKCPPLIB_NODISCARD
#endif

QUICKCPPLIB_NAMESPACE_BEGIN

namespace detach_cast
{
  using QUICKCPPLIB_NAMESPACE::byte::to_byte;

  using QUICKCPPLIB_NAMESPACE::bit_cast::bit_cast;
  using QUICKCPPLIB_NAMESPACE::byte::byte;

  //! Namespace for user specialised traits
  namespace traits
  {
    /*! \brief Specialise to true if you want to enable the reinterpret cast based
    implementation of `detach_cast()` for some type `T`. This introduces undefined
    behaviour in C++ 20.
    */
    template <class T> struct enable_reinterpret_detach_cast : public std::false_type
    {
    };
    /*! \brief Specialise to true if you want to enable the reinterpret cast based
    implementation of `attach_cast()` for some type `T`. This introduces undefined
    behaviour in C++ 20.
    */
    template <class T> struct enable_reinterpret_attach_cast : public std::false_type
    {
    };
  }  // namespace traits

  namespace detail
  {
    QUICKCPPLIB_TEMPLATE(class To, class From)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TEXPR(QUICKCPPLIB_NAMESPACE::bit_cast::bit_cast<To, From>(std::declval<From>())))
    inline constexpr bool _is_bit_cast_valid(int) { return true; };
    template <class To, class From>  //
    inline constexpr bool _is_bit_cast_valid(...)
    {
      return false;
    };
    template <class To, class From>  //
    inline constexpr bool is_bit_cast_valid()
    {
      return _is_bit_cast_valid<To, From>(5);
    };

    template <class T> struct byte_array_wrapper
    {
      byte value[sizeof(T)];
    };

    struct bit_castable_overload
    {
    };
    struct reinterpret_cast_overload
    {
    };
  }  // namespace detail

  //! \brief A reference to a byte array sized the same as `T`
  template <class T> using byte_array_reference = byte (&)[sizeof(T)];

  //! \brief A const reference to a byte array sized the same as `const T`
  template <class T> using const_byte_array_reference = const byte (&)[sizeof(T)];

  /*! \brief Detaches a live object into its detached byte representation, ending the
  lifetime of the input object, and beginning the lifetime of an array of byte sized
  exactly the size of the input object at the same memory location, which is returned.
  All references to the input object become INVALID. Any use of the input object after
  detachment has occurred is illegal!

  Implementation notes: If the input type is bit castable, bit casting is used to
  implement detachment using defined behaviour in C++ 20. Otherwise
  `traits::enable_reinterpret_detach_cast<T>` is used to determine whether to
  implement detachment using undefined behaviour by reinterpret casting.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(!detail::is_bit_cast_valid<detail::byte_array_wrapper<T>, T>()  //
                                          && !traits::enable_reinterpret_detach_cast<typename std::decay<T>::type>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline byte_array_reference<T> detach_cast(const T &, ...) noexcept
  {                                                                                                                                     //
    static_assert(!std::is_same<T, T>::value, "In C++ 20, detach_cast(T) is defined behaviour only for types which are bit castable. "  //
                                              "Set traits::enable_reinterpret_detach_cast<T> for specific types if you don't mind undefined behaviour.");
  }
  /*! \brief Reattaches a previously detached object, beginning the lifetime of the
  output object, and ending the lifetime of the input array of byte.
  All references to the input byte array become INVALID. Any use of the input array
  after attachment has occurred is illegal!

  Implementation notes: If the output type is bit castable, bit casting is used to
  implement attachment using defined behaviour in C++ 20. Otherwise
  `traits::enable_reinterpret_attach_cast<T>` is used to determine whether to
  implement attachment using undefined behaviour by reinterpret casting.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(!detail::is_bit_cast_valid<T, detail::byte_array_wrapper<T>>()  //
                                          && !traits::enable_reinterpret_attach_cast<typename std::decay<T>::type>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline T &attach_cast(const_byte_array_reference<T> &, ...) noexcept
  {                                                                                                                                     //
    static_assert(!std::is_same<T, T>::value, "In C++ 20, attach_cast(T) is defined behaviour only for types which are bit castable. "  //
                                              "Set traits::enable_reinterpret_attach_cast<T> for specific types if you don't mind undefined behaviour.");
  }

  /*! \brief Detaches a non-const bit-castable object into its detached non-const byte representation,
  ending the lifetime of the input object. Defined behaviour in C++ 20 (though only
  the clang compiler currently reliably does not copy the byte array twice. GCC avoids
  the memory copy for small objects, MSVC always copies the byte array twice).
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_bit_cast_valid<detail::byte_array_wrapper<T>, T>()  //
                                          && !traits::enable_reinterpret_detach_cast<typename std::decay<T>::type>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline byte_array_reference<T> detach_cast(T &v, detail::bit_castable_overload = {}) noexcept
  {
    // Bit cast and copy the input object into a stack allocated byte array. This
    // is defined behaviour for trivially copyable types.
    auto buffer(bit_cast<detail::byte_array_wrapper<T>>(v));
    // Cast input reference to output reference. Using the cast reference is defined
    // behaviour for a few special functions e.g. memcpy()
    auto &ret = reinterpret_cast<byte_array_reference<T>>(v);
    // Copy the detached byte representation back over the input storage. This ends
    // the lifetime of the input object, which is defined behaviour due to it being
    // trivially copyable. The compiler now knows that the output reference does not
    // alias the same object given by the input reference.
    memcpy(&ret, buffer.value, sizeof(T));
    // Return a reference to the byte array representing the detached object.
    return ret;
  }
  /*! \brief Detaches a const bit-castable object into its detached const byte representation,
  ending the lifetime of the input object. Defined behaviour in C++ 20 (though only
  the clang compiler currently reliably does not copy the byte array twice. GCC avoids
  the memory copy for small objects, MSVC always copies the byte array twice).
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_bit_cast_valid<const detail::byte_array_wrapper<T>, const T>()  //
                                          && !traits::enable_reinterpret_detach_cast<typename std::decay<T>::type>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline const_byte_array_reference<T> detach_cast(const T &v, detail::bit_castable_overload = {}) noexcept
  {
    const auto buffer(bit_cast<const detail::byte_array_wrapper<T>>(v));
    auto &ret = const_cast<byte_array_reference<T>>(reinterpret_cast<const_byte_array_reference<T>>(v));
    memcpy(&ret, buffer.value, sizeof(T));
    return ret;
  }
  /*! \brief Attaches a non-const bit-castable object from its detached non-const byte representation,
  ending the lifetime of the input array. Defined behaviour in C++ 20 (though only
  the clang compiler currently reliably does not copy the byte array twice. GCC avoids
  the memory copy for small objects, MSVC always copies the byte array twice).
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_bit_cast_valid<T, detail::byte_array_wrapper<T>>()                    //
                                          && !traits::enable_reinterpret_attach_cast<typename std::decay<T>::type>::value  //
                                          && !std::is_const<T>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline T &attach_cast(byte_array_reference<T> v, detail::bit_castable_overload = {}) noexcept
  {
    // Bit cast and copy the input byte array into a stack allocated object. This
    // is defined behaviour for trivially copyable types.
    T temp(bit_cast<T>(v));
    // Cast input reference to output reference. Using the cast reference is defined
    // behaviour for a few special functions e.g. memcpy()
    T &ret = reinterpret_cast<T &>(v);
    // Trivially copyable types can be memcpy()ied, this begins lifetime in the destination
    memcpy(&ret, &temp, sizeof(T));
    // Return a reference to the new object.
    return ret;
  }
  /*! \brief Attaches a const bit-castable object from its detached const byte representation,
  ending the lifetime of the input array. Defined behaviour in C++ 20 (though only
  the clang compiler currently reliably does not copy the byte array twice. GCC avoids
  the memory copy for small objects, MSVC always copies the byte array twice).
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_bit_cast_valid<T, const detail::byte_array_wrapper<T>>()              //
                                          && !traits::enable_reinterpret_attach_cast<typename std::decay<T>::type>::value  //
                                          && std::is_const<T>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline const T &attach_cast(const_byte_array_reference<T> v, detail::bit_castable_overload = {}) noexcept
  {
    using nonconst = typename std::remove_const<T>::type;
    T temp(bit_cast<nonconst>(v));
    nonconst &ret = const_cast<nonconst &>(reinterpret_cast<T &>(v));
    memcpy(&ret, &temp, sizeof(T));
    return ret;
  }

  /*! \brief Reinterpret casts a non-const object reference into a non-const byte representation.
  Pure undefined behaviour. Available only if `traits::enable_reinterpret_detach_cast<T>` is true
  for the type.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(!detail::is_bit_cast_valid<detail::byte_array_wrapper<T>, T>()  //
                                          && traits::enable_reinterpret_detach_cast<typename std::decay<T>::type>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline byte_array_reference<T> detach_cast(T &v, detail::reinterpret_cast_overload = {}) noexcept { return reinterpret_cast<byte_array_reference<T>>(v); }
  /*! \brief Reinterpret casts a const object reference into a const byte representation.
  Pure undefined behaviour. Available only if `traits::enable_reinterpret_detach_cast<T>` is true
  for the type.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(!detail::is_bit_cast_valid<const detail::byte_array_wrapper<T>, const T>()  //
                                          && traits::enable_reinterpret_detach_cast<typename std::decay<T>::type>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline const_byte_array_reference<T> detach_cast(const T &v, detail::reinterpret_cast_overload = {}) noexcept { return reinterpret_cast<const_byte_array_reference<T>>(v); }
  /*! \brief Reinterpret casts a const byte representation into a const object.
  Pure undefined behaviour. Available only if `traits::enable_reinterpret_attach_cast<T>` is true
  for the type.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(!detail::is_bit_cast_valid<T, detail::byte_array_wrapper<T>>()                  //
                                          && traits::enable_reinterpret_attach_cast<typename std::decay<T>::type>::value  //
                                          && !std::is_const<T>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline T &attach_cast(byte_array_reference<T> v, detail::reinterpret_cast_overload = {}) noexcept { return reinterpret_cast<T &>(v); }
  /*! \brief Reinterpret casts a non-const byte representation into a non-const object.
  Pure undefined behaviour. Available only if `traits::enable_reinterpret_attach_cast<T>` is true
  for the type.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(!detail::is_bit_cast_valid<const T, const detail::byte_array_wrapper<T>>()      //
                                          && traits::enable_reinterpret_attach_cast<typename std::decay<T>::type>::value  //
                                          && std::is_const<T>::value))
  QUICKCPPLIB_DETACH_CAST_NODISCARD constexpr inline T &attach_cast(const_byte_array_reference<T> v, detail::reinterpret_cast_overload = {}) noexcept { return reinterpret_cast<T &>(v); }

}  // namespace detach_cast

QUICKCPPLIB_NAMESPACE_END

#endif
