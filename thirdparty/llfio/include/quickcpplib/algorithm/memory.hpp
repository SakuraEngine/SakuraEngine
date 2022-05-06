/* Memory algorithms
(C) 2018-2020 Niall Douglas <http://www.nedproductions.biz/> (7 commits)
File Created: Jun 2018


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

#ifndef QUICKCPPLIB_ALGORITHM_MEMORY_HPP
#define QUICKCPPLIB_ALGORITHM_MEMORY_HPP

#include "../config.hpp"

#include <cstring>  // for memcpy

QUICKCPPLIB_NAMESPACE_BEGIN

namespace algorithm
{
  namespace memory
  {
    QUICKCPPLIB_TEMPLATE(class T)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
    constexpr inline T *cmemcpy(T *dst, const T *src, size_t num)
    {
#if __cpp_lib_is_constant_evaluated >= 201811
      if(std::is_constant_evaluated())
      {
#endif
        for(size_t n = 0; n < num; n++)
        {
          dst[n] = src[n];
        }
#if __cpp_lib_is_constant_evaluated >= 201811
      }
      else
      {
        memcpy(dst, src, num);
      }
#endif
      return dst;
    }
    QUICKCPPLIB_TEMPLATE(class T)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
    constexpr inline int cmemcmp(const T *a, const T *b, size_t num)
    {
#if __cpp_lib_is_constant_evaluated >= 201811
      if(std::is_constant_evaluated())
      {
#endif
        for(size_t n = 0; n < num; n++)
        {
          if(a[n] < b[n])
          {
            return -1;
          }
          else if(a[n] > b[n])
          {
            return 1;
          }
        }
        return 0;
#if __cpp_lib_is_constant_evaluated >= 201811
      }
      else
      {
        return memcmp(a, b, num);
      }
#endif
    }
    QUICKCPPLIB_TEMPLATE(class T)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(sizeof(T) == 1), QUICKCPPLIB_TPRED(std::is_trivially_copyable<T>::value))
    constexpr inline T *cmemset(T *dst, T value, size_t num)
    {
#if __cpp_lib_is_constant_evaluated >= 201811
      if(std::is_constant_evaluated())
      {
#endif
        for(size_t n = 0; n < num; n++)
        {
          dst[n] = value;
        }
#if __cpp_lib_is_constant_evaluated >= 201811
      }
      else
      {
        memset(dst, (int) value, num);
      }
#endif
      return dst;
    }
  }  // namespace memory
}  // namespace algorithm

QUICKCPPLIB_NAMESPACE_END

#endif
