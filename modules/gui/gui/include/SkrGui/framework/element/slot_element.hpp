#pragma once
#include "SkrGui/framework/element/proxy_element.hpp"

namespace skr::gui
{
struct SlotElement : public ProxyElement {
    SKR_GUI_OBJECT(SlotElement, "70e475e0-193e-4945-a17b-0c358621a1b3", ProxyElement)
    using Super = ProxyElement;
    using Super::Super;

    void updated(NotNull<ProxyWidget*> old_widget) override;
};
} // namespace skr::gui