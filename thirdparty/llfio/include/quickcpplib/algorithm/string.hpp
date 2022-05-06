/* String algorithms
(C) 2016-2020 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
File Created: Jun 2016


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

#ifndef QUICKCPPLIB_ALGORITHM_STRING_HPP
#define QUICKCPPLIB_ALGORITHM_STRING_HPP

#include "../span.hpp"

#include <algorithm>
#include <locale>
#include <string>

QUICKCPPLIB_NAMESPACE_BEGIN

namespace algorithm
{
  namespace string
  {
    //! Returns an all lower case edition of the input string. i18n aware.
    template <class Char> std::basic_string<Char> tolower(std::basic_string<Char> s)
    {
      auto &f = std::use_facet<std::ctype<Char>>(std::locale());
      std::transform(s.begin(), s.end(), s.begin(), [&](Char c) { return f.tolower(c); });
      return s;
    }

    //! Returns an all upper case edition of the input string. i18n aware.
    template <class Char> std::basic_string<Char> toupper(std::basic_string<Char> s)
    {
      auto &f = std::use_facet<std::ctype<Char>>(std::locale());
      std::transform(s.begin(), s.end(), s.begin(), [&](Char c) { return f.toupper(c); });
      return s;
    }

/*! \brief Converts a number to a hex string. Out buffer can be same as in buffer.

Note that the character range used is a 16 item table of:

0123456789abcdef

This lets one pack one byte of input into two bytes of output.

\ingroup utils
\complexity{O(N) where N is the length of the number.}
\exceptionmodel{Throws exception if output buffer is too small for input.}
*/
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6293)  // MSVC sanitiser warns that we wrap n in the for loop
#endif
    QUICKCPPLIB_TEMPLATE(class CharType, class T)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value),
                          QUICKCPPLIB_TPRED(!std::is_const<CharType>::value))
    inline size_t to_hex_string(CharType *out, size_t outlen, const T *_in, size_t inlen)
    {
      unsigned const char *in = (unsigned const char *) _in;
      static constexpr char table[] = "0123456789abcdef";
      if(outlen < inlen * 2)
        throw std::invalid_argument("Output buffer too small.");
      if(inlen >= 2)
      {
        for(size_t n = inlen - 2; n <= inlen - 2; n -= 2)
        {
          out[n * 2 + 3] = table[in[n + 1] & 0xf];
          out[n * 2 + 2] = table[(in[n + 1] >> 4) & 0xf];
          out[n * 2 + 1] = table[in[n] & 0xf];
          out[n * 2 + 0] = table[(in[n] >> 4) & 0xf];
        }
      }
      if(inlen & 1)
      {
        out[1] = table[in[0] & 0xf];
        out[0] = table[(in[0] >> 4) & 0xf];
      }
      return inlen * 2;
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    //! \overload
    QUICKCPPLIB_TEMPLATE(class CharType, class T)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value),
                          QUICKCPPLIB_TPRED(!std::is_const<CharType>::value))
    inline size_t to_hex_string(span::span<CharType> out, const span::span<T> in) { return to_hex_string(out.data(), out.size(), in.data(), in.size()); }
    //! \overload
    QUICKCPPLIB_TEMPLATE(class T)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
    inline std::string to_hex_string(span::span<T> in)
    {
      std::string out(in.size() * 2, ' ');
      to_hex_string(const_cast<char *>(out.data()), out.size(), in.data(), in.size());
      return out;
    }
    //! \overload
    QUICKCPPLIB_TEMPLATE(class T)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
    inline std::string to_hex_string(const T *in, size_t len)
    {
      std::string out(len * 2, ' ');
      to_hex_string(const_cast<char *>(out.data()), out.size(), in, len);
      return out;
    }

    /*! \brief Converts a hex string to a number. Out buffer can be same as in buffer.

    Note that this routine is about 43% slower than to_hex_string(), half of which is due to input validation.

    \ingroup utils
    \complexity{O(N) where N is the length of the string.}
    \exceptionmodel{Throws exception if output buffer is too small for input or input size is not multiple of two.}
    */
    QUICKCPPLIB_TEMPLATE(class T, class CharType)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value),
                          QUICKCPPLIB_TPRED(!std::is_const<T>::value))
    inline size_t from_hex_string(T *out, size_t outlen, const CharType *in, size_t inlen)
    {
      if(inlen % 2)
        throw std::invalid_argument("Input buffer not multiple of two.");
      if(outlen < inlen / 2)
        throw std::invalid_argument("Output buffer too small.");
      bool is_invalid = false;
      auto fromhex = [&is_invalid](CharType c) -> unsigned char {
#if 1
        // ASCII starting from 48 is 0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
        //                           48               65                              97
        static constexpr unsigned char table[] = {0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  // +10 = 58
                                                  255, 255, 255, 255, 255, 255, 255,               // +7  = 65
                                                  10,  11,  12,  13,  14,  15,                     // +6  = 71
                                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // +26 = 97
                                                  10,  11,  12,  13,  14,  15};
        unsigned char r = 255;
        if(c >= 48 && c <= 102)
          r = table[c - 48];
        if(r == 255)
          is_invalid = true;
        return r;
#else
        if(c >= '0' && c <= '9')
          return c - '0';
        if(c >= 'a' && c <= 'f')
          return c - 'a' + 10;
        if(c >= 'A' && c <= 'F')
          return c - 'A' + 10;
        throw std::invalid_argument("Input is not hexadecimal.");
#endif
      };
      const auto bulklen = inlen / 2 - (inlen / 2) % 4;
      if(bulklen >= 4)
      {
        for(size_t n = 0; n < bulklen; n += 4)
        {
          unsigned char c[8];
          c[0] = fromhex(in[n * 2]);
          c[1] = fromhex(in[n * 2 + 1]);
          c[2] = fromhex(in[n * 2 + 2]);
          c[3] = fromhex(in[n * 2 + 3]);
          out[n] = (T)((c[0] << 4) | c[1]);
          c[4] = fromhex(in[n * 2 + 4]);
          c[5] = fromhex(in[n * 2 + 5]);
          out[n + 1] = (T)((c[2] << 4) | c[3]);
          c[6] = fromhex(in[n * 2 + 6]);
          c[7] = fromhex(in[n * 2 + 7]);
          out[n + 2] = (T)((c[4] << 4) | c[5]);
          out[n + 3] = (T)((c[6] << 4) | c[7]);
        }
      }
      for(size_t n = bulklen; n < inlen / 2; n++)
      {
        auto c1 = fromhex(in[n * 2]), c2 = fromhex(in[n * 2 + 1]);
        out[n] = (T)((c1 << 4) | c2);
      }
      if(is_invalid)
        throw std::invalid_argument("Input is not hexadecimal.");
      return inlen / 2;
    }
  }  // namespace string
}  // namespace algorithm

QUICKCPPLIB_NAMESPACE_END

#endif
