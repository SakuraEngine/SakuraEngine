#include "SkrGui/render_objects/render_constrained_box.hpp"

namespace skr::gui
{
// intrinsic size
float RenderConstrainedBox::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    if (additional_constraint().has_bounded_width() && additional_constraint().has_tight_width())
    {
        return _additional_constraint.min_width;
    }

    const float width = Super::compute_min_intrinsic_width(height);
    return !additional_constraint().has_infinite_width() ? additional_constraint().constrain_width(width) : width;
}
float RenderConstrainedBox::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    if (additional_constraint().has_bounded_width() && additional_constraint().has_tight_width())
    {
        return _additional_constraint.min_width;
    }

    const float width = Super::compute_max_intrinsic_width(height);
    return !additional_constraint().has_infinite_width() ? additional_constraint().constrain_width(width) : width;
}
float RenderConstrainedBox::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    if (additional_constraint().has_bounded_height() && additional_constraint().has_tight_height())
    {
        return _additional_constraint.min_height;
    }

    const float height = Super::compute_min_intrinsic_height(width);
    return !additional_constraint().has_infinite_height() ? additional_constraint().constrain_height(height) : height;
}
float RenderConstrainedBox::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    if (additional_constraint().has_bounded_height() && additional_constraint().has_tight_height())
    {
        return _additional_constraint.min_height;
    }

    const float height = Super::compute_max_intrinsic_height(width);
    return !additional_constraint().has_infinite_height() ? additional_constraint().constrain_height(height) : height;
}

// dry layout
Sizef RenderConstrainedBox::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    if (child())
    {
        return child()->get_dry_layout(additional_constraint().enforce(constraints));
    }
    else
    {
        return additional_constraint().enforce(constraints).constrain(Sizef::Zero());
    }
}

// layout
void RenderConstrainedBox::perform_layout() SKR_NOEXCEPT
{
    if (child())
    {
        child()->set_constraints(additional_constraint().enforce(constraints()));
        child()->layout(true);
        set_size(child()->size());
    }
    else
    {
        set_size(additional_constraint().enforce(constraints()).constrain(Sizef::Zero()));
    }
}
} // namespace skr::gui