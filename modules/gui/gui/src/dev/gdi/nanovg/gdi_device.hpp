#pragma once
#include <nanovg.h>
#include "dev/gdi/private/gdi_device.hpp"

namespace skr::gdi
{
struct GDIDeviceNVG : public GDIDevicePrivate {
    ~GDIDeviceNVG();
    int initialize() SKR_NOEXCEPT;
    int finalize() SKR_NOEXCEPT;

    GDICanvas* create_canvas() final;
    void       free_canvas(GDICanvas* canvas) final;

    GDIViewport* create_viewport() final;
    void         free_viewport(GDIViewport* group) final;

    GDIElement* create_element() final;
    void        free_element(GDIElement* element) final;

    GDIPaint* create_paint() final;
    void      free_paint(GDIPaint* paint) final;
};
} // namespace skr::gdi