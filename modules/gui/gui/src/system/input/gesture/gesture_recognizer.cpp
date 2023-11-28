#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"

namespace skr::gui
{
// 监听 pointer
void GestureRecognizer::add_pointer(NotNull<PointerDownEvent*> event)
{
}
void GestureRecognizer::add_allowed_pointer(NotNull<PointerDownEvent*> event)
{
}

// 事件处理
bool GestureRecognizer::handle_event(Event* event)
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