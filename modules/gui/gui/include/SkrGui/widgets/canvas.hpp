#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/framework/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Canvas : public MultiChildRenderObjectWidget {
    SKR_GUI_TYPE(Canvas, "94714676-422c-4e4e-a754-e26b5466900f", MultiChildRenderObjectWidget)

    struct Slot {
        Positional layout = Positional::fill();
        int32_t z_index = 0;
        Widget* child = nullptr;
    };

    //==> Begin Construct
    struct Params {
        using WidgetType = Canvas;
        Span<Slot> children;
    };
    void construct(Params params) SKR_NOEXCEPT;
    //==> End Construct
private:
    Array<Slot> _children_slots;
};
} // namespace skr::gui