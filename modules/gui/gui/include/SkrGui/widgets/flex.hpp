#pragma once
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/flex.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "4a3df0ec-b346-4f5b-a1bd-a46763a7818d"
)
SKR_GUI_API Flex : public MultiChildRenderObjectWidget {
    SKR_RTTR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    EFlexDirection      flex_direction       = EFlexDirection::Row;
    EMainAxisAlignment  main_axis_alignment  = EMainAxisAlignment::Start;
    ECrossAxisAlignment cross_axis_alignment = ECrossAxisAlignment::Start;
    EMainAxisSize       main_axis_size       = EMainAxisSize::Max;
};
} // namespace gui sreflect
} // namespace skr sreflect