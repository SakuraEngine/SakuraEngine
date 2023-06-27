#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct ICanvas;

struct SKR_GUI_API ICanvasService {
    virtual ~ICanvasService() = default;

    NotNull<ICanvas*> create_canvas();
    void              destroy_canvas(NotNull<ICanvas*> canvas);
};
} // namespace skr::gui