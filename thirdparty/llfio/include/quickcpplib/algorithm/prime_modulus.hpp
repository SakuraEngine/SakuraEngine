/* Fixed overhead twos power prime modulus
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
File Created: Aug 2019


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

#ifndef QUICKCPPLIB_ALGORITHM_PRIME_MODULUS_HPP
#define QUICKCPPLIB_ALGORITHM_PRIME_MODULUS_HPP

#include "../config.hpp"

#include <cstdint>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace algorithm
{
  namespace prime_modulus
  {
    //! \brief Returns a constexpr prime just under a twos power e.g. for 8, you get 251, which is the prime just below (1<<8) = 256. Very handy for hash tables.
    constexpr inline uint64_t twos_power_prime(size_t power) noexcept
    {
      // Primes come from https://primes.utm.edu/lists/2small/0bit.html
      // There is a fascinatingly close relationship between powers of two and prime numbers,
      // as is obvious from the table below. See https://en.wikipedia.org/wiki/Mersenne_prime/.
      switch(power)
      {
      case 0:
        return 1;
      case 1:
        return 2;
      case 2:
        return (1ULL << 2) - 1;
      case 3:
        return (1ULL << 3) - 1;
      case 4:
        return (1ULL << 4) - 3;
      case 5:
        return (1ULL << 5) - 1;
      case 6:
        return (1ULL << 6) - 3;
      case 7:
        return (1ULL << 7) - 1;
      case 8:
        return (1ULL << 8) - 5;
      case 9:
        return (1ULL << 9) - 3;
      case 10:
        return (1ULL << 10) - 3;
      case 11:
        return (1ULL << 11) - 9;
      case 12:
        return (1ULL << 12) - 3;
      case 13:
        return (1ULL << 13) - 1;
      case 14:
        return (1ULL << 14) - 3;
      case 15:
        return (1ULL << 15) - 19;
      case 16:
        return (1ULL << 16) - 15;  //
      case 17:
        return (1ULL << 17) - 1;
      case 18:
        return (1ULL << 18) - 5;
      case 19:
        return (1ULL << 19) - 1;
      case 20:
        return (1ULL << 20) - 3;
      case 21:
        return (1ULL << 21) - 9;
      case 22:
        return (1ULL << 22) - 3;
      case 23:
        return (1ULL << 23) - 15;
      case 24:
        return (1ULL << 24) - 3;
      case 25:
        return (1ULL << 25) - 39;
      case 26:
        return (1ULL << 26) - 5;
      case 27:
        return (1ULL << 27) - 39;
      case 28:
        return (1ULL << 28) - 57;
      case 29:
        return (1ULL << 29) - 3;
      case 30:
        return (1ULL << 30) - 35;
      case 31:
        return (1ULL << 31) - 1;  //
      case 32:
        return (1ULL << 32) - 5;
      case 33:
        return (1ULL << 33) - 9;
      case 34:
        return (1ULL << 34) - 41;
      case 35:
        return (1ULL << 35) - 31;
      case 36:
        return (1ULL << 36) - 5;
      case 37:
        return (1ULL << 37) - 25;
      case 38:
        return (1ULL << 38) - 45;
      case 39:
        return (1ULL << 39) - 7;
      case 40:
        return (1ULL << 40) - 87;
      case 41:
        return (1ULL << 41) - 21;
      case 42:
        return (1ULL << 42) - 11;
      case 43:
        return (1ULL << 43) - 57;
      case 44:
        return (1ULL << 44) - 17;
      case 45:
        return (1ULL << 45) - 55;
      case 46:
        return (1ULL << 46) - 21;
      case 47:
        return (1ULL << 47) - 115;
      case 48:
        return (1ULL << 48) - 59;  //
      case 49:
        return (1ULL << 49) - 81;
      case 50:
        return (1ULL << 50) - 27;
      case 51:
        return (1ULL << 51) - 129;
      case 52:
        return (1ULL << 52) - 47;
      case 53:
        return (1ULL << 53) - 111;
      case 54:
        return (1ULL << 54) - 33;
      case 55:
        return (1ULL << 55) - 55;
      case 56:
        return (1ULL << 56) - 5;
      case 57:
        return (1ULL << 57) - 13;
      case 58:
        return (1ULL << 58) - 27;
      case 59:
        return (1ULL << 59) - 55;
      case 60:
        return (1ULL << 60) - 93;
      case 61:
        return (1ULL << 61) - 1;
      case 62:
        return (1ULL << 62) - 57;
      case 63:
        return (1ULL << 63) - 25;
      case 64:
        return (0xffffffffffffffffULL - 58);  //(1ULL << 64) - 59
      default:
        return 0;
      }
    }

    namespace detail
    {
      template <uint32_t power, class T> constexpr inline T mod(T v) { return v % twos_power_prime(power); }
    }  // namespace detail

    /*! \brief Return the modulus of a number by the prime just below a twos power.
    Implemented as a fixed jump table, so the compiler can avoid the CPU division opcode,
    which still costs 40-90 CPU cycles, and excludes all CPU level parallelism.

    Example: prime_modulus(1234, 8) is 1234 % 251 = 230. 251 is the next prime number
    below (1<<8) = 256.
    */
    template <class T> constexpr inline T prime_modulus(T v, uint32_t power)
    {
      using detail::mod;
      switch(power)
      {
      case 0:
        return mod<0>(v);
      case 1:
        return mod<1>(v);
      case 2:
        return mod<2>(v);
      case 3:
        return mod<3>(v);
      case 4:
        return mod<4>(v);
      case 5:
        return mod<5>(v);
      case 6:
        return mod<6>(v);
      case 7:
        return mod<7>(v);
      case 8:
        return mod<8>(v);
      case 9:
        return mod<9>(v);
      case 10:
        return mod<10>(v);
      case 11:
        return mod<11>(v);
      case 12:
        return mod<12>(v);
      case 13:
        return mod<13>(v);
      case 14:
        return mod<14>(v);
      case 15:
        return mod<15>(v);
      case 16:
        return mod<16>(v);
      case 17:
        return mod<17>(v);
      case 18:
        return mod<18>(v);
      case 19:
        return mod<19>(v);
      case 20:
        return mod<20>(v);
      case 21:
        return mod<21>(v);
      case 22:
        return mod<22>(v);
      case 23:
        return mod<23>(v);
      case 24:
        return mod<24>(v);
      case 25:
        return mod<25>(v);
      case 26:
        return mod<26>(v);
      case 27:
        return mod<27>(v);
      case 28:
        return mod<28>(v);
      case 29:
        return mod<29>(v);
      case 30:
        return mod<30>(v);
      case 31:
        return mod<31>(v);
      case 32:
        return mod<32>(v);
      case 33:
        return mod<33>(v);
      case 34:
        return mod<34>(v);
      case 35:
        return mod<35>(v);
      case 36:
        return mod<36>(v);
      case 37:
        return mod<37>(v);
      case 38:
        return mod<38>(v);
      case 39:
        return mod<39>(v);
      case 40:
        return mod<40>(v);
      case 41:
        return mod<41>(v);
      case 42:
        return mod<42>(v);
      case 43:
        return mod<43>(v);
      case 44:
        return mod<44>(v);
      case 45:
        return mod<45>(v);
      case 46:
        return mod<46>(v);
      case 47:
        return mod<47>(v);
      case 48:
        return mod<48>(v);
      case 49:
        return mod<49>(v);
      case 50:
        return mod<50>(v);
      case 51:
        return mod<51>(v);
      case 52:
        return mod<52>(v);
      case 53:
        return mod<53>(v);
      case 54:
        return mod<54>(v);
      case 55:
        return mod<55>(v);
      case 56:
        return mod<56>(v);
      case 57:
        return mod<57>(v);
      case 58:
        return mod<58>(v);
      case 59:
        return mod<59>(v);
      case 60:
        return mod<60>(v);
      case 61:
        return mod<61>(v);
      case 62:
        return mod<62>(v);
      case 63:
        return mod<63>(v);
      case 64:
        return mod<64>(v);
      default:
        return 0;
      }
    }
  }  // namespace prime_modulus
}  // namespace algorithm

QUICKCPPLIB_NAMESPACE_END

#endif
