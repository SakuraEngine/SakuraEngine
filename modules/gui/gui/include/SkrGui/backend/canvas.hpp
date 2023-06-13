#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/paint_params.hpp"

namespace skr::gui
{
struct SKR_GUI_API ICanvas SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(ICanvas, "03c50c32-e83f-4d47-b87a-6d692f32b739")
    virtual ~ICanvas() = default;

    //==> layer & draw
    virtual void begin_paint() = 0;
    virtual void end_paint() = 0;
    virtual void save() = 0;
    virtual void restore() = 0;

    //==> path
    virtual void begin_path() = 0;
    virtual void end_path(const PaintParams& params) = 0;

    //==> custom path
    // move to
    // line to
    // quadratic bezier to
    // cubic bezier to
    // arc to

    //==> simple shape path
    // arc
    // rect
    // round rect
    // circle
    // ellipse

    //==> transform (state)
    // translate
    // rotate
    // scale
    // skew
};
} // namespace skr::gui