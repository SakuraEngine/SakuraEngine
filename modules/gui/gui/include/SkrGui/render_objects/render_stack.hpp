#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{

struct SKR_GUI_API RenderStack : public RenderBox {
public:
    SKR_GUI_TYPE(RenderStack, "b3c8ede6-d878-472c-a1c1-6b3acdc9f1f0", RenderBox);
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

    // paint
    void paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT override;

    struct Slot {
        // slot data

        // child data
        Offset     offset = Offset::Zero();
        RenderBox* child = nullptr;
    };

private:
    friend struct _StackHelper;
    Alignment      _stack_alignment = Alignment::TopLeft();
    EPositionalFit _child_fit = EPositionalFit::PassThrough;
    EStackSize     _stack_size = EStackSize::Shrink;
    Array<Slot>    _stack_slots;
};

} // namespace skr::gui
