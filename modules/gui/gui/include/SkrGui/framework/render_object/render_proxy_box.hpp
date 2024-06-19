#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/render_proxy_box.generated.h"
#endif

namespace skr::gui
{

// 代理 Box，其渲染 Sizef 等属性严格由 child 决定，通常起到修饰作用
sreflect_struct(
    "guid": "5b0d4830-6eea-4ab5-91df-2b6d5633f473"
)
RenderProxyBox : public RenderBox,
                 public ISingleChildRenderObject {
    SKR_RTTR_GENERATE_BODY()

    // hit test
    bool hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT override;

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