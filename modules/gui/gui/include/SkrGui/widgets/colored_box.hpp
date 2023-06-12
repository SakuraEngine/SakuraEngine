#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/framework/color.hpp"

namespace skr::gui
{
struct SKR_GUI_API ColoredBox : public SingleChildRenderObjectWidget {
    SKR_GUI_TYPE(ColoredBox, "5ae444e6-eb5a-44a1-aeb0-9050c8587795", SingleChildRenderObjectWidget)

    //==> Begin Constructor
    struct Params {
        using WidgetType = ColoredBox;
        Color   color = {};
        Widget* child = nullptr;
    };
    inline void construct(Params params) SKR_NOEXCEPT
    {
        _color = params.color;
        _child = params.child;
    }
    //==> End Constructor

private:
    Color _color;
};
} // namespace skr::gui