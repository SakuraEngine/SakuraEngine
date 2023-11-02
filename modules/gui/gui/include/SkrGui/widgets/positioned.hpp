#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/positioned.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "649db60e-3fa3-4e45-9de9-0ca572950259",
    "rtti": true
)
SKR_GUI_API Positioned : public SingleChildRenderObjectWidget
{
    SKR_RTTR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    Positional positional = {};
};
} // namespace gui sreflect
} // namespace skr sreflect