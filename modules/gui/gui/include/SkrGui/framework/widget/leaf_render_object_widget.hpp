#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widget/leaf_render_object_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "bd2562c4-832f-4df2-a56a-8f0424c6bae4",
    "rtti": true
)
SKR_GUI_API LeafRenderObjectWidget : public RenderObjectWidget
{
    SKR_RTTR_GENERATE_BODY()

    NotNull<Element*> create_element() SKR_NOEXCEPT override;
};
} // namespace gui sreflect
} // namespace skr sreflect