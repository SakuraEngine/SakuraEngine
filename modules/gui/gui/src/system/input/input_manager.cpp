#include "SkrGui/system/input/input_manager.hpp"
#include "SkrGui/system/input/render_input_context.hpp"
#include "SkrGui/system/input/pointer_event.hpp"

namespace skr::gui
{
// dispatch event
bool InputManager::dispatch_event(Event* event)
{
    if (event->type_is<PointerDownEvent>() || event->type_is<PointerMoveEvent>())
    {
        // TODO. hit test
        auto pointer_event = event->type_cast_fast<PointerEvent>();

        // hit test
        HitTestResult result;
        hit_test(&result, pointer_event->global_position);

        // dispatch event
        PointerMoveEvent event;
        for (const auto& entry : result.path())
        {
            if (entry.target->handle_event(&event, const_cast<HitTestEntry*>(&entry)))
            {
                return true;
            }
        }
    }
    else if (event->type_is<PointerUpEvent>())
    {
        // TODO. restore hit test path and remove
    }
    else if (event->type_is<PointerMoveEvent>())
    {
        auto pointer_event = event->type_cast_fast<PointerScrollEvent>();
        SKR_LOG_INFO(u8"%f, %f", pointer_event->scroll_delta.x, pointer_event->scroll_delta.y);
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

// register
void InputManager::register_context(NotNull<RenderInputContext*> context)
{
    _contexts.add_unique(context.get());
}
void InputManager::unregister_context(NotNull<RenderInputContext*> context)
{
    _contexts.remove(context.get());
}
} // namespace skr::gui