/* 128 bit integer support
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: Sept 2016


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

#ifndef QUICKCPPLIB_UINT128_HPP
#define QUICKCPPLIB_UINT128_HPP

#include "config.hpp"

#include <cstdint>
#include <stdexcept>  // for std::domain_error

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#include <emmintrin.h>  // for __m128i on VS2017
#endif

QUICKCPPLIB_NAMESPACE_BEGIN

namespace integers128
{
  /*! \union uint128
  \brief An unsigned 128 bit value
  */
  union alignas(16) uint128 {
    struct empty_type
    {
    } _empty;
    uint8_t as_bytes[16];
    uint16_t as_shorts[8];
    uint32_t as_ints[4];
    uint64_t as_longlongs[2];
// Strongly hint to the compiler what to do here
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    __m128i as_m128i;
#endif
#if defined(__GNUC__) || defined(__clang__)
    typedef unsigned uint32_4_t __attribute__((vector_size(16)));
    uint32_4_t as_uint32_4;
#endif
    //! Default constructor, no bits set
    constexpr uint128() noexcept : _empty() {}
    //! Construct from a number
    constexpr uint128(uint64_t v) noexcept : as_longlongs{v, 0} {}
    //! Construct from input
    constexpr uint128(uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5, uint8_t v6, uint8_t v7, uint8_t v8, uint8_t v9, uint8_t v10, uint8_t v11, uint8_t v12, uint8_t v13, uint8_t v14, uint8_t v15) noexcept : as_bytes{v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15} {}
    //! Construct from input
    constexpr uint128(uint16_t v0, uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4, uint16_t v5, uint16_t v6, uint16_t v7) noexcept : as_shorts{v0, v1, v2, v3, v4, v5, v6, v7} {}
    //! Construct from input
    constexpr uint128(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3) noexcept : as_ints{v0, v1, v2, v3} {}
    //! Construct from input
    constexpr uint128(uint64_t v0, uint64_t v1) noexcept : as_longlongs{v0, v1} {}
    //! Return the bottom unsigned short bits of the number
    explicit operator unsigned short() const noexcept { return static_cast<unsigned short>(as_longlongs[0]); }
    //! Return the bottom unsigned bits of the number
    explicit operator unsigned() const noexcept { return static_cast<unsigned>(as_longlongs[0]); }
    //! Return the bottom long bits of the number
    explicit operator unsigned long() const noexcept { return static_cast<unsigned long>(as_longlongs[0]); }
    //! Return the bottom long long bits of the number
    explicit operator unsigned long long() const noexcept { return static_cast<unsigned long long>(as_longlongs[0]); }
  private:
    static const uint128 &_allbitszero()
    {
      static uint128 v(0);
      return v;
    }

  public:
    explicit operator bool() const noexcept { return (*this) != _allbitszero(); }
    bool operator!() const noexcept { return (*this) == _allbitszero(); }
    // uint128 operator~() const noexcept;
    // uint128 operator++() noexcept;
    // uint128 operator++(int) noexcept;
    // uint128 operator--() noexcept;
    // uint128 operator--(int) noexcept;

    uint128 operator+(const uint128 &v) const noexcept
    {
      uint128 t(*this);
      t += v;
      return t;
    }
    uint128 operator+=(const uint128 &v) noexcept
    {
// Produces wrong result on GCC
#if 0  // defined(__GNUC__) || defined(__clang__)
      as_uint32_4 += v.as_uint32_4;
      return *this;
#endif
      auto o = as_longlongs[0];
      as_longlongs[0] += v.as_longlongs[0];
      as_longlongs[1] += v.as_longlongs[1];
      as_longlongs[1] += (as_longlongs[0] < o);
      return *this;
    }
    uint128 operator-(const uint128 &v) const noexcept
    {
      uint128 t(*this);
      t -= v;
      return t;
    }
    uint128 operator-=(const uint128 &v) noexcept
    {
// Produces wrong result on GCC
#if 0  // defined(__GNUC__) || defined(__clang__)
      as_uint32_4 += v.as_uint32_4;
      return *this;
#endif
      auto o = as_longlongs[0];
      as_longlongs[0] -= v.as_longlongs[0];
      as_longlongs[1] -= v.as_longlongs[1];
      as_longlongs[1] -= (as_longlongs[0] > o);
      return *this;
    }
    // uint128 operator*(uint128 v) const noexcept;
    // uint128 operator*=(uint128 v) noexcept;
    // uint128 operator/(uint128 v) const noexcept;
    // uint128 operator/=(uint128 v) noexcept;
    uint128 operator%(const uint128 &v) const noexcept
    {
      uint128 t(*this);
      t %= v;
      return t;
    }
    uint128 operator%=(const uint128 &b)
    {
      if(!b)
        throw std::domain_error("divide by zero");
// Looks like this fails with SIGFPE on both GCC and clang no matter what
#if 0  // defined(__GNUC__) || defined(__clang__)
      as_uint32_4 %= b.as_uint32_4;
      return *this;
#endif

      uint128 x(b), y(*this >> 1);
      while(x <= y)
      {
        x <<= 1;
      }
      while(*this >= b)
      {
        if(*this >= x)
          *this -= x;
        x >>= 1;
      }
      return *this;
    }
#if 0  // actually slower than the 128 bit version, believe it or not. Modern optimisers are amazing!
    uint32_t operator%(uint32_t b)
    {
      if(!b)
        throw std::domain_error("divide by zero");
// Looks like this fails with SIGFPE on both GCC and clang no matter what
#if 0  // defined(__GNUC__) || defined(__clang__)
      as_uint32_4 %= b;
      return *this;
#endif
      uint64_t result = 0;
      uint64_t a = (~0 % b) + 1;
      as_longlongs[1] %= b;

      while(as_longlongs[1] != 0)
      {
        if((as_longlongs[1] & 1) == 1)
        {
          result += a;
          if(result >= b)
          {
            result -= b;
          }
        }
        a <<= 1;
        if(a >= b)
        {
          a -= b;
        }
        as_longlongs[1] >>= 1;
      }
      if(as_longlongs[0] > b)
      {
        as_longlongs[0] -= b;
      }
      return (as_longlongs[0] + result) % b;
    }
#endif
    // uint128 operator&(uint128 v) const noexcept;
    // uint128 operator&=(uint128 v) noexcept;
    // uint128 operator|(uint128 v) const noexcept;
    // uint128 operator|=(uint128 v) noexcept;
    // uint128 operator^(uint128 v) const noexcept;
    // uint128 operator^=(uint128 v) noexcept;
    uint128 operator<<(uint8_t v) const noexcept
    {
      uint128 t(*this);
      t <<= v;
      return t;
    }
    uint128 operator<<=(uint8_t v) noexcept
    {
#if 0  // defined(__GNUC__) || defined(__clang__)
      as_uint32_4 <<= v;
      return *this;
#endif
      as_longlongs[1] <<= v;
      as_longlongs[1] |= as_longlongs[0] >> (64 - v);
      as_longlongs[0] <<= v;
      return *this;
    }
    uint128 operator>>(uint8_t v) const noexcept
    {
      uint128 t(*this);
      t >>= v;
      return t;
    }
    uint128 operator>>=(uint8_t v) noexcept
    {
#if 0  // defined(__GNUC__) || defined(__clang__)
      as_uint32_4 >>= v;
      return *this;
#endif
      as_longlongs[0] >>= v;
      as_longlongs[0] |= as_longlongs[1] << (64 - v);
      as_longlongs[1] >>= v;
      return *this;
    }

    bool operator==(const uint128 &o) const noexcept { return as_longlongs[1] == o.as_longlongs[1] && as_longlongs[0] == o.as_longlongs[0]; }
    bool operator!=(const uint128 &o) const noexcept { return as_longlongs[1] != o.as_longlongs[1] || as_longlongs[0] != o.as_longlongs[0]; }
    bool operator<(const uint128 &o) const noexcept { return as_longlongs[1] < o.as_longlongs[1] || (as_longlongs[1] == o.as_longlongs[1] && as_longlongs[0] < o.as_longlongs[0]); }
    bool operator<=(const uint128 &o) const noexcept { return as_longlongs[1] < o.as_longlongs[1] || (as_longlongs[1] == o.as_longlongs[1] && as_longlongs[0] <= o.as_longlongs[0]); }
    bool operator>(const uint128 &o) const noexcept { return as_longlongs[1] > o.as_longlongs[1] || (as_longlongs[1] == o.as_longlongs[1] && as_longlongs[0] > o.as_longlongs[0]); }
    bool operator>=(const uint128 &o) const noexcept { return as_longlongs[1] > o.as_longlongs[1] || (as_longlongs[1] == o.as_longlongs[1] && as_longlongs[0] >= o.as_longlongs[0]); }
  };
  static_assert(sizeof(uint128) == 16, "uint128 is not 16 bytes long!");
  static_assert(alignof(uint128) == 16, "uint128 is not aligned to 16 byte multiples!");
  //! \brief Hashes a uint128
  struct uint128_hasher
  {
    size_t operator()(const uint128 &v) const { return (size_t)(v.as_longlongs[0] ^ v.as_longlongs[1]); }
  };
}

QUICKCPPLIB_NAMESPACE_END

#endif
