/* Proposed SG14 status_code
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Feb 2018


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

#ifndef SYSTEM_ERROR2_SYSTEM_CODE_HPP
#define SYSTEM_ERROR2_SYSTEM_CODE_HPP

#ifndef SYSTEM_ERROR2_NOT_POSIX
#include "posix_code.hpp"
#else
#include "quick_status_code_from_enum.hpp"
#endif

#if defined(_WIN32) || defined(STANDARDESE_IS_IN_THE_HOUSE)
#include "nt_code.hpp"
#include "win32_code.hpp"
// NOT "com_code.hpp"
#endif

SYSTEM_ERROR2_NAMESPACE_BEGIN
/*! An erased-mutable status code suitably large for all the system codes
which can be returned on this system.

For Windows, these might be:

    - `com_code` (`HRESULT`)  [you need to include "com_code.hpp" explicitly for this]
    - `nt_code` (`LONG`)
    - `win32_code` (`DWORD`)

For POSIX, `posix_code` is possible.

You are guaranteed that `system_code` can be transported by the compiler
in exactly two CPU registers.
*/
using system_code = status_code<erased<intptr_t>>;

#ifndef NDEBUG
static_assert(sizeof(system_code) == 2 * sizeof(void *), "system_code is not exactly two pointers in size!");
static_assert(traits::is_move_bitcopying<system_code>::value, "system_code is not move bitcopying!");
#endif

SYSTEM_ERROR2_NAMESPACE_END

#endif
