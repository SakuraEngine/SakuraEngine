/* Include the default amount of outcome
(C) 2018-2021 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: Mar 2018


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

#if !OUTCOME_ENABLE_CXX_MODULES || !defined(__cpp_modules) || defined(GENERATING_OUTCOME_MODULE_INTERFACE) || OUTCOME_DISABLE_CXX_MODULES
#include "outcome/coroutine_support.hpp"
#include "outcome/iostream_support.hpp"
#include "outcome/try.hpp"
#else
#include "outcome/try.hpp"

import OUTCOME_V2_CXX_MODULE_NAME;
#endif
