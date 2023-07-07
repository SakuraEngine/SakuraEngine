#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API ProxyWidget : public Widget {
    SKR_GUI_OBJECT(ProxyWidget, "698b53db-2fd1-4747-99e7-27503a6bcc8a", Widget);

    Widget* child = nullptr;
};
} // namespace skr::gui