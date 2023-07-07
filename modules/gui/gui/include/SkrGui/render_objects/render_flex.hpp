#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/multi_child_render_object.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
class SKR_GUI_API RenderFlex : public RenderBox, public IMultiChildRenderObject
{
public:
    SKR_GUI_OBJECT(RenderFlex, "d3987dfd-24d2-478a-910e-537f24c4bae7", RenderBox, IMultiChildRenderObject);
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

} // namespace skr::gui