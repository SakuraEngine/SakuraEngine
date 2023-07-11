#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"

namespace skr::gui
{
// 会对 child 施加布局偏移的 RenderBox
struct RenderShiftedBox : public RenderBox, public ISingleChildRenderObject {
    SKR_GUI_OBJECT(RenderShiftedBox, "58e69c83-86c1-4f86-90e0-a82d4f78038c", RenderBox, ISingleChildRenderObject);

    inline Offsetf offset() const SKR_NOEXCEPT { return _offset; }
    inline void    set_offset(Offsetf offset) SKR_NOEXCEPT { _offset = offset; }

protected:
    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // paint
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

private:
    Offsetf _offset = {};

    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderShiftedBox, RenderBox)
};
} // namespace skr::gui