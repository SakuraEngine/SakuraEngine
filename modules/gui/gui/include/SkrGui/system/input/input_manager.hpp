#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/system/input/hit_test.hpp"
#include "SkrGui/system/input/event.hpp"
#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/input_manager.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct RenderInputContext;
struct PointerMoveEvent;
struct GestureRecognizer;

// TODO. roadmap
// //  1. hit test
// //  2. pointer events
// //  3. pointer events bind
//  4. 实现 MouseRegion（同时补全 state 和 增量更新的机制）
//  5. focus (Pointer)
//  6. keyboard events
//  7. keyboard events bind
//  8. focus & navigation (keyboard)         // ! 可 TODO

// InputManager 管理 Context 提供全局 hit test 支持与事件分发
sreflect_struct("guid": "5c9d7e26-c2a1-4785-8832-bda746906801")
SKR_GUI_API InputManager {

    // dispatch event
    bool dispatch_event(Event* event);

    // hit test
    bool hit_test(HitTestResult* result, Offsetf system_location);

    // route event
    bool route_event(HitTestResult* result, PointerEvent* event, EEventRoutePhase phase = EEventRoutePhase::NoBroadcast);

    // register context
    void register_context(NotNull<RenderInputContext*> context);
    void unregister_context(NotNull<RenderInputContext*> context);

    // gesture
    // TODO. add-remove => track-stop_track 并且 dispatch 形式也需要与 PointerId 和 ButtonId 有关
    void add_gesture(NotNull<GestureRecognizer*> gesture);
    void remove_gesture(NotNull<GestureRecognizer*> gesture);
    bool route_event_for_gesture(PointerEvent* event);

private:
    // complex dispatch functional
    void _dispatch_enter_exit(HitTestResult* result, PointerMoveEvent* event);

private:
    Array<RenderInputContext*> _contexts;
    Array<GestureRecognizer*>  _gestures;

    HitTestResult _last_hover_path;
};
} // namespace gui sreflect
} // namespace skr sreflect