#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct ICanvas;

struct SKR_GUI_API IRenderBackend {
    virtual ~IRenderBackend() = default;

    // canvas，或许应该交给 INativeWindow 来创建
    virtual ICanvas* create_canvas() = 0;
    virtual void     destroy_canvas(ICanvas* canvas) = 0;

    // draw
    // TODO. draw layer
    virtual void _temp_draw_canvas(ICanvas* canvas) = 0;
};
} // namespace skr::gui