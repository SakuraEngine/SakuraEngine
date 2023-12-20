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
    if (auto pointer_up_event = event->type_cast<PointerUpEvent>())
    {
        _up_event = *pointer_up_event;
        if (_has_preview_up_event)
        {
            _check_up();
        }
        else
        {
            _check_cancel();
            request_reject_all();
        }
        return true;
    }
    return false;
}
bool ClickGestureRecognizer::handle_event_from_widget(Event* event)
{
    if (event->type_is<PointerUpEvent>())
    {
        _has_preview_up_event = true;
        return true;
    }
    return false;
}

// 手势竞争
void ClickGestureRecognizer::accept_gesture(CombinePointerId pointer)
{
    _won_arena = true;
    _check_down();
    if (_has_preview_up_event)
    {
        _check_up();
    }
}
void ClickGestureRecognizer::reject_gesture(CombinePointerId pointer)
{
    _reset();
}

// help functions
void ClickGestureRecognizer::_check_down()
{
    if (on_click_down)
    {
        on_click_down(&_down_event.value());
    }
}
void ClickGestureRecognizer::_check_up()
{
    if (_won_arena && _up_event)
    {
        if (on_click_up)
        {
            on_click_up(&_up_event.value());
        }
        if (on_click)
        {
            on_click();
        }
        _reset();
    }
}
void ClickGestureRecognizer::_check_cancel()
{
    if (on_cancel)
    {
        on_cancel();
    }
}
void ClickGestureRecognizer::_reset()
{
    _down_event           = Optional<PointerDownEvent>{};
    _up_event             = Optional<PointerUpEvent>{};
    _has_preview_up_event = false;
    _won_arena            = false;

    clean_tracing_pointers();
}
} // namespace skr::gui