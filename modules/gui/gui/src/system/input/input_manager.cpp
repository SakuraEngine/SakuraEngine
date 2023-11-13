#include "SkrGui/system/input/input_manager.hpp"
#include "SkrGui/system/input/render_input_context.hpp"

namespace skr::gui
{
// hit test
bool InputManager::hit_test(HitTestResult* result, Offsetf global_position)
{
    for (const auto& context : _contexts)
    {
        if (context->hit_test(result, global_position))
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