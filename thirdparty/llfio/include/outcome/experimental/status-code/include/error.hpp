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

#ifndef SYSTEM_ERROR2_ERROR_HPP
#define SYSTEM_ERROR2_ERROR_HPP

#include "errored_status_code.hpp"
#include "system_code.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

/*! An erased `system_code` which is always a failure. The closest equivalent to
`std::error_code`, except it cannot be null and cannot be modified.

This refines `system_code` into an `error` object meeting the requirements of
[P0709 Zero-overhead deterministic exceptions](https://wg21.link/P0709).

Differences from `system_code`:

- Always a failure (this is checked at construction, and if not the case,
the program is terminated as this is a logic error)
- No default construction.
- No empty state possible.
- Is immutable.

As with `system_code`, it remains guaranteed to be two CPU registers in size,
and move bitcopying.
*/
using error = errored_status_code<erased<system_code::value_type>>;

#ifndef NDEBUG
static_assert(sizeof(error) == 2 * sizeof(void *), "error is not exactly two pointers in size!");
static_assert(traits::is_move_bitcopying<error>::value, "error is not move bitcopying!");
#endif

SYSTEM_ERROR2_NAMESPACE_END

#endif
