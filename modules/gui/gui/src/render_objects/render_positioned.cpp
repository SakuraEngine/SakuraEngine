#include "SkrGui/render_objects/render_positioned.hpp"

namespace skr::gui
{
// intrinsic size
float RenderPositioned::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    if (positional().is_width_padding())
    {
        const float width_padding  = positional().resolve_padding_width(0);
        const float height_padding = positional().resolve_padding_width(height);

        return child() ? child()->get_min_intrinsic_width(std::max(0.f, height - height_padding)) + width_padding : width_padding;
    }
    else
    {
        return Super::compute_min_intrinsic_width(height);
    }
}
float RenderPositioned::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    if (positional().is_width_padding())
    {
        const float width_padding  = positional().resolve_padding_width(0);
        const float height_padding = positional().resolve_padding_width(height);

        return child() ? child()->get_max_intrinsic_width(std::max(0.f, height - height_padding)) + width_padding : width_padding;
    }
    else
    {
        return Super::compute_max_intrinsic_width(height);
    }
}
float RenderPositioned::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    if (positional().is_height_padding())
    {
        const float width_padding  = positional().resolve_padding_width(width);
        const float height_padding = positional().resolve_padding_width(0);

        return child() ? child()->get_min_intrinsic_height(std::max(0.f, width - width_padding)) + height_padding : height_padding;
    }
    else
    {
        return Super::compute_min_intrinsic_height(width);
    }
}
float RenderPositioned::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    if (positional().is_height_padding())
    {
        const float width_padding  = positional().resolve_padding_width(width);
        const float height_padding = positional().resolve_padding_width(0);

        return child() ? child()->get_max_intrinsic_height(std::max(0.f, width - width_padding)) + height_padding : height_padding;
    }
    else
    {
        return Super::compute_max_intrinsic_height(width);
    }
}

// dry layout
Sizef RenderPositioned::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    if (child())
    {
        BoxConstraints child_constraints = positional().resolve_constraints(constraints);
        Sizef          child_size        = child()->get_dry_layout(child_constraints);
        Sizef          parent_size;

        // calc horizontal
        if (positional().is_width_padding())
        {
            parent_size.width = (positional().left + positional().right).inflate(child_size.width);
        }
        else
        {
            parent_size.width = shrink_wrap_width() ? child_size.width * _width_factor.value_or(1) : std::numeric_limits<float>::infinity();
        }

        // calc vertical
        if (positional().is_height_padding())
        {
            parent_size.height = (positional().top + positional().bottom).inflate(child_size.height);
        }
        else
        {
            parent_size.height = shrink_wrap_height() ? child_size.height * _height_factor.value_or(1) : std::numeric_limits<float>::infinity();
        }

        return constraints.constrain(parent_size);
    }
    else
    {
        float width, height;

        // calc horizontal
        if (positional().is_width_padding())
        {
            float left_padding  = positional().left.resolve(constraints.min_width);
            float right_padding = positional().right.resolve(constraints.min_width);

            width = left_padding + right_padding;
        }
        else
        {
            width = shrink_wrap_width() ? 0.0 : std::numeric_limits<float>::infinity();
        }

        // calc vertical
        if (positional().is_height_padding())
        {
            float top_padding    = positional().top.resolve(constraints.min_height);
            float bottom_padding = positional().bottom.resolve(constraints.min_height);

            height = top_padding + bottom_padding;
        }
        else
        {
            height = shrink_wrap_height() ? 0.0 : std::numeric_limits<float>::infinity();
        }

        return constraints.constrain({ width, height });
    }
}

// layout
void RenderPositioned::perform_layout() SKR_NOEXCEPT
{
    if (child())
    {
        // layout child
        child()->set_constraints(positional().resolve_constraints(constraints()));
        child()->layout(true);

        Sizef child_size = child()->size();
        Sizef parent_size;

        // calc horizontal
        if (positional().is_width_padding())
        {
            parent_size.width = (positional().left + positional().right).inflate(child_size.width);
        }
        else
        {
            parent_size.width = shrink_wrap_width() ? child_size.width * _width_factor.value_or(1) : std::numeric_limits<float>::infinity();
        }

        // calc vertical
        if (positional().is_height_padding())
        {
            parent_size.height = (positional().top + positional().bottom).inflate(child_size.height);
        }
        else
        {
            parent_size.height = shrink_wrap_height() ? child_size.height * _height_factor.value_or(1) : std::numeric_limits<float>::infinity();
        }

        parent_size = constraints().constrain(parent_size);
        set_size(parent_size);
        set_offset(positional().resolve_offset(child_size, parent_size));
    }
    else
    {
        float width, height, offset_x, offset_y;

        // calc horizontal
        if (positional().is_width_padding())
        {
            float left_padding  = positional().left.resolve(constraints().min_width);
            float right_padding = positional().right.resolve(constraints().min_width);

            width    = left_padding + right_padding;
            offset_x = left_padding;
        }
        else
        {
            width    = shrink_wrap_width() ? 0.0 : std::numeric_limits<float>::infinity();
            offset_x = 0.0;
        }

        // calc vertical
        if (positional().is_height_padding())
        {
            float top_padding    = positional().top.resolve(constraints().min_height);
            float bottom_padding = positional().bottom.resolve(constraints().min_height);

            height   = top_padding + bottom_padding;
            offset_y = top_padding;
        }
        else
        {
            height   = shrink_wrap_height() ? 0.0 : std::numeric_limits<float>::infinity();
            offset_y = 0.0;
        }

        set_size(constraints().constrain({ width, height }));
        set_offset({ offset_x, offset_y });
    }
}

} // namespace skr::gui