/* C interface for result
(C) 2017-2020 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
File Created: Aug 2017


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

#ifndef OUTCOME_EXPERIMENTAL_RESULT_H
#define OUTCOME_EXPERIMENTAL_RESULT_H

#include <stdint.h>  // for intptr_t

#ifdef __cplusplus
extern "C"
{
#endif

#define CXX_DECLARE_RESULT(ident, R, S)                                                                                                                        \
  struct cxx_result_##ident                                                                                                                                    \
  {                                                                                                                                                            \
    union                                                                                                                                                      \
    {                                                                                                                                                          \
      R value;                                                                                                                                                 \
      S error;                                                                                                                                                 \
    };                                                                                                                                                         \
    unsigned flags;                                                                                                                                            \
  }

#define CXX_RESULT(ident) struct cxx_result_##ident


#define CXX_RESULT_HAS_VALUE(r) (((r).flags & 1U) == 1U)

#define CXX_RESULT_HAS_ERROR(r) (((r).flags & 2U) == 2U)

#define CXX_RESULT_ERROR_IS_ERRNO(r) (((r).flags & (1U << 4U)) == (1U << 4U))


  /***************************** <system_error2> support ******************************/

#define CXX_DECLARE_STATUS_CODE(ident, value_type)                                                                                                             \
  struct cxx_status_code_##ident                                                                                                                               \
  {                                                                                                                                                            \
    void *domain;                                                                                                                                              \
    value_type value;                                                                                                                                          \
  };

#define CXX_STATUS_CODE(ident) struct cxx_status_code_##ident

#define CXX_DECLARE_RESULT_STATUS_CODE(ident, R, S)                                                                                                            \
  struct cxx_result_status_code_##ident                                                                                                                        \
  {                                                                                                                                                            \
    R value;                                                                                                                                                   \
    unsigned flags;                                                                                                                                            \
    S error;                                                                                                                                                   \
  }

#define CXX_RESULT_STATUS_CODE(ident) struct cxx_result_status_code_##ident


  struct cxx_status_code_posix
  {
    void *domain;
    int value;
  };
#define CXX_DECLARE_RESULT_ERRNO(ident, R) CXX_DECLARE_RESULT_STATUS_CODE(posix_##ident, R, struct cxx_status_code_posix)
#define CXX_RESULT_ERRNO(ident) CXX_RESULT_STATUS_CODE(posix_##ident)

  struct cxx_status_code_system
  {
    void *domain;
    intptr_t value;
  };
#define CXX_DECLARE_RESULT_SYSTEM(ident, R) CXX_DECLARE_RESULT_STATUS_CODE(system_##ident, R, struct cxx_status_code_system)
#define CXX_RESULT_SYSTEM(ident) CXX_RESULT_STATUS_CODE(system_##ident)

#ifdef __cplusplus
}
#endif

#endif
