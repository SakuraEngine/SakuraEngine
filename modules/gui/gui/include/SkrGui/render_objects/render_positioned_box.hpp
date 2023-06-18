#pragma once
#include "SkrGui/framework/render_object/render_shifted_box.hpp"

namespace skr::gui
{
struct RenderPositionedBox : public RenderShiftedBox {
    SKR_GUI_TYPE(RenderPositionedBox, "17953364-abcc-4430-ac0c-7242c11dce92", RenderShiftedBox)

    // getter & setter
    inline bool pass_constraints() const SKR_NOEXCEPT { return _pass_constraints; }
    inline void set_pass_constraints(bool value) SKR_NOEXCEPT
    {
        if (_pass_constraints != value)
        {
            _pass_constraints = value;
            mark_needs_layout();
        }
    }
    inline float width_factor() const SKR_NOEXCEPT { return _width_factor; }
    inline void  set_width_factor(float value) SKR_NOEXCEPT
    {
        if (_width_factor != value)
        {
            _width_factor = value;
            mark_needs_layout();
        }
    }
    inline float height_factor() const SKR_NOEXCEPT { return _height_factor; }
    inline void  set_height_factor(float value) SKR_NOEXCEPT
    {
        if (_height_factor != value)
        {
            _height_factor = value;
            mark_needs_layout();
        }
    }
    inline const Positional& positional() const SKR_NOEXCEPT { return _positional; }
    inline void              set_positional(const Positional& positional) SKR_NOEXCEPT
    {
        if (_positional != positional)
        {
            _positional = positional;
            mark_needs_layout();
        }
    }

    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // dry layout
    Size compute_dry_layout(BoxConstraint constraints) const SKR_NOEXCEPT override;

    // layout
    void perform_layout() SKR_NOEXCEPT override;

private:
    bool       _pass_constraints;
    float      _width_factor;
    float      _height_factor;
    Positional _positional;
};
} // namespace skr::gui