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
Sizef RenderProxyBox::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
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
void RenderProxyBox::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    if (child())
    {
        context->paint_child(child(), offset);
    }
}

// hit test
bool RenderProxyBox::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    if (size().contains(local_position))
    {
        return child() && child()->hit_test(result, local_position);
    }
    return false;
}
} // namespace skr::gui