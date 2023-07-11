#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API MultiChildRenderObjectWidget : public RenderObjectWidget {
    SKR_GUI_OBJECT(MultiChildRenderObjectWidget, "d9e3dde8-f6e2-48bc-8a33-13f109ea5149", RenderObjectWidget);

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    Array<Widget*> children = {};
};
} // namespace skr::gui