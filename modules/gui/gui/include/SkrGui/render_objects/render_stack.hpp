#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/layout.hpp"
#include "SkrGui/framework/render_object/multi_child_render_object.hpp"

namespace skr::gui
{

struct SKR_GUI_API RenderStack : public RenderBox, public IMultiChildRenderObject {
public:
    SKR_GUI_OBJECT(RenderStack, "b3c8ede6-d878-472c-a1c1-6b3acdc9f1f0", RenderBox, IMultiChildRenderObject);
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
    void set_stack_alignment(Alignment alignment) SKR_NOEXCEPT;
    void set_child_fit(EPositionalFit fit) SKR_NOEXCEPT;
    void set_stack_size(EStackSize size) SKR_NOEXCEPT;

    struct SlotData {
        Offsetf offset = Offsetf::Zero();
    };

private:
    friend struct _StackHelper;
    Alignment      _stack_alignment = Alignment::TopLeft();
    EPositionalFit _child_fit       = EPositionalFit::PassThrough;
    EStackSize     _stack_size      = EStackSize::Shrink;

    // MIXIN
    MULTI_CHILD_RENDER_OBJECT_MIX_IN(RenderStack, RenderBox, SlotData)
};

} // namespace skr::gui
