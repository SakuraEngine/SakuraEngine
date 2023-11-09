#include "SkrGui/system/input/input_context.hpp"

namespace skr::gui
{
InputContext::InputContext(NotNull<RenderBox*> widget)
    : widget(widget)
{
}

} // namespace skr::gui