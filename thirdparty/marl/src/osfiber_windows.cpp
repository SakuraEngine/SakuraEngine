#ifdef _WIN32
#include "marl/debug.h"
#include "marl/osfiber_windows.h"
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <memory>

namespace marl
{
OSFiber::~OSFiber() {
  if (fiber != nullptr) {
    if (isFiberFromThread) {
      ConvertFiberToThread();
    } else {
      DeleteFiber(fiber);
    }
  }
}

Allocator::unique_ptr<OSFiber> OSFiber::createFiberFromCurrentThread(
    Allocator* allocator) {
  auto out = allocator->make_unique<OSFiber>();
  out->fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
  out->isFiberFromThread = true;
  MARL_ASSERT(out->fiber != nullptr,
              "ConvertThreadToFiberEx() failed with error 0x%x",
              int(GetLastError()));
  return out;
}

Allocator::unique_ptr<OSFiber> OSFiber::createFiber(
    Allocator* allocator,
    size_t stackSize,
    const marl::function<void()>& func) {
  auto out = allocator->make_unique<OSFiber>();
  // stackSize is rounded up to the system's allocation granularity (typically
  // 64 KB).
  out->fiber = CreateFiberEx(stackSize - 1, stackSize, FIBER_FLAG_FLOAT_SWITCH,
                             &OSFiber::run, out.get());
  out->target = func;
  MARL_ASSERT(out->fiber != nullptr, "CreateFiberEx() failed with error 0x%x",
              int(GetLastError()));
  return out;
}

void OSFiber::switchTo(OSFiber* to) {
  SwitchToFiber(to->fiber);
}

void __stdcall OSFiber::run(void* self) {
  marl::function<void()> func;
  marl::swap(func, reinterpret_cast<OSFiber*>(self)->target);
  func();
}
}
#endif