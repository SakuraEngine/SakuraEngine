#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/render_objects/render_color_picker.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "f9de7eb7-9431-4dde-a06d-2fb9bc211bb9"
)]] SKR_GUI_API RenderColorPicker : public RenderBox
{
    SKR_GENERATE_BODY(RenderColorPicker)

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}

    // hit test
    bool hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT override;
};

} // namespace skr::gui
