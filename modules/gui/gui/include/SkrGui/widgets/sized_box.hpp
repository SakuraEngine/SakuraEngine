#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API SizedBox : public SingleChildRenderObjectWidget {
    SKR_GUI_TYPE(SizedBox, "26d686b6-06b3-4514-af3d-f1354b53b884", SingleChildRenderObjectWidget)

    Size size;
};
} // namespace skr::gui