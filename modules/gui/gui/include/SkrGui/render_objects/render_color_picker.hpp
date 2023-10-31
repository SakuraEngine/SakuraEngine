#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_color_picker.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "f9de7eb7-9431-4dde-a06d-2fb9bc211bb9",
    "rtti": true
)
SKR_GUI_API RenderColorPicker : public RenderBox {
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderBox;

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}

    // hit test
    bool hit_test_self(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT override;
};

} // namespace gui sreflect
} // namespace skr sreflect
