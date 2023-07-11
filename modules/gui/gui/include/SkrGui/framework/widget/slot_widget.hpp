#pragma once
#include "SkrGui/framework/widget/proxy_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API SlotWidget : public ProxyWidget {
    SKR_GUI_OBJECT(SlotWidget, "698b53db-2fd1-4747-99e7-27503a6bcc8a", ProxyWidget);

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    virtual void apply_slot_data(NotNull<RenderObject*> parent, NotNull<RenderObject*> child) const SKR_NOEXCEPT = 0;
};
} // namespace skr::gui