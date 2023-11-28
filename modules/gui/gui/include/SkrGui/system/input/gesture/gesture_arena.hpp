#pragma once
#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/gesture_arena.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{

sreflect_struct("guid": "952528b4-510f-450f-81fa-cbdb3d8b2d4a")
SKR_GUI_API GestureArena {
    void add(GestureRecognizer* gesture);
    bool close();
    bool sweep(CombinePointerId pointer);
    void hold();
    void release(CombinePointerId pointer);

private:
    // helper functions
    void _resolve(GestureRecognizer* member, bool accept);
    bool _try_to_resolve_arena();
    void _resolve_in_favor_of(GestureRecognizer* member);

private:
    CombinePointerId          _pointer = {};
    Array<GestureRecognizer*> _members = {};

    bool _is_open           = true;  // 开启期间允许手势加入，开启期间通常指 down 事件 dispatch 期间
    bool _is_held           = false; // 是否需要维持生命周期以进行例如双击等长周期的手势竞争
    bool _has_pending_sweep = false; // 用于标识结束生命周期维持后，是否需要进行 sweep

    GestureRecognizer* eager_winner = nullptr;
};

} // namespace gui sreflect
} // namespace skr sreflect