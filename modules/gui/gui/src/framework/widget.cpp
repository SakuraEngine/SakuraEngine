#include "SkrGui/framework/widget.hpp"
#include "platform/guid.hpp"

namespace skr {
namespace gui {

SKR_GUI_TYPE_IMPLMENTATION(Widget);

not_null<Element*> Widget::create_element() noexcept
{
    SKR_UNREACHABLE_CODE();
    return {(Element*)0};
}

bool Widget::CanUpdate(not_null<Widget*> old_widget, not_null<Widget*> new_widget) noexcept
{
    if(old_widget->key != new_widget->key)
    {
        return false;
    }
    if (old_widget->get_type() != new_widget->get_type())
    {
        return false;
    }
    return true;
}

} }