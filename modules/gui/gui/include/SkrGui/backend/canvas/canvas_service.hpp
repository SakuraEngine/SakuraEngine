#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct ICanvas;

struct SKR_GUI_API ICanvasService SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(ICanvasService, "27d97b13-bf0a-4f0e-9ac3-5fb0d6c96755")
    virtual ~ICanvasService() = default;

    virtual NotNull<ICanvas*> create_canvas() = 0;
    virtual void              destroy_canvas(NotNull<ICanvas*> canvas) = 0;
};
} // namespace skr::gui