#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"

namespace skr::gui
{
// 会对 child 施加布局偏移的 RenderBox
struct RenderShiftedBox : public RenderBox, public ISingleChildRenderObject {
    SKR_GUI_TYPE(RenderShiftedBox, "58e69c83-86c1-4f86-90e0-a82d4f78038c", RenderBox, ISingleChildRenderObject);

    inline Offset offset() const SKR_NOEXCEPT { return _offset; }
    inline void   set_offset(Offset offset) SKR_NOEXCEPT { _offset = offset; }

protected:
    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // paint
    void paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT override;

private:
    Offset _offset;

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