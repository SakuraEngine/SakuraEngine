#include "SkrGui/system/input/input_manager.hpp"
#include "SkrGui/system/input/render_input_context.hpp"

namespace skr::gui
{
// hit test
bool InputManager::hit_test(HitTestResult* result, Offsetf system_location)
{
    for (const auto& context : _contexts)
    {
        Offsetf local_position = context->system_to_local(system_location);
        // SKR_LOG_INFO(u8"system: (%f, %f) local:(%f, %f)", system_location.x, system_location.y, local_position.x, local_position.y);
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