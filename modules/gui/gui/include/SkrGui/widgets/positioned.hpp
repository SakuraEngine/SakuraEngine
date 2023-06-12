#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/framework/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Positioned : public SingleChildRenderObjectWidget {
    SKR_GUI_TYPE(Positioned, "7084ef1f-3c12-43d9-b2b0-c6dfa2fab257", SingleChildRenderObjectWidget)

    //==> Begin Construct
    struct Params {
        using WidgetType = Positioned;
        Positional positional;
        Widget*    child = nullptr;
    };
    inline void construct(Params params) SKR_NOEXCEPT
    {
        _positional = params.positional;
        _child = params.child;
    }
    //==> End Construct

private:
    Positional _positional;
};
} // namespace skr::gui