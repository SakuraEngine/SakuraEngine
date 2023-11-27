#include "SkrGui/system/input/gesture_recognizer.hpp"

namespace skr::gui
{
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