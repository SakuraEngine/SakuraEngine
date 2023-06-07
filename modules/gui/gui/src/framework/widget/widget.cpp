#include "SkrGui/framework/widget/widget.hpp"
#include "platform/guid.hpp"

namespace skr::gui
{
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
    if (old_widget->type_id() != new_widget->type_id())
    {
        return false;
    }
    return true;
}

} // namespace skr::gui