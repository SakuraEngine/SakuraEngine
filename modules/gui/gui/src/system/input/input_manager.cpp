#include "SkrGui/system/input/input_manager.hpp"
#include "SkrGui/system/input/event.hpp"
#include "SkrGui/system/input/pointer_event.hpp"
#include "SkrGui/system/input/gesture/gesture_recognizer.hpp"
#include "SkrGui/framework/render_object/render_native_window.hpp"

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

        // dispatch to widget
        bool handled = route_event(&result, pointer_move_event);

        // dispatch to gesture
        handled |= _gesture_arena_manager.route_event(pointer_move_event);

        return handled;
    }
    else if (auto pointer_up_event = event->type_cast<PointerUpEvent>())
    {
        // do hit test
        HitTestResult result;
        hit_test(&result, pointer_down_event->global_position);

        // dispatch to widget
        bool handled = route_event(&result, pointer_up_event);

        // dispatch to gesture
        handled |= _gesture_arena_manager.route_event(pointer_up_event);

        return handled;
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
void InputManager::register_context(NotNull<RenderNativeWindow*> context)
{
    _contexts.add_unique(context.get());
}
void InputManager::unregister_context(NotNull<RenderNativeWindow*> context)
{
    _contexts.remove(context.get());
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