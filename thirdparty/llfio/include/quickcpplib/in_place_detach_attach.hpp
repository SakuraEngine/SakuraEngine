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

#ifndef QUICKCPPLIB_IN_PLACE_DETACH_ATTACH_HPP
#define QUICKCPPLIB_IN_PLACE_DETACH_ATTACH_HPP

#include "detach_cast.hpp"

QUICKCPPLIB_NAMESPACE_BEGIN

namespace in_place_attach_detach
{
  using QUICKCPPLIB_NAMESPACE::detach_cast::attach_cast;
  using QUICKCPPLIB_NAMESPACE::detach_cast::detach_cast;

  //! Namespace for user specialised traits
  namespace traits
  {
    /*! \brief Specialise to true if you want to `attached<>` to do nothing
    when constructed from some source `T`.
    */
    template <class T> struct disable_attached_for : public std::false_type
    {
    };
  }  // namespace traits

  namespace detail
  {
    template <class T> using byte_array_wrapper = QUICKCPPLIB_NAMESPACE::detach_cast::detail::byte_array_wrapper<T>;
    struct default_cast_operator_overload
    {
    };
  }  // namespace detail

  /*! \brief An ADL customisation point for the in-place detachment of an array of
  live `T` objects into an array of bytes representing their detached object representations.
  This overload is available if `detach_cast()` is available for `T`.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TEXPR(detach_cast(std::declval<T>())))
  constexpr inline auto in_place_detach(QUICKCPPLIB_NAMESPACE::span::span<T> objects, detail::default_cast_operator_overload = {}) noexcept
  {
    using byte_type = typename std::conditional<std::is_const<T>::value, const QUICKCPPLIB_NAMESPACE::byte::byte, QUICKCPPLIB_NAMESPACE::byte::byte>::type;
    QUICKCPPLIB_NAMESPACE::span::span<byte_type> ret;
    if(objects.empty())
    {
      ret = QUICKCPPLIB_NAMESPACE::span::span<byte_type>((byte_type *) objects.data(), (byte_type *) objects.data());
      return ret;
    }
    for(size_t n = 0; n < objects.size(); n++)
    {
      auto &d = detach_cast(objects[n]);
      if(0 == n)
      {
        ret = {d, sizeof(d)};
      }
      else
      {
        ret = {ret.data(), ret.size() + sizeof(d)};
      }
    }
    return ret;
  }

  /*! \brief An ADL customisation point for the in-place attachment of previously detached
  object representations, back into an array of live `T` objects.
  This overload is available if `attach_cast()` is available for `T`.
  */
  QUICKCPPLIB_TEMPLATE(class T)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TEXPR(attach_cast<T>(std::declval<QUICKCPPLIB_NAMESPACE::detach_cast::byte_array_reference<T>>())))
  constexpr inline QUICKCPPLIB_NAMESPACE::span::span<T> in_place_attach(QUICKCPPLIB_NAMESPACE::span::span<QUICKCPPLIB_NAMESPACE::byte::byte> bytearray, detail::default_cast_operator_overload = {}) noexcept
  {
    QUICKCPPLIB_NAMESPACE::span::span<detail::byte_array_wrapper<T>> input = {(detail::byte_array_wrapper<T> *) bytearray.data(), bytearray.size() / sizeof(T)};
    QUICKCPPLIB_NAMESPACE::span::span<T> ret;
    if(bytearray.empty())
    {
      ret = QUICKCPPLIB_NAMESPACE::span::span<T>((T *) bytearray.data(), (T *) bytearray.data());
      return ret;
    }
    for(size_t n = 0; n < input.size(); n++)
    {
      T &a = attach_cast<T>(input[n].value);
      if(0 == n)
      {
        ret = {&a, 1};
      }
      else
      {
        ret = {ret.data(), ret.size() + 1};
      }
    }
    return ret;
  }

  struct adopt_t
  {
  };
  constexpr adopt_t adopt{};

  /*! \brief An RAII refinement of `span<T>` for automatically calling `in_place_attach()`
  and `in_place_detach()` on an input array of `T`. Move-only, detaches only on final
  object destruction.
  */
  template <class T> class attached : protected QUICKCPPLIB_NAMESPACE::span::span<T>
  {
    using _base = QUICKCPPLIB_NAMESPACE::span::span<T>;
    
    bool _disabled{false};

  public:
    //! The index type
    //using index_type = typename _base::index_type;
    //! The element type
    using element_type = typename _base::element_type;
    //! The value type
    using value_type = typename _base::value_type;
    //! The reference type
    using reference = typename _base::reference;
    //! The pointer type
    using pointer = typename _base::pointer;
    //! The const reference type
    using const_reference = typename _base::const_reference;
    //! The const pointer type
    using const_pointer = typename _base::const_pointer;
    //! The iterator type
    using iterator = typename _base::iterator;
    //! The const iterator type
    //using const_iterator = typename _base::const_iterator;
    //! The reverse iterator type
    using reverse_iterator = typename _base::reverse_iterator;
    //! The const reverse iterator type
    //using const_reverse_iterator = typename _base::const_reverse_iterator;
    //! The difference type
    using difference_type = typename _base::difference_type;

  public:
    //! Default constructor
    constexpr attached() {}  // NOLINT

    attached(const attached &) = delete;
    //! Move constructs the instance, leaving the source empty.
    constexpr attached(attached &&o) noexcept : _base(std::move(o)), _disabled{o._disabled} { static_cast<_base &>(o) = {}; }
    attached &operator=(const attached &) = delete;
    constexpr attached &operator=(attached &&o) noexcept
    {
      this->~attached();
      new(this) attached(std::move(o));
      return *this;
    }

    //! Detaches the array of `T`, if not empty
    ~attached()
    {
      if(!this->empty() && !_disabled)
      {
        in_place_detach(as_span());
      }
    }

    //! Implicitly construct from anything for which `in_place_attach<T>()` is valid.
    QUICKCPPLIB_TEMPLATE(class U)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TEXPR(in_place_attach<T>(std::declval<U>())))
    constexpr attached(U &&v)
        : _base(in_place_attach<T>(static_cast<U &&>(v))), _disabled(traits::disable_attached_for<std::decay_t<U>>::value)
    {
    }
    //! Explicitly construct from a span of already attached objects
    explicit constexpr attached(adopt_t /*unused*/, _base v)
        : _base(v)
    {
    }

    //! Returns the attached region as a plain span of `T`
    constexpr _base as_span() const noexcept { return *this; }

    using _base::empty;
    using _base::first;
    using _base::last;
    using _base::size;
    using _base::size_bytes;
    //using _base::ssize;
    using _base::subspan;
    using _base::operator[];
    //using _base::operator();
    //using _base::at;
    using _base::begin;
    //using _base::cbegin;
    //using _base::cend;
    //using _base::crbegin;
    //using _base::crend;
    using _base::data;
    using _base::end;
    using _base::rbegin;
    using _base::rend;
    //using _base::swap;
  };

}  // namespace in_place_attach_detach

QUICKCPPLIB_NAMESPACE_END

#endif
