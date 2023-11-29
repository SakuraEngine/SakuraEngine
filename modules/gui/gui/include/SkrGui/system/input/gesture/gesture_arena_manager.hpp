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
    ~GestureArenaManager();

    GestureArena* open_if_resolved(CombinePointerId pointer);
    GestureArena* add_gesture(CombinePointerId pointer, GestureRecognizer* gesture);

    GestureArena* find_arena(CombinePointerId pointer);
    GestureArena* find_arena_or_add(CombinePointerId pointer);

private:
    UMap<CombinePointerId, GestureArena*> _arenas = {};
};
} // namespace gui sreflect
} // namespace skr sreflect