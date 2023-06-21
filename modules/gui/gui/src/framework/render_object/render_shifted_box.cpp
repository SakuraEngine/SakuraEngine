#include "SkrGui/framework/render_object/render_shifted_box.hpp"
#include "SkrGui/framework/painting_context.hpp"

namespace skr::gui
{
// intrinsic size
float RenderShiftedBox::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return child() ? child()->get_min_intrinsic_width(height) : 0.f;
}
float RenderShiftedBox::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return child() ? child()->get_max_intrinsic_width(height) : 0.f;
}
float RenderShiftedBox::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return child() ? child()->get_min_intrinsic_height(width) : 0.f;
}
float RenderShiftedBox::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return child() ? child()->get_max_intrinsic_height(width) : 0.f;
}

// paint
void RenderShiftedBox::paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT
{
    if (child())
    {
        context->paint_child(make_not_null(child()), this->offset() + offset);
    }
}

//==> MIXIN: single child render object
SKR_GUI_TYPE_ID RenderShiftedBox::accept_child_type() const noexcept { return SKR_GUI_TYPE_ID_OF_STATIC(RenderBox); }
void            RenderShiftedBox::set_child(RenderObject* child) noexcept
{
    if (_child) drop_child(make_not_null(_child));
    _child = child->type_cast_fast<RenderBox>();
    if (_child) adopt_child(make_not_null(_child));
}
void RenderShiftedBox::flush_depth() noexcept
{
    RenderBox::flush_depth();
    if (_child) _child->flush_depth();
}
void RenderShiftedBox::visit_children(FunctionRef<void(RenderObject*)> visitor) const noexcept
{
    if (_child) visitor(_child);
}
void RenderShiftedBox::visit_children_recursive(FunctionRef<void(RenderObject*)> visitor) const noexcept
{
    if (_child)
    {
        visitor(_child);
        _child->visit_children_recursive(visitor);
    }
}
void RenderShiftedBox::attach(NotNull<PipelineOwner*> owner) noexcept
{
    RenderBox ::attach(owner);
    if (_child) _child->attach(owner);
}
void RenderShiftedBox::detach() noexcept
{
    if (_child) _child->detach();
    RenderBox::detach();
}
//==> MIXIN: single child render object
} // namespace skr::gui
