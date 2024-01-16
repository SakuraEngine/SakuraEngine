#include "SkrGui/system/input/gesture/pointer_gesture_recognizer.hpp"
#include "SkrGui/system/input/input_manager.hpp"

namespace skr::gui
{
void PointerGestureRecognizer::add_pointer(NotNull<Event*> event)
{
    if (auto pointer_down_event = event->type_cast<PointerDownEvent>())
    {
        CombinePointerId id = { pointer_down_event->pointer_id, static_cast<uint32_t>(pointer_down_event->button) };
        if (!_tracing_pointers.find(id))
        {
            GestureArena* arena = input_manager()->gesture_arena_manager()->add_gesture(id, this);
            _tracing_pointers.add(id, arena);
            on_pointer_added(pointer_down_event);
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"PointerGestureRecognizer only accept PointerDownEvent for start trace a pointer");
        SKR_UNREACHABLE_CODE()
    }
}

// accept or reject
void PointerGestureRecognizer::request_accept(CombinePointerId pointer)
{
    if (auto found = _tracing_pointers.find(pointer))
    {
        found.value()->accept_gesture(this);
    }
}
void PointerGestureRecognizer::request_reject(CombinePointerId pointer)
{
    if (auto found = _tracing_pointers.find(pointer))
    {
        found.value()->reject_gesture(this);
    }
}
void PointerGestureRecognizer::request_accept_all()
{
    for (auto& pair : _tracing_pointers)
    {
        pair.value->accept_gesture(this);
    }
    _tracing_pointers.clear();
}
void PointerGestureRecognizer::request_reject_all()
{
    for (auto& pair : _tracing_pointers)
    {
        pair.value->reject_gesture(this);
    }
    _tracing_pointers.clear();
}

} // namespace skr::gui