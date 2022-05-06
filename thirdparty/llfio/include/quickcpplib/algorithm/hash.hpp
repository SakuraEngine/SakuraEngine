/* Hash algorithms
(C) 2016-2021 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
File Created: Aug 2016


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

#ifndef QUICKCPPLIB_ALGORITHM_HASH_HPP
#define QUICKCPPLIB_ALGORITHM_HASH_HPP

#include "../uint128.hpp"
#include "memory.hpp"

#include <cassert>
#include <type_traits>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace algorithm
{
  namespace hash
  {
    //! \brief A STL compatible hash which passes through its input
    template <class T> struct passthru_hash
    {
      size_t operator()(T v) const { return static_cast<size_t>(v); }
    };

    //! \brief A STL compatible hash based on the high quality FNV1 hash algorithm
    template <class T> struct fnv1a_hash
    {
      size_t operator()(T v) const
      {
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__ia64__) || defined(_M_IA64) || defined(__ppc64__)
        static constexpr size_t basis = 14695981039346656037ULL, prime = 1099511628211ULL;
        static_assert(sizeof(size_t) == 8, "size_t is not 64 bit");
#else
        static constexpr size_t basis = 2166136261U, prime = 16777619U;
        static_assert(sizeof(size_t) == 4, "size_t is not 32 bit");
#endif
        const unsigned char *_v = (const unsigned char *) &v;
        size_t ret = basis;
        for(size_t n = 0; n < sizeof(T); n++)
        {
          ret ^= (size_t) _v[n];
          ret *= prime;
        }
        return ret;
      }
    };

    namespace fash_hash_detail
    {
      using uint8 = unsigned char;
      using uint32 = unsigned int;
      using uint64 = unsigned long long;
      using uint128 = integers128::uint128;
#if(defined(mips) || defined(__mips__) || defined(MIPS) || defined(_MIPS_) || defined(__mips64)) ||                                                            \
(defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__))
      // These require aligned reads
      static constexpr bool ALLOW_UNALIGNED_READS = false;
#else
      static constexpr bool ALLOW_UNALIGNED_READS = !QUICKCPPLIB_IN_UNDEFINED_SANITIZER;
#endif

      static inline QUICKCPPLIB_FORCEINLINE uint64 Rot64(uint64 x, int k) { return (x << k) | (x >> (64 - k)); }
      static inline QUICKCPPLIB_FORCEINLINE void Mix(const uint64 *data, uint64 &s0, uint64 &s1, uint64 &s2, uint64 &s3, uint64 &s4, uint64 &s5, uint64 &s6,
                                                     uint64 &s7, uint64 &s8, uint64 &s9, uint64 &s10, uint64 &s11)
      {
        s0 += data[0];
        s2 ^= s10;
        s11 ^= s0;
        s0 = Rot64(s0, 11);
        s11 += s1;
        s1 += data[1];
        s3 ^= s11;
        s0 ^= s1;
        s1 = Rot64(s1, 32);
        s0 += s2;
        s2 += data[2];
        s4 ^= s0;
        s1 ^= s2;
        s2 = Rot64(s2, 43);
        s1 += s3;
        s3 += data[3];
        s5 ^= s1;
        s2 ^= s3;
        s3 = Rot64(s3, 31);
        s2 += s4;
        s4 += data[4];
        s6 ^= s2;
        s3 ^= s4;
        s4 = Rot64(s4, 17);
        s3 += s5;
        s5 += data[5];
        s7 ^= s3;
        s4 ^= s5;
        s5 = Rot64(s5, 28);
        s4 += s6;
        s6 += data[6];
        s8 ^= s4;
        s5 ^= s6;
        s6 = Rot64(s6, 39);
        s5 += s7;
        s7 += data[7];
        s9 ^= s5;
        s6 ^= s7;
        s7 = Rot64(s7, 57);
        s6 += s8;
        s8 += data[8];
        s10 ^= s6;
        s7 ^= s8;
        s8 = Rot64(s8, 55);
        s7 += s9;
        s9 += data[9];
        s11 ^= s7;
        s8 ^= s9;
        s9 = Rot64(s9, 54);
        s8 += s10;
        s10 += data[10];
        s0 ^= s8;
        s9 ^= s10;
        s10 = Rot64(s10, 22);
        s9 += s11;
        s11 += data[11];
        s1 ^= s9;
        s10 ^= s11;
        s11 = Rot64(s11, 46);
        s10 += s0;
      }
      static inline QUICKCPPLIB_FORCEINLINE void EndPartial(uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3, uint64 &h4, uint64 &h5, uint64 &h6, uint64 &h7,
                                                            uint64 &h8, uint64 &h9, uint64 &h10, uint64 &h11)
      {
        h11 += h1;
        h2 ^= h11;
        h1 = Rot64(h1, 44);
        h0 += h2;
        h3 ^= h0;
        h2 = Rot64(h2, 15);
        h1 += h3;
        h4 ^= h1;
        h3 = Rot64(h3, 34);
        h2 += h4;
        h5 ^= h2;
        h4 = Rot64(h4, 21);
        h3 += h5;
        h6 ^= h3;
        h5 = Rot64(h5, 38);
        h4 += h6;
        h7 ^= h4;
        h6 = Rot64(h6, 33);
        h5 += h7;
        h8 ^= h5;
        h7 = Rot64(h7, 10);
        h6 += h8;
        h9 ^= h6;
        h8 = Rot64(h8, 13);
        h7 += h9;
        h10 ^= h7;
        h9 = Rot64(h9, 38);
        h8 += h10;
        h11 ^= h8;
        h10 = Rot64(h10, 53);
        h9 += h11;
        h0 ^= h9;
        h11 = Rot64(h11, 42);
        h10 += h0;
        h1 ^= h10;
        h0 = Rot64(h0, 54);
      }

      static inline QUICKCPPLIB_FORCEINLINE void End(const uint64 *data, uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3, uint64 &h4, uint64 &h5, uint64 &h6,
                                                     uint64 &h7, uint64 &h8, uint64 &h9, uint64 &h10, uint64 &h11)
      {
        h0 += data[0];
        h1 += data[1];
        h2 += data[2];
        h3 += data[3];
        h4 += data[4];
        h5 += data[5];
        h6 += data[6];
        h7 += data[7];
        h8 += data[8];
        h9 += data[9];
        h10 += data[10];
        h11 += data[11];
        EndPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
        EndPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
        EndPartial(h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
      }

      static inline QUICKCPPLIB_FORCEINLINE void ShortMix(uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3)
      {
        h2 = Rot64(h2, 50);
        h2 += h3;
        h0 ^= h2;
        h3 = Rot64(h3, 52);
        h3 += h0;
        h1 ^= h3;
        h0 = Rot64(h0, 30);
        h0 += h1;
        h2 ^= h0;
        h1 = Rot64(h1, 41);
        h1 += h2;
        h3 ^= h1;
        h2 = Rot64(h2, 54);
        h2 += h3;
        h0 ^= h2;
        h3 = Rot64(h3, 48);
        h3 += h0;
        h1 ^= h3;
        h0 = Rot64(h0, 38);
        h0 += h1;
        h2 ^= h0;
        h1 = Rot64(h1, 37);
        h1 += h2;
        h3 ^= h1;
        h2 = Rot64(h2, 62);
        h2 += h3;
        h0 ^= h2;
        h3 = Rot64(h3, 34);
        h3 += h0;
        h1 ^= h3;
        h0 = Rot64(h0, 5);
        h0 += h1;
        h2 ^= h0;
        h1 = Rot64(h1, 36);
        h1 += h2;
        h3 ^= h1;
      }

      static inline QUICKCPPLIB_FORCEINLINE void ShortEnd(uint64 &h0, uint64 &h1, uint64 &h2, uint64 &h3)
      {
        h3 ^= h2;
        h2 = Rot64(h2, 15);
        h3 += h2;
        h0 ^= h3;
        h3 = Rot64(h3, 52);
        h0 += h3;
        h1 ^= h0;
        h0 = Rot64(h0, 26);
        h1 += h0;
        h2 ^= h1;
        h1 = Rot64(h1, 51);
        h2 += h1;
        h3 ^= h2;
        h2 = Rot64(h2, 28);
        h3 += h2;
        h0 ^= h3;
        h3 = Rot64(h3, 9);
        h0 += h3;
        h1 ^= h0;
        h0 = Rot64(h0, 47);
        h1 += h0;
        h2 ^= h1;
        h1 = Rot64(h1, 54);
        h2 += h1;
        h3 ^= h2;
        h2 = Rot64(h2, 32);
        h3 += h2;
        h0 ^= h3;
        h3 = Rot64(h3, 25);
        h0 += h3;
        h1 ^= h0;
        h0 = Rot64(h0, 63);
        h1 += h0;
      }
    }  // namespace fash_hash_detail

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)  // use of ALLOW_UNALIGNED_READS
#endif

    /*! \class fast_hash
    \brief Fast very collision resistant uint128 hash. Currently SpookyHash @ 0.3 cycles/byte.
    */
    class fast_hash
    {
      using uint8 = unsigned char;
      using uint64 = unsigned long long;
      using uint128 = integers128::uint128;

      // number of uint64's in internal state
      static constexpr size_t sc_numVars = 12;

      // size of the internal state
      static constexpr size_t sc_blockSize = sc_numVars * 8;

      // size of buffer of unhashed data, in bytes
      static constexpr size_t sc_bufSize = 2 * sc_blockSize;

      //
      // sc_const: a constant which:
      //  * is not zero
      //  * is odd
      //  * is a not-very-regular mix of 1's and 0's
      //  * does not need any other special mathematical properties
      //
      static constexpr uint64 sc_const = 0xdeadbeefdeadbeefULL;

      union
      {
        uint8 m_data_[2 * sc_numVars * 8];
        uint64 m_data[2 * sc_numVars];  // unhashed data, for partial messages
      };
      uint64 m_state[sc_numVars];  // internal state of the hash
      size_t m_length;             // total length of the input so far
      uint8 m_remainder;           // length of unhashed data stashed in m_data

      QUICKCPPLIB_TEMPLATE(class T)
      QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
      static inline void short_(uint128 &hash, const T *message, size_t length) noexcept
      {
        using namespace fash_hash_detail;
        uint64 buf[2 * sc_numVars];
        union
        {
          const uint8 *p8;
          uint32 *p32;
          uint64 *p64;
          size_t i;
        } u;

        u.p8 = (const uint8 *) message;

        if(!ALLOW_UNALIGNED_READS && (u.i & 0x7))
        {
          memcpy(buf, message, length);
          u.p64 = buf;
        }

        size_t remainder = length % 32;
        uint64 a = hash.as_longlongs[0];
        uint64 b = hash.as_longlongs[1];
        uint64 c = sc_const;
        uint64 d = sc_const;

        if(length > 15)
        {
          const uint64 *end = u.p64 + (length / 32) * 4;

          // handle all complete sets of 32 bytes
          for(; u.p64 < end; u.p64 += 4)
          {
            c += u.p64[0];
            d += u.p64[1];
            ShortMix(a, b, c, d);
            a += u.p64[2];
            b += u.p64[3];
          }

          // Handle the case of 16+ remaining bytes.
          if(remainder >= 16)
          {
            c += u.p64[0];
            d += u.p64[1];
            ShortMix(a, b, c, d);
            u.p64 += 2;
            remainder -= 16;
          }
        }

        // Handle the last 0..15 bytes, and its length
        d += ((uint64) length) << 56;
        switch(remainder)
        {
        case 15:
          d += ((uint64) u.p8[14]) << 48;
        // fallthrough
        case 14:
          d += ((uint64) u.p8[13]) << 40;
        // fallthrough
        case 13:
          d += ((uint64) u.p8[12]) << 32;
        // fallthrough
        case 12:
          d += u.p32[2];
          c += u.p64[0];
          break;
        case 11:
          d += ((uint64) u.p8[10]) << 16;
        // fallthrough
        case 10:
          d += ((uint64) u.p8[9]) << 8;
        // fallthrough
        case 9:
          d += (uint64) u.p8[8];
        // fallthrough
        case 8:
          c += u.p64[0];
          break;
        case 7:
          c += ((uint64) u.p8[6]) << 48;
        // fallthrough
        case 6:
          c += ((uint64) u.p8[5]) << 40;
        // fallthrough
        case 5:
          c += ((uint64) u.p8[4]) << 32;
        // fallthrough
        case 4:
          c += u.p32[0];
          break;
        case 3:
          c += ((uint64) u.p8[2]) << 16;
        // fallthrough
        case 2:
          c += ((uint64) u.p8[1]) << 8;
        // fallthrough
        case 1:
          c += (uint64) u.p8[0];
          break;
        case 0:
          c += sc_const;
          d += sc_const;
        }
        ShortEnd(a, b, c, d);
        hash.as_longlongs[0] = a;
        hash.as_longlongs[1] = b;
      }

    public:
      //! The result type of the hash
      using result_type = uint128;

      //! Initialise the hash with an optional seed
      constexpr fast_hash(const uint128 &seed = 0) noexcept
          : m_data{0}
          , m_state{seed.as_longlongs[0], seed.as_longlongs[1]}
          , m_length(0)
          , m_remainder(0)
      {
      }

      //! Hash input
      QUICKCPPLIB_TEMPLATE(class T)
      QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
      inline void add(const T *message, size_t length) noexcept
      {
        using namespace fash_hash_detail;
        uint64 h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
        size_t newLength = length + m_remainder;
        uint8 remainder;
        union
        {
          const uint8 *p8;
          uint64 *p64;
          size_t i;
        } u;
        const uint64 *end;

        // Is this message fragment too short?  If it is, stuff it away.
        if(newLength < sc_bufSize)
        {
          memcpy(&((uint8 *) m_data)[m_remainder], message, length);
          m_length = length + m_length;
          m_remainder = (uint8) newLength;
          return;
        }

        // init the variables
        if(m_length < sc_bufSize)
        {
          h0 = h3 = h6 = h9 = m_state[0];
          h1 = h4 = h7 = h10 = m_state[1];
          h2 = h5 = h8 = h11 = sc_const;
        }
        else
        {
          h0 = m_state[0];
          h1 = m_state[1];
          h2 = m_state[2];
          h3 = m_state[3];
          h4 = m_state[4];
          h5 = m_state[5];
          h6 = m_state[6];
          h7 = m_state[7];
          h8 = m_state[8];
          h9 = m_state[9];
          h10 = m_state[10];
          h11 = m_state[11];
        }
        m_length = length + m_length;

        // if we've got anything stuffed away, use it now
        if(m_remainder)
        {
          uint8 prefix = sc_bufSize - m_remainder;
          memcpy(&(((uint8 *) m_data)[m_remainder]), message, prefix);
          u.p64 = m_data;
          Mix(u.p64, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
          Mix(&u.p64[sc_numVars], h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
          u.p8 = ((const uint8 *) message) + prefix;
          length -= prefix;
        }
        else
        {
          u.p8 = (const uint8 *) message;
        }

        // handle all whole blocks of sc_blockSize bytes
        end = u.p64 + (length / sc_blockSize) * sc_numVars;
        remainder = (uint8)(length - ((const uint8 *) end - u.p8));
        if(ALLOW_UNALIGNED_READS || (u.i & 0x7) == 0)
        {
          while(u.p64 < end)
          {
            Mix(u.p64, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
            u.p64 += sc_numVars;
          }
        }
        else
        {
          while(u.p64 < end)
          {
            memcpy(m_data, u.p8, sc_blockSize);
            Mix(m_data, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
            u.p64 += sc_numVars;
          }
        }

        // stuff away the last few bytes
        m_remainder = remainder;
        memcpy(m_data, end, remainder);

        // stuff away the variables
        m_state[0] = h0;
        m_state[1] = h1;
        m_state[2] = h2;
        m_state[3] = h3;
        m_state[4] = h4;
        m_state[5] = h5;
        m_state[6] = h6;
        m_state[7] = h7;
        m_state[8] = h8;
        m_state[9] = h9;
        m_state[10] = h10;
        m_state[11] = h11;
      }

      //! Finalise and return hash
      inline uint128 finalise() noexcept
      {
        using namespace fash_hash_detail;
        uint128 ret;
        // init the variables
        if(m_length < sc_bufSize)
        {
          ret.as_longlongs[0] = m_state[0];
          ret.as_longlongs[1] = m_state[1];
          short_(ret, m_data_, m_length);
          return ret;
        }

        const uint64 *data = (const uint64 *) m_data;
        uint8 remainder = m_remainder;

        uint64 h0 = m_state[0];
        uint64 h1 = m_state[1];
        uint64 h2 = m_state[2];
        uint64 h3 = m_state[3];
        uint64 h4 = m_state[4];
        uint64 h5 = m_state[5];
        uint64 h6 = m_state[6];
        uint64 h7 = m_state[7];
        uint64 h8 = m_state[8];
        uint64 h9 = m_state[9];
        uint64 h10 = m_state[10];
        uint64 h11 = m_state[11];

        if(remainder >= sc_blockSize)
        {
          // m_data can contain two blocks; handle any whole first block
          Mix(data, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
          data += sc_numVars;
          remainder -= sc_blockSize;
        }

        // mix in the last partial block, and the length mod sc_blockSize
        memset(&((uint8 *) data)[remainder], 0, (sc_blockSize - remainder));

        ((uint8 *) data)[sc_blockSize - 1] = remainder;

        // do some final mixing
        End(data, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);

        ret.as_longlongs[0] = h0;
        ret.as_longlongs[1] = h1;
        return ret;
      }

      //! Single shot hash of a sequence of bytes
      QUICKCPPLIB_TEMPLATE(class T)
      QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
      static inline uint128 hash(const T *message, size_t length, const uint128 &_ret = 0) noexcept
      {
        uint128 ret(_ret);
        using namespace fash_hash_detail;
        if(length < sc_bufSize)
        {
          short_(ret, message, length);
          return ret;
        }

        uint64 h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11;
        uint64 buf[sc_numVars];
        uint64 *end;
        union
        {
          const uint8 *p8;
          uint64 *p64;
          size_t i;
        } u;
        size_t remainder;

        h0 = h3 = h6 = h9 = ret.as_longlongs[0];
        h1 = h4 = h7 = h10 = ret.as_longlongs[1];
        h2 = h5 = h8 = h11 = sc_const;

        u.p8 = (const uint8 *) message;
        end = u.p64 + (length / sc_blockSize) * sc_numVars;

        // handle all whole sc_blockSize blocks of bytes
        if(ALLOW_UNALIGNED_READS || ((u.i & 0x7) == 0))
        {
          while(u.p64 < end)
          {
            Mix(u.p64, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
            u.p64 += sc_numVars;
          }
        }
        else
        {
          while(u.p64 < end)
          {
            memcpy(buf, u.p64, sc_blockSize);
            Mix(buf, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);
            u.p64 += sc_numVars;
          }
        }

        // handle the last partial block of sc_blockSize bytes
        remainder = (length - ((const uint8 *) end - (const uint8 *) message));
        memcpy(buf, end, remainder);
        memory::cmemset(((uint8 *) buf) + remainder, (uint8) 0, sc_blockSize - remainder);
        ((uint8 *) buf)[sc_blockSize - 1] = (uint8) remainder;

        // do some final mixing
        End(buf, h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11);

        ret.as_longlongs[0] = h0;
        ret.as_longlongs[1] = h1;
        return ret;
      }

      //! Single shot hash of a span
      //      template <typename T> static inline uint128 hash(const span::span<T> &str) noexcept { return hash((char *) str.data(), str.size() * sizeof(T));
      //      }
    };


    /*! \class sha256_hash
    \brief SHA256 hash.
    */
    class sha256_hash
    {
      // Implementation is adapted from https://github.com/amosnier/sha-2/blob/master/sha-256.c, which does
      // not have a licence as it was placed into the public domain.
      static constexpr size_t _CHUNK_SIZE = 64;
      static constexpr size_t _TOTAL_LEN_LEN = 8;
      static const uint32_t *_k() noexcept
      {
        static const uint32_t v[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,  //
        0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,  //
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,  //
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,  //
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,  //
        0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,  //
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2                                                                           //
        };
        return v;
      }
      static constexpr inline uint32_t _right_rot(uint32_t value, unsigned int count) { return (value >> count) | (value << (32 - count)); }

    public:
      //! The result type of the hash
      union alignas(32) result_type
      {
        struct empty_type
        {
        } _empty;
        uint8_t as_bytes[32];
        uint32_t as_ints[8];
        uint64_t as_longlongs[4];
// Strongly hint to the compiler what to do here
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
        __m128i as_m128is[2];
#endif
#if defined(__GNUC__) || defined(__clang__)
        typedef unsigned uint32_4_t __attribute__((vector_size(32)));
        uint32_4_t as_uint32_8;
#endif
        constexpr result_type()
            : _empty{}
        {
        }
        constexpr result_type(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h)
            : as_ints{a, b, c, d, e, f, g, h}
        {
        }
      };

    private:
      /*
       * Initialize hash values:
       * (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):
       */
      result_type _h{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
      uint8_t _chunk[_CHUNK_SIZE]{};
      size_t _chunkidx{0};
      size_t _total_len{0};

      QUICKCPPLIB_TEMPLATE(class T)
      QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
      const uint8_t *_calc_chunk(const T *&data, size_t &bytes)
      {
        // fixed size fast path
        if(_chunkidx == 0 && bytes >= _CHUNK_SIZE)
        {
          memcpy(_chunk, data, _CHUNK_SIZE);
          _total_len += _CHUNK_SIZE;
          data += _CHUNK_SIZE;
          bytes -= _CHUNK_SIZE;
          return (const uint8_t *) (data - _CHUNK_SIZE);
        }
        size_t tocopy = _CHUNK_SIZE;
        if(tocopy > _CHUNK_SIZE - _chunkidx)
        {
          tocopy = _CHUNK_SIZE - _chunkidx;
        }
        if(tocopy > bytes)
        {
          tocopy = bytes;
        }
        if(data != nullptr)
        {
          memcpy(_chunk + _chunkidx, data, tocopy);
          _total_len += tocopy;
          data += tocopy;
          bytes -= tocopy;
        }
        _chunkidx += tocopy;
        if(_chunkidx == _CHUNK_SIZE)
        {
          _chunkidx = 0;
          return _chunk;
        }
        return nullptr;
      }

    public:
      //! Initialise the hash
      constexpr sha256_hash() noexcept {}

      //! Hash input
      QUICKCPPLIB_TEMPLATE(class T)
      QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
      void add(const T *data, size_t bytes) noexcept
      {
        static const uint32_t *const k = _k();
        /*
         * Note 1: All integers (expect indexes) are 32-bit unsigned integers and addition is calculated modulo 2^32.
         * Note 2: For each round, there is one round constant k[i] and one entry in the message schedule array w[i], 0 = i = 63
         * Note 3: The compression function uses 8 working variables, a through h
         * Note 4: Big-endian convention is used when expressing the constants in this pseudocode,
         *     and when parsing message block data from bytes to words, for example,
         *     the first word of the input message "abc" after padding is 0x61626380
         */
        while(const uint8_t *p = _calc_chunk(data, bytes))
        {
          /* Note that if we ever do support CPUID detection for the Intel SHA extensions, there is a public domain
          implementation of the SHA256 rounds at https://github.com/noloader/SHA-Intrinsics/blob/master/sha256-x86.c
          */
          alignas(32) uint32_t ah[8];

          /* Initialize working variables to current hash value: */
          memcpy(ah, _h.as_ints, sizeof(ah));

          /*
           * create a 64-entry message schedule array w[0..63] of 32-bit words
           * copy chunk into first 16 words w[0..15] of the message schedule array
           */
          alignas(32) uint32_t w[64];
          for(unsigned i = 0; i < 16; i++)
          {
            w[i] = ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | ((uint32_t) p[3]);
            p += 4;
          }
          /* Extend the first 16 words into the remaining 48 words w[16..63] of the message schedule array: */
          for(unsigned i = 16; i < 64; i++)
          {
            const uint32_t s0 = _right_rot(w[i - 15], 7) ^ _right_rot(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const uint32_t s1 = _right_rot(w[i - 2], 17) ^ _right_rot(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
          }

          /* Compression function main loop: */
          for(unsigned i = 0; i < 64; i++)
          {
            const uint32_t s1 = _right_rot(ah[4], 6) ^ _right_rot(ah[4], 11) ^ _right_rot(ah[4], 25);
            const uint32_t ch = (ah[4] & ah[5]) ^ (~ah[4] & ah[6]);
            const uint32_t temp1 = ah[7] + s1 + ch + k[i] + w[i];
            const uint32_t s0 = _right_rot(ah[0], 2) ^ _right_rot(ah[0], 13) ^ _right_rot(ah[0], 22);
            const uint32_t maj = (ah[0] & ah[1]) ^ (ah[0] & ah[2]) ^ (ah[1] & ah[2]);
            const uint32_t temp2 = s0 + maj;

            ah[7] = ah[6];
            ah[6] = ah[5];
            ah[5] = ah[4];
            ah[4] = ah[3] + temp1;
            ah[3] = ah[2];
            ah[2] = ah[1];
            ah[1] = ah[0];
            ah[0] = temp1 + temp2;
          }

          /* Add the compressed chunk to the current hash value: */
          for(unsigned i = 0; i < 8; i++)
            _h.as_ints[i] += ah[i];
        }
      }

      //! Finalise and return hash
      result_type finalise() noexcept
      {
        /*
         * Now:
         * - either there is enough space left for the total length, and we can conclude,
         * - or there is too little space left, and we have to pad the rest of this chunk with zeroes.
         * In the latter case, we will conclude at the next invokation of this function.
         */
        size_t space_in_chunk = _CHUNK_SIZE - _chunkidx;
        assert(space_in_chunk > 0);
        _chunk[_chunkidx++] = 0x80;
        space_in_chunk--;
        if(space_in_chunk < _TOTAL_LEN_LEN)
        {
          memory::cmemset(_chunk + _chunkidx, (uint8_t) 0, space_in_chunk);
          _chunkidx += space_in_chunk;
          assert(_chunkidx == _CHUNK_SIZE);
          add((const uint8_t *) nullptr, 0);
          assert(_chunkidx == 0);
          space_in_chunk = _CHUNK_SIZE - _chunkidx;
        }
        assert(space_in_chunk >= _TOTAL_LEN_LEN);

        const size_t left = space_in_chunk - _TOTAL_LEN_LEN;
        memory::cmemset(_chunk + _chunkidx, (uint8_t) 0, left);
        _chunkidx += left;

        /* Storing of len * 8 as a big endian 64-bit without overflow. */
        size_t len = _total_len;
        _chunk[_chunkidx + 7] = (uint8_t)((len << 3) & 0xff);
        len >>= 5;
        for(int i = 6; i >= 0; i--)
        {
          _chunk[_chunkidx + i] = (uint8_t)(len & 0xff);
          len >>= 8;
        }
        _chunkidx += 8;
        assert(_chunkidx == _CHUNK_SIZE);
        add((const uint8_t *) nullptr, 0);
        assert(_chunkidx == 0);

        result_type ret;
        /* Produce the final hash value (big-endian): */
        for(unsigned i = 0, j = 0; i < 8; i++)
        {
          ret.as_bytes[j++] = (uint8_t)((_h.as_ints[i] >> 24) & 0xff);
          ret.as_bytes[j++] = (uint8_t)((_h.as_ints[i] >> 16) & 0xff);
          ret.as_bytes[j++] = (uint8_t)((_h.as_ints[i] >> 8) & 0xff);
          ret.as_bytes[j++] = (uint8_t)(_h.as_ints[i] & 0xff);
        }
        _h = result_type{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
        _chunkidx = _total_len = 0;
        return ret;
      }

      //! Single shot hash of a sequence of bytes
      QUICKCPPLIB_TEMPLATE(class T)
      QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
      static constexpr result_type hash(const T *data, size_t bytes) noexcept
      {
        sha256_hash ret;
        ret.add(data, bytes);
        return ret.finalise();
      }
    };


#ifdef _MSC_VER
#pragma warning(pop)
#endif
  }  // namespace hash
}  // namespace algorithm

QUICKCPPLIB_NAMESPACE_END

#endif
