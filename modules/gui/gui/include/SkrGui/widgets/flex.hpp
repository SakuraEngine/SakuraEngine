#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Flex : public MultiChildRenderObjectWidget {
    SKR_GUI_OBJECT(Flex, "03fbfa97-39bb-4233-afdb-1f53648e5152", MultiChildRenderObjectWidget)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    EFlexDirection      flex_direction       = EFlexDirection::Row;
    EMainAxisAlignment  main_axis_alignment  = EMainAxisAlignment::Start;
    ECrossAxisAlignment cross_axis_alignment = ECrossAxisAlignment::Start;
    EMainAxisSize       main_axis_size       = EMainAxisSize::Max;
};

} // namespace skr::gui