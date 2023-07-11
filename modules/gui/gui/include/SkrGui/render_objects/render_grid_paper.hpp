#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderGridPaper : public RenderBox {
public:
    SKR_GUI_OBJECT(RenderGridPaper, "13dd33c9-5d56-4b06-94ce-d1c526fe75d0", RenderBox);
    using Super = RenderBox;

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}
};

} // namespace skr::gui