#include "SkrGui/framework/element/proxy_element.hpp"
#include "SkrGui/framework/widget/proxy_widget.hpp"

namespace skr::gui
{
// build & update
void ProxyElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    auto old_proxy_widget = widget()->type_cast<ProxyWidget>();
    Super::update(new_widget);
    updated(old_proxy_widget);
    rebuild(true);
}

Widget* ProxyElement::build() SKR_NOEXCEPT
{
    auto proxy_widget = widget()->type_cast<ProxyWidget>();
    return proxy_widget->child;
}
} // namespace skr::gui