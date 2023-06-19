#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{

class SKR_GUI_API RenderFlex : public RenderBox
{
public:
    SKR_GUI_TYPE(RenderFlex, "d3987dfd-24d2-478a-910e-537f24c4bae7", RenderBox);
    using Super = RenderBox;

    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // dry layout
    Size compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT override;

    // layout
    void perform_layout() SKR_NOEXCEPT override;

    struct Slot {
        float   flex = 1;
        FlexFit flex_fit = FlexFit::Loose;

        Offset     offset = Offset::zero();
        RenderBox* child = nullptr;
    };

private:
    friend struct _FlexHelper;
    FlexDirection      _flex_direction = FlexDirection::Row;
    MainAxisAlignment  _main_axis_alignment = MainAxisAlignment::Start;
    CrossAxisAlignment _cross_axis_alignment = CrossAxisAlignment::Start;
    MainAxisSize       _main_axis_size = MainAxisSize::Max;
    Array<Slot>        _flexible_slots;

    float _overflow;
};

} // namespace skr::gui