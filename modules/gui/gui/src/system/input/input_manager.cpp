#include "SkrGui/system/input/input_manager.hpp"
#include "SkrGui/system/input/event.hpp"
#include "SkrGui/system/input/render_input_context.hpp"
#include "SkrGui/system/input/pointer_event.hpp"

namespace skr::gui
{
// dispatch event
bool InputManager::dispatch_event(Event* event)
{
    HitTestResult result;

    if (event->type_is<PointerDownEvent>() || event->type_is<PointerMoveEvent>())
    {
        auto pointer_event = event->type_cast_fast<PointerEvent>();

        // do hit test
        hit_test(&result, pointer_event->global_position);

        if (event->type_is<PointerMoveEvent>())
        {
            _dispatch_enter_exit(&result, pointer_event->type_cast_fast<PointerMoveEvent>());
        }

        return route_event(&result, event->type_cast<PointerEvent>());
    }
    else if (event->type_is<PointerUpEvent>())
    {
        // TODO. restore hit test path and remove
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