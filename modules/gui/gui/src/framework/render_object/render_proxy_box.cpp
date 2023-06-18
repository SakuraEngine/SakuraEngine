#include "SkrGui/framework/render_object/render_proxy_box.hpp"
#include "SkrGui/framework/painting_context.hpp"

namespace skr::gui
{
// intrinsic size
float RenderProxyBox::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return child() ? child()->get_min_intrinsic_width(height) : 0.0f;
}
float RenderProxyBox::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return child() ? child()->get_max_intrinsic_width(height) : 0.0f;
}
float RenderProxyBox::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return child() ? child()->get_min_intrinsic_height(width) : 0.0f;
}
float RenderProxyBox::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return child() ? child()->get_max_intrinsic_height(width) : 0.0f;
}

// dry layout
Size RenderProxyBox::compute_dry_layout(BoxConstraint constraints) const SKR_NOEXCEPT
{
    return child() ? child()->get_dry_layout(constraints) : constraints.smallest();
}

// layout
void RenderProxyBox::perform_layout() SKR_NOEXCEPT
{
    if (child())
    {
        child()->set_constraints(constraints());
        child()->layout(true);
        set_size(child()->size());
    }
    else
    {
        set_size(constraints().smallest());
    }
}

// paint
void RenderProxyBox::paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT
{
    if (child())
    {
        context->paint_child(make_not_null(child()), offset);
    }
}

//==> MIXIN: single child render object
SKR_GUI_TYPE_ID RenderProxyBox::accept_child_type() const noexcept { return SKR_GUI_TYPE_ID_OF_STATIC(RenderBox); }
void            RenderProxyBox::set_child(RenderObject* child) noexcept
{
    if (_child) drop_child(make_not_null(_child));
    _child = child->type_cast_fast<RenderBox>();
    if (_child) adopt_child(make_not_null(_child));
}
void RenderProxyBox::flush_depth() noexcept
{
    RenderBox::flush_depth();
    if (_child) _child->flush_depth();
}
void RenderProxyBox::visit_children(function_ref<void(RenderObject*)> visitor) const noexcept
{
    if (_child) visitor(_child);
}
void RenderProxyBox::visit_children_recursive(function_ref<void(RenderObject*)> visitor) const noexcept
{
    if (_child) _child->visit_children_recursive(visitor);
}
void RenderProxyBox::attach(NotNull<PipelineOwner*> owner) noexcept
{
    RenderBox::attach(owner);
    if (_child) _child->attach(owner);
}
void RenderProxyBox::detach() noexcept
{
    if (_child) _child->detach();
    RenderBox::detach();
}
//==> MIXIN: single child render object
} // namespace skr::gui