#include "SkrGui/framework/widget/widget.hpp"

namespace skr::gui
{
NotNull<Element*> Widget::create_element() noexcept
{
    SKR_UNREACHABLE_CODE();
    return NotNull<Element*>{ (Element*)0 };
}

bool Widget::can_update(NotNull<Widget*> old_widget, NotNull<Widget*> new_widget) noexcept
{
    if (old_widget->key != new_widget->key)
    {
        return false;
    }
    if (old_widget->iobject_get_typeid() != new_widget->iobject_get_typeid())
    {
        return false;
    }
    return true;
}

} // namespace skr::gui