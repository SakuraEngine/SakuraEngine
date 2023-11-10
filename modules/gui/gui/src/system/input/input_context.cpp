#include "SkrGui/system/input/input_context.hpp"
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gui
{
InputContext::InputContext(NotNull<InputManager*> manager, NotNull<RenderBox*> widget)
    : _owner(manager)
    , _widget(widget)
{
}

// hit test
bool InputContext::hit_test(HitTestResult* result, Offsetf global_position)
{
    return false;
}

} // namespace skr::gui