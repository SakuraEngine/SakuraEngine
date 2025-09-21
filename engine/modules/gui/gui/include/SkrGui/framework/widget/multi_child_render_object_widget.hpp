#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/widget/multi_child_render_object_widget.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "21ff85e4-9f05-48a8-80bf-7eec604de7f4"
)]] SKR_GUI_API MultiChildRenderObjectWidget : public RenderObjectWidget
{
    SKR_GENERATE_BODY(MultiChildRenderObjectWidget)

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    Array<Widget*> children = {};
};
} // namespace skr::gui