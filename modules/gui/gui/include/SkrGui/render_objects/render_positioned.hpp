#pragma once
#include "SkrGui/framework/render_object/render_shifted_box.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_positioned.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "50971da8-2439-46c1-adcb-579f270e354d",
    "rtti": true
)
RenderPositioned : public RenderShiftedBox
{
    SKR_RTTR_GENERATE_BODY()
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
    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    Optional<float> _width_factor  = {}; // used in anchor mode
    Optional<float> _height_factor = {}; // used in anchor mode
    Positional      _positional    = {};
};
} // namespace gui sreflect
} // namespace skr sreflect