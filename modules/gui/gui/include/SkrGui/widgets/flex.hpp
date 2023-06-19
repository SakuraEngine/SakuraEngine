#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Flex : public MultiChildRenderObjectWidget {
    SKR_GUI_TYPE(Flex, "03fbfa97-39bb-4233-afdb-1f53648e5152", MultiChildRenderObjectWidget)

    JustifyContent justify_content;
    FlexDirection  flex_direction;
    AlignItems     align_items;
    Array<Widget*> flex_children;
};

} // namespace skr::gui