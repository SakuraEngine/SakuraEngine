#pragma once
#include "SkrGui/system/input/gesture/pointer_gesture_recognizer.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/gesture/click_gesture_recognizer.generated.h"
#endif

namespace skr::gui
{

sreflect_struct("guid": "58887860-92da-4f41-a09b-3e91604e4ae0")
SKR_GUI_API ClickGestureRecognizer : public PointerGestureRecognizer {
    SKR_RTTR_GENERATE_BODY()
    using Super = PointerGestureRecognizer;
    using Super::Super;

    void on_pointer_added(PointerDownEvent* event) override;

    // 事件处理
    bool handle_event(Event* event) override;
    bool handle_event_from_widget(Event* event) override;

    // 手势竞争
    void accept_gesture(CombinePointerId pointer) override;
    void reject_gesture(CombinePointerId pointer) override;

private:
    // help functions
    void _check_down();
    void _check_up();
    void _check_cancel();
    void _reset();

public:
    // TODO. click 没有事件，但是是否应该传入一个 context 来存储 modifier key 的情况
    Function<void(PointerDownEvent*)> on_click_down = {};
    Function<void(PointerUpEvent*)>   on_click_up   = {};
    Function<void()>                  on_click      = {};
    Function<void()>                  on_cancel     = {};

private:
    // recorded state
    // TODO. use smart ptr
    Optional<PointerDownEvent> _down_event           = {};
    Optional<PointerUpEvent>   _up_event             = {};
    bool                       _has_preview_up_event = false;
    bool                       _won_arena            = false;
};

} // namespace skr::gui