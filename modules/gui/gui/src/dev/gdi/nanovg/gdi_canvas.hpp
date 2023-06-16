#pragma once
#include <nanovg.h>
#include "dev/gdi/private/gdi_canvas.hpp"

namespace skr::gdi
{
struct GDICanvasNVG : public GDICanvasPrivate {
    void add_element(IGDIElement* element) SKR_NOEXCEPT final { GDICanvasPrivate::add_element(element); }
};
} // namespace skr::gdi