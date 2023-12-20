#pragma once
#include "SkrGui/system/input/gesture/gesture_arena.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/gesture_arena_manager.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct("guid": "6b1d8469-6958-4800-affa-bf1e12cd5197")
GestureArenaManager {

private:
    UMap<CombinePointerId, GestureArena> _arenas = {};
};
} // namespace gui sreflect
} // namespace skr sreflect