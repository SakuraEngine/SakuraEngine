// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once
#include "marl/memory.h"

#ifdef MARL_USE_EASTL
#include <EASTL/functional.h>
namespace marl { using eastl::function; }
#else
#include <functional>
namespace marl { using std::function; }
#endif

namespace marl {

class OSFiber {
 public:
  ~OSFiber();

  // createFiberFromCurrentThread() returns a fiber created from the current
  // thread.
  static Allocator::unique_ptr<OSFiber> createFiberFromCurrentThread(
      Allocator* allocator);

  // createFiber() returns a new fiber with the given stack size that will
  // call func when switched to. func() must end by switching back to another
  // fiber, and must not return.
  static Allocator::unique_ptr<OSFiber> createFiber(
      Allocator* allocator,
      size_t stackSize,
      const marl::function<void()>& func);

  // switchTo() immediately switches execution to the given fiber.
  // switchTo() must be called on the currently executing fiber.
  void switchTo(OSFiber* fiber);

 private:
  static void __stdcall run(void* self);
  // LPVOID
  void* fiber = nullptr;
  bool isFiberFromThread = false;
  marl::function<void()> target;
};

}  // namespace marl
