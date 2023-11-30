#include "SkrGui/system/input/gesture/click_gesture_recognizer.hpp"

namespace skr::gui
{
void ClickGestureRecognizer::on_pointer_added(PointerDownEvent* event)
{
    _down_event = *event;
}

// 事件处理
bool ClickGestureRecognizer::handle_event(Event* event)
{
    // TODO. handle up event
    return false;
}
bool ClickGestureRecognizer::handle_event_from_widget(Event* event)
{
    // TODO. handle up event and record state
    return false;
}

// 手势竞争
void ClickGestureRecognizer::accept_gesture(CombinePointerId pointer)
{
    // TODO. apply recorded state, for this reason, we may need to copy(or keep) input event
}
void ClickGestureRecognizer::reject_gesture(CombinePointerId pointer)
{
}
} // namespace skr::gui