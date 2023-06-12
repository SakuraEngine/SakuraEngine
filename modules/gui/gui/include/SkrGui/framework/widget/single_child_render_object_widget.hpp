#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API SingleChildRenderObjectWidget : public RenderObjectWidget {
    SKR_GUI_TYPE(SingleChildRenderObjectWidget, "1263dfea-b865-4aa1-b6c1-c1d05062e559", RenderObjectWidget);

    // create_element
protected:
    Widget* _child;
};
} // namespace skr::gui