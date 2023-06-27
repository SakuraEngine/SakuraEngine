#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"

namespace skr::gui
{
// 代理 Box，其渲染 Sizef 等属性严格由 child 决定，通常起到修饰作用
struct RenderProxyBox : public RenderBox, public ISingleChildRenderObject {
    SKR_GUI_OBJECT(RenderProxyBox, "7b69daee-a739-4497-b64a-a7421035975d", RenderBox, ISingleChildRenderObject);

protected:
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

    // MIXIN
    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderProxyBox, RenderBox);
};
} // namespace skr::gui