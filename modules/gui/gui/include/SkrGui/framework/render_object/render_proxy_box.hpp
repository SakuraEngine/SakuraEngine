#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"

namespace skr::gui
{
// 代理 Box，其渲染 Size 等属性严格由 child 决定，通常起到修饰作用
struct RenderProxyBox : public RenderBox, public ISingleChildRenderObject {
    SKR_GUI_TYPE(RenderProxyBox, "7b69daee-a739-4497-b64a-a7421035975d", RenderBox, ISingleChildRenderObject);

protected:
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

    //==> MIXIN: single child render object
public:
    SKR_GUI_TYPE_ID   accept_child_type() const noexcept override;
    void              set_child(RenderObject* child) noexcept override;
    void              flush_depth() noexcept override;
    void              visit_children(function_ref<void(RenderObject*)> visitor) const noexcept override;
    void              visit_children_recursive(function_ref<void(RenderObject*)> visitor) const noexcept override;
    void              attach(NotNull<PipelineOwner*> owner) noexcept override;
    void              detach() noexcept override;
    inline RenderBox* child() const noexcept { return _child; }

private:
    RenderBox* _child;
    //==> MIXIN: single child render object
};
} // namespace skr::gui