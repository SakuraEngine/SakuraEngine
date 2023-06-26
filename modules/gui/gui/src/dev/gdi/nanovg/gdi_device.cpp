#include "dev/gdi/nanovg/gdi_device.hpp"
#include "dev/gdi/nanovg/gdi_viewport.hpp"
#include "dev/gdi/nanovg/gdi_canvas.hpp"
#include "dev/gdi/nanovg/gdi_element.hpp"
#include "platform/memory.h"
#include "misc/make_zeroed.hpp"
#include "math/rtm/rtmx.h"

// nvg helper functions

// device code
namespace skr::gui
{
GDIDeviceNVG::~GDIDeviceNVG()
{
    finalize();
}
int GDIDeviceNVG::initialize() SKR_NOEXCEPT
{
    return 0;
}

int GDIDeviceNVG::finalize() SKR_NOEXCEPT
{
    return 0;
}

IGDIViewport* GDIDeviceNVG::create_viewport()
{
    return SkrNew<GDIViewportNVG>();
}

void GDIDeviceNVG::free_viewport(IGDIViewport* canvas)
{
    SkrDelete(canvas);
}

IGDICanvas* GDIDeviceNVG::create_canvas()
{
    return SkrNew<GDICanvasNVG>();
}

void GDIDeviceNVG::free_canvas(IGDICanvas* group)
{
    SkrDelete(group);
}

IGDIElement* GDIDeviceNVG::create_element()
{
    auto element = SkrNew<GDIElementNVG>();
    auto params = make_zeroed<NVGparams>();
    params.userPtr = element;
    params.edgeAntiAlias = true;
    element->nvg = nvgCreateInternal(&params);
    return element;
}

void GDIDeviceNVG::free_element(IGDIElement* element)
{
    SkrDelete(element);
}

IGDIPaint* GDIDeviceNVG::create_paint()
{
    return SkrNew<GDIPaintNVG>();
}

void GDIDeviceNVG::free_paint(IGDIPaint* paint)
{
    SkrDelete(paint);
}
} // namespace skr::gui