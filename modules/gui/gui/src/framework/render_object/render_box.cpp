#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/system/input/hit_test.hpp"

namespace skr::gui
{
RenderBox::RenderBox() {}
RenderBox::~RenderBox() {}

// intrinsic size
float RenderBox::get_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_min_intrinsic_width(height);
}
float RenderBox::get_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_max_intrinsic_width(height);
}
float RenderBox::get_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_min_intrinsic_height(width);
}
float RenderBox::get_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_max_intrinsic_height(width);
}

// dry layout
Sizef RenderBox::get_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    // TODO. cache
    return compute_dry_layout(constraints);
}

// intrinsic size
float RenderBox::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return 0.0f;
}
float RenderBox::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return 0.0f;
}
float RenderBox::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return 0.0f;
}
float RenderBox::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return 0.0f;
}

// hit test
bool RenderBox::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    return false;
}

// dry layout
Sizef RenderBox::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    return Sizef::Zero();
}

void RenderBox::perform_resize() SKR_NOEXCEPT
{
    set_size(compute_dry_layout(constraints()));
    if (!size().is_finite()) { SKR_GUI_LOG_ERROR(u8"Box that [is_sized_by_parent() == true] must return finite size"); }
}

} // namespace skr::gui