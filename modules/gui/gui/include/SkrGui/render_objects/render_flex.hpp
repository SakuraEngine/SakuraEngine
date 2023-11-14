#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/multi_child_render_object.hpp"
#include "SkrGui/math/layout.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_flex.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "1bc957ef-1203-489d-911d-94ba3fb81080"
)
SKR_GUI_API RenderFlex : public RenderBox,
                         public IMultiChildRenderObject {
public:
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderBox;

    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // dry layout
    Sizef compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT override;

    // layout
    void perform_layout() SKR_NOEXCEPT override;

    // paint
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

    // setter
    void set_flex_direction(EFlexDirection value) SKR_NOEXCEPT;
    void set_main_axis_alignment(EMainAxisAlignment value) SKR_NOEXCEPT;
    void set_cross_axis_alignment(ECrossAxisAlignment value) SKR_NOEXCEPT;
    void set_main_axis_size(EMainAxisSize value) SKR_NOEXCEPT;

    // hit test
    bool hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT override;

    // transform
    void apply_paint_transform(NotNull<const RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT override;

    struct SlotData {
        // slot data
        float    flex     = 1;
        EFlexFit flex_fit = EFlexFit::Loose;

        // child data
        Offsetf offset = Offsetf::Zero();
    };

private:
    friend struct _FlexHelper;
    EFlexDirection      _flex_direction       = EFlexDirection::Row;
    EMainAxisAlignment  _main_axis_alignment  = EMainAxisAlignment::Start;
    ECrossAxisAlignment _cross_axis_alignment = ECrossAxisAlignment::Start;
    EMainAxisSize       _main_axis_size       = EMainAxisSize::Max;

    float _overflow = 0.0f;

    // MIXIN
    MULTI_CHILD_RENDER_OBJECT_MIX_IN(RenderFlex, RenderBox, SlotData)
};
} // namespace gui sreflect
} // namespace skr sreflect