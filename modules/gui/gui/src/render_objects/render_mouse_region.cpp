#include "SkrGui/render_objects/render_mouse_region.hpp"
#include "SkrGui/system/input/pointer_event.hpp"

namespace skr::gui
{
bool RenderMouseRegion::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    return Super::hit_test(result, local_position);
}
bool RenderMouseRegion::handle_event(NotNull<PointerEvent*> event, NotNull<HitTestEntry*> entry)
{
    if (auto move_event = event->type_cast<PointerMoveEvent>())
    {
        if (on_hover)
        {
            return on_hover(move_event);
        }
    }
    else if (auto enter_event = event->type_cast<PointerEnterEvent>())
    {
        if (on_enter)
        {
            return on_enter(enter_event);
        }
    }
    else if (auto exit_event = event->type_cast<PointerExitEvent>())
    {
        if (on_exit)
        {
            return on_exit(exit_event);
        }
    }
    else if (auto down_event = event->type_cast<PointerDownEvent>())
    {
        if (on_down)
        {
            return on_down(down_event);
        }
    }
    else if (auto up_event = event->type_cast<PointerUpEvent>())
    {
        if (on_up)
        {
            return on_up(up_event);
        }
    }
    return false;
}
} // namespace skr::gui