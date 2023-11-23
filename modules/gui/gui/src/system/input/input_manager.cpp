#include "SkrGui/system/input/input_manager.hpp"
#include "SkrGui/system/input/event.hpp"
#include "SkrGui/system/input/render_input_context.hpp"
#include "SkrGui/system/input/pointer_event.hpp"
#include "SkrGui/system/input/gesture_recognizer.hpp"

namespace skr::gui
{
// dispatch event
bool InputManager::dispatch_event(Event* event)
{
    if (auto pointer_down_event = event->type_cast<PointerDownEvent>())
    {
        // do hit test
        HitTestResult result;
        hit_test(&result, pointer_down_event->global_position);

        // route
        return route_event(&result, pointer_down_event);
    }
    else if (auto pointer_move_event = event->type_cast<PointerMoveEvent>())
    {
        // do hit test
        HitTestResult result;
        hit_test(&result, pointer_move_event->global_position);

        // handle enter & exit
        _dispatch_enter_exit(&result, pointer_move_event->type_cast_fast<PointerMoveEvent>());

        // dispatch to gesture or route to widget
        if (route_event_for_gesture(pointer_move_event))
        {
            return true;
        }
        else
        {
            // route
            return route_event(&result, pointer_move_event);
        }
    }
    else if (auto pointer_up_event = event->type_cast<PointerUpEvent>())
    {
        // do hit test
        HitTestResult result;
        hit_test(&result, pointer_down_event->global_position);

        // dispatch to gesture or route to widget
        if (route_event_for_gesture(pointer_up_event))
        {
            return true;
        }
        else
        {
            // route
            return route_event(&result, pointer_up_event);
        }
    }

    else // pan/zoom & scroll/scale
    {
    }

    return false;
}

// hit test
bool InputManager::hit_test(HitTestResult* result, Offsetf system_location)
{
    for (const auto& context : _contexts)
    {
        Offsetf local_position = context->system_to_local(system_location);
        if (context->hit_test(result, local_position))
        {
            return true;
        }
    }
    return false;
}

// route event
bool InputManager::route_event(HitTestResult* result, PointerEvent* event, EEventRoutePhase phase)
{
    if (result->empty())
    {
        return false;
    }

    if (flag_any(phase, EEventRoutePhase::TrickleDown))
    {
        for (uint64_t i = 0; i < result->path().size(); ++i)
        {
            auto& entry = result->path()[result->path().size() - i - 1];
            if (entry.target->handle_event(event, const_cast<HitTestEntry*>(&entry)))
            {
                return true;
            }
        }
    }
    else if (flag_any(phase, EEventRoutePhase::Reach))
    {
        auto& entry = result->path()[0];
        if (entry.target->handle_event(event, const_cast<HitTestEntry*>(&entry)))
        {
            return true;
        }
    }
    else if (flag_any(phase, EEventRoutePhase::Broadcast))
    {
        bool handled = false;
        for (const auto& entry : result->path())
        {
            handled |= entry.target->handle_event(event, const_cast<HitTestEntry*>(&entry));
        }
        if (handled)
        {
            return true;
        }
    }
    else if (flag_any(phase, EEventRoutePhase::BubbleUp))
    {
        for (const auto& entry : result->path())
        {
            if (entry.target->handle_event(event, const_cast<HitTestEntry*>(&entry)))
            {
                return true;
            }
        }
    }

    return false;
}

// register
void InputManager::register_context(NotNull<RenderInputContext*> context)
{
    _contexts.add_unique(context.get());
}
void InputManager::unregister_context(NotNull<RenderInputContext*> context)
{
    _contexts.remove(context.get());
}

// gesture
void InputManager::add_gesture(NotNull<GestureRecognizer*> gesture)
{
    _gestures.add_unique(gesture.get());
}
void InputManager::remove_gesture(NotNull<GestureRecognizer*> gesture)
{
    _gestures.remove(gesture.get());
}
bool InputManager::route_event_for_gesture(PointerEvent* event)
{
    bool handled = false;
    for (auto gesture : _gestures)
    {
        handled |= gesture->handle_event(event);
    }
    return handled;
}

// complex dispatch functional
void InputManager::_dispatch_enter_exit(HitTestResult* result, PointerMoveEvent* event)
{
    for (auto& entry : result->path())
    {
        if (!_last_hover_path.path().contain_if([&](const auto& other_entry) {
                return entry.target == other_entry.target;
            }))
        {
            PointerEnterEvent enter_event;
            entry.target->handle_event(&enter_event, const_cast<HitTestEntry*>(&entry));
        }
    }

    for (auto& entry : _last_hover_path.path())
    {
        if (!result->path().contain_if([&](const auto& other_entry) {
                return entry.target == other_entry.target;
            }))
        {
            PointerExitEvent exit_event;
            entry.target->handle_event(&exit_event, const_cast<HitTestEntry*>(&entry));
        }
    }

    _last_hover_path = *result;
}
} // namespace skr::gui