#pragma once
#include "SkrGui/system/input/gesture_recognizer.hpp"

namespace skr sreflect
{
namespace gui sreflect
{

sreflect_struct("guid": "952528b4-510f-450f-81fa-cbdb3d8b2d4a")
SKR_GUI_API GestureArena {
    struct PointerArenaData {
        Array<GestureRecognizer*> members = {};

        bool is_open           = false; // 开启期间允许手势加入，开启期间通常指 down 事件 dispatch 期间
        bool is_held           = false; // 是否需要维持生命周期以进行例如双击等长周期的手势竞争
        bool has_pending_sweep = false; // 用于标识结束生命周期维持后，是否需要进行 sweep
    };

    void add(CombinePointerId pointer, GestureRecognizer* gesture)
    {
    }
    void close(CombinePointerId pointer)
    {
    }
    void sweep(CombinePointerId pointer)
    {
    }
    void hold(CombinePointerId pointer)
    {
    }
    void release(CombinePointerId pointer)
    {
    }

private:
    // helper functions

private:
    UMap<CombinePointerId, PointerArenaData> _arenas = {};
};

} // namespace gui sreflect
} // namespace skr sreflect