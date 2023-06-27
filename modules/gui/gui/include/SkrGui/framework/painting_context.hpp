#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct PaintingContext final {
    inline ICanvas* canvas() const noexcept { return _canvas; }
    void            paint_child(NotNull<RenderObject*> child, Offsetf offset) noexcept;

private:
    ICanvas* _canvas;
    // layer
};
} // namespace skr::gui