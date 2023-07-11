#pragma once
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API LeafRenderObjectWidget : public RenderObjectWidget {
    SKR_GUI_OBJECT(LeafRenderObjectWidget, "eb358741-7b29-417e-afb5-adc8685c6e82", RenderObjectWidget)

    NotNull<Element*> create_element() SKR_NOEXCEPT override;
};
} // namespace skr::gui