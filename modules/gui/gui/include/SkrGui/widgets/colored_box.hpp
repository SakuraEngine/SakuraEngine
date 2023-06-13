#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/math/color.hpp"

namespace skr::gui
{
struct SKR_GUI_API ColoredBox : public SingleChildRenderObjectWidget {
    SKR_GUI_TYPE(ColoredBox, "5ae444e6-eb5a-44a1-aeb0-9050c8587795", SingleChildRenderObjectWidget)

    Color color;
};
} // namespace skr::gui