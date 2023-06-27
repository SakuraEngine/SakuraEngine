#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderColorPicker : public RenderBox {
    SKR_GUI_OBJECT(RenderColorPicker, "25a95354-b3fa-4729-b06f-1a85d0f227c4", RenderBox);
    using Super = RenderBox;

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}
};

} // namespace skr::gui
