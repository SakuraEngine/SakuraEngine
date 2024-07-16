#pragma once
#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/gesture_arena.generated.h"
#endif

namespace skr::gui
{
sreflect_enum_class("guid": "64d988e6-5d8c-4418-b070-2ae6ec3ddf08")
GestureArenaState
{
    Opened,   // 开启，可以向内注册手势
    Closed,   // 关闭，并进行手势决议，不可以向内注册手势
    Resolved, // 决议完成，等待下一次开启
};

// Arena 生命周期：
sreflect_struct("guid": "952528b4-510f-450f-81fa-cbdb3d8b2d4a")
SKR_GUI_API GestureArena {
    GestureArena(CombinePointerId pointer);

    void open();
    void add(GestureRecognizer* gesture);
    void close();
    void sweep();
    void hold();
    void release();
    void accept_gesture(GestureRecognizer* member);
    void reject_gesture(GestureRecognizer* member);

    // getter
    inline GestureArenaState                state() const { return _state; }
    inline const Array<GestureRecognizer*>& members() { return _members; }
    inline bool                             is_held() const { return _is_held; }

private:
    // helper functions
    void _try_to_resolve_arena();
    void _resolve_in_favor_of(GestureRecognizer* member);

private:
    CombinePointerId          _pointer = {};
    Array<GestureRecognizer*> _members = {};

    GestureArenaState _state             = GestureArenaState::Opened;
    bool              _is_held           = false; // 是否需要维持生命周期以进行例如双击等长周期的手势竞争
    bool              _has_pending_sweep = false; // 用于标识结束生命周期维持后，是否需要进行 sweep

    GestureRecognizer* eager_winner = nullptr;
};

} // namespace skr::gui