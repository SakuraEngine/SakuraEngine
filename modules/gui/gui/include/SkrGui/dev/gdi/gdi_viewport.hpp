#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gdi
{
struct SKR_GUI_API IGDIViewport {
    virtual ~IGDIViewport() SKR_NOEXCEPT = default;

    virtual void              add_canvas(IGDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual void              remove_canvas(IGDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual void              clear_canvas() SKR_NOEXCEPT = 0;
    virtual Span<IGDICanvas*> all_canvas() SKR_NOEXCEPT = 0;
};
} // namespace skr::gdi