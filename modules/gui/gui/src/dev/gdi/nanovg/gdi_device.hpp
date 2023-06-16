#pragma once
#include <nanovg.h>
#include "dev/gdi/private/gdi_device.hpp"

namespace skr::gdi
{
struct GDIDeviceNVG : public GDIDevicePrivate {
    ~GDIDeviceNVG();
    int initialize() SKR_NOEXCEPT;
    int finalize() SKR_NOEXCEPT;

    IGDICanvas* create_canvas() final;
    void        free_canvas(IGDICanvas* canvas) final;

    IGDIViewport* create_viewport() final;
    void          free_viewport(IGDIViewport* group) final;

    IGDIElement* create_element() final;
    void         free_element(IGDIElement* element) final;

    IGDIPaint* create_paint() final;
    void       free_paint(IGDIPaint* paint) final;
};
} // namespace skr::gdi