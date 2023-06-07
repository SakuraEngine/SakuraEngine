#include "SkrGui/framework/widget/widget.hpp"
#include "platform/guid.hpp"

namespace skr::gui
{
SKR_GUI_TYPE_IMPLEMENTATION(Widget);

not_null<Element*> Widget::create_element() noexcept
{
    SKR_UNREACHABLE_CODE();
    return not_null<Element*>{ (Element*)0 };
}

bool Widget::can_update(not_null<Widget*> old_widget, not_null<Widget*> new_widget) noexcept
{
    if (old_widget->key() != new_widget->key())
    {
        return false;
    }
    if (old_widget->get_type() != new_widget->get_type())
    {
        return false;
    }
    return true;
}

} // namespace skr::gui