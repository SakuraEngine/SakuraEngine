#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"

namespace skr::gui
{
// 监听 pointer
void GestureRecognizer::add_pointer(NotNull<Event*> event)
{
}

// 事件处理
bool GestureRecognizer::handle_event(Event* event)
{
    return false;
}
bool GestureRecognizer::handle_event_from_widget(Event* event)
{
    return false;
}

// 手势竞争
void GestureRecognizer::accept_gesture(CombinePointerId pointer)
{
}
void GestureRecognizer::reject_gesture(CombinePointerId pointer)
{
}
} // namespace skr::gui