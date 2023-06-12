#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/framework/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Canvas : public MultiChildRenderObjectWidget {
    SKR_GUI_TYPE(Canvas, "94714676-422c-4e4e-a754-e26b5466900f", MultiChildRenderObjectWidget)

    struct Slot {
        Positional positional = Positional::Fill();
        int32_t    z_index = 0;
        Widget*    child = nullptr;
    };
    Array<Slot> canvas_children;
};
} // namespace skr::gui