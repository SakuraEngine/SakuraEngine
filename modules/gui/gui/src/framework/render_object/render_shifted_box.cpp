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
void RenderShiftedBox::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    if (child())
    {
        context->paint_child(make_not_null(child()), this->offset() + offset);
    }
}

// hit test
bool RenderShiftedBox::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    return _default_hit_test(
    result,
    local_position,
    nullptr, // shifted box 通常不会自主响应 hit test
    [this](HitTestResult* result, Offsetf local_position) {
        return child() &&
               result->add_with_paint_offset(
               _offset,
               local_position,
               [this](HitTestResult* result, Offsetf transformed_position) {
                   return child()->hit_test(result, transformed_position);
               });
    });
}

} // namespace skr::gui
