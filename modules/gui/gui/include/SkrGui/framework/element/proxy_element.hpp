#pragma once
#include "SkrGui/framework/element/component_element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API ProxyElement : public ComponentElement {
    SKR_GUI_OBJECT(ProxyElement, "752d1492-e111-4f86-b50e-954ee871e53c", ComponentElement)
    using Super = ComponentElement;
    using Super::Super;

    // build & update
    void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    Widget* build() SKR_NOEXCEPT override;

    virtual void updated(NotNull<ProxyWidget*> old_widget) = 0;
};
} // namespace skr::gui