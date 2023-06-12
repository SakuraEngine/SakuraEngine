#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API Text : public LeafRenderObjectWidget {
    SKR_GUI_TYPE(Text, "b852a75d-8035-422f-8738-e251b15acb26", LeafRenderObjectWidget)

    String text;
};
} // namespace skr::gui