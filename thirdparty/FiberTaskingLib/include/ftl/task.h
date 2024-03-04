/**
 * FiberTaskingLib - A tasking library that uses fibers for efficient task switching
 *
 * This library was created as a proof of concept of the ideas presented by
 * Christian Gyrling in his 2015 GDC Talk 'Parallelizing the Naughty Dog Engine Using Fibers'
 *
 * http://gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
 *
 * FiberTaskingLib is the legal property of Adrian Astley
 * Copyright Adrian Astley 2015 - 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "SkrBase/config.h"
#include <string>
#include <memory>

#include "SkrProfile/profile.h"
#include "SkrProfile/profile.h"

#ifdef SKR_PROFILE_ENABLE
    #define FTL_TASK_NAME(...) __VA_ARGS__
#else
    #define FTL_TASK_NAME(...)
#endif

namespace ftl
{

class TaskScheduler;

using TaskFunction = void (*)(TaskScheduler* taskScheduler, void* arg);
using TaskPostFunction = void (*)(void* arg);

struct Task {
    TaskFunction Function;
    void* ArgData;
    std::shared_ptr<void> RefCounter;
};

template<class F>
std::shared_ptr<void> PostTask(F&& f)
{
    struct Helper
    {
        std::remove_reference_t<F> f;
        Helper(F&& p)
            :f(std::forward<F>(p)) {}
        ~Helper()
        {
            f();
        }
    };
    return std::static_pointer_cast<void>(std::make_shared<Helper>(std::forward<F>(f)));
}

enum class TaskPriority
{
    High,
    Normal,
};

} // End of namespace ftl
