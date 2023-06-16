#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gdi
{
struct GDICanvas;

struct SKR_GUI_API GDIViewport {
    virtual ~GDIViewport() SKR_NOEXCEPT = default;

    virtual void             add_canvas(GDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual void             remove_canvas(GDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual void             clear_canvas() SKR_NOEXCEPT = 0;
    virtual Span<GDICanvas*> all_canvas() SKR_NOEXCEPT = 0;
};
} // namespace skr::gdi