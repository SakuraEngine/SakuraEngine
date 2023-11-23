#include "SkrGui/system/input/gesture_recognizer.hpp"

namespace skr::gui
{
bool GestureRecognizer::handle_event(Event* event)
{
    return false;
}

void PointerGestureRecognizer::add_pointer_pan_zoom(PointerPanZoomStartEvent* event)
{
}
void PointerGestureRecognizer::add_pointer(PointerDownEvent* event)
{
}
} // namespace skr::gui