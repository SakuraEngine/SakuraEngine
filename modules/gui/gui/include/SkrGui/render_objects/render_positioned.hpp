#pragma once
#include "SkrGui/framework/render_object/render_shifted_box.hpp"

namespace skr::gui
{
struct RenderPositioned : public RenderShiftedBox {
    SKR_GUI_OBJECT(RenderPositioned, "17953364-abcc-4430-ac0c-7242c11dce92", RenderShiftedBox)
    using Super = RenderShiftedBox;

    // getter & setter
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
    Sizef compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT override;

    // layout
    void perform_layout() SKR_NOEXCEPT override;

private:
    inline bool shrink_wrap_width() const SKR_NOEXCEPT { return _width_factor || !constraints().has_bounded_width(); }
    inline bool shrink_wrap_height() const SKR_NOEXCEPT { return _height_factor || !constraints().has_bounded_height(); }

private:
    Optional<float> _width_factor  = {}; // used in anchor mode
    Optional<float> _height_factor = {}; // used in anchor mode
    Positional      _positional    = {};
};
} // namespace skr::gui