#pragma once
#include "SkrGui/framework/widget/proxy_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API SlotWidget : public ProxyWidget {
    SKR_GUI_TYPE(SlotWidget, "698b53db-2fd1-4747-99e7-27503a6bcc8a", ProxyWidget);
};
} // namespace skr::gui