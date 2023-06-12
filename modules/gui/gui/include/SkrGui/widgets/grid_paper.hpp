#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API GridPaper : public LeafRenderObjectWidget {
    SKR_GUI_TYPE(GridPaper, "85e5d8b0-d216-40f4-9932-bb284bc2adf9", LeafRenderObjectWidget)

    //==>Begin Construct
    struct Params {
        using WidgetType = GridPaper;
    };
    inline void construct(Params params) {}
    //==>End Construct
};
} // namespace skr::gui