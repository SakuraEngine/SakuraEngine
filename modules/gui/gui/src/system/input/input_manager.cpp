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