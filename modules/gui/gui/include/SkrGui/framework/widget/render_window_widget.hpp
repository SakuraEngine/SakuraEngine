#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderWindowWidget : public RenderObjectWidget {
    SKR_GUI_OBJECT(RenderWindowWidget, "e6dd9e94-ff5e-419e-a58b-eab8c7c1c9c5", RenderObjectWidget)
    Widget* child = nullptr;
};
} // namespace skr::gui