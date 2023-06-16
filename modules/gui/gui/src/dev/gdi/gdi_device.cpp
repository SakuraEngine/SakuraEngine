#include "dev/gdi/private/gdi_device.hpp"
#include "dev/gdi/private/gdi_element.hpp"
#include "dev/gdi/private/gdi_viewport.hpp"
#include "dev/gdi/private/gdi_canvas.hpp"
#include "dev/gdi/nanovg/gdi_device.hpp"

namespace skr::gdi
{
GDIDevice* GDIDevice::Create(EGDIBackend backend)
{
    switch (backend)
    {
        case EGDIBackend::NANOVG: {
            auto nvgDevice = SkrNew<GDIDeviceNVG>();
            nvgDevice->initialize();
            return nvgDevice;
        }
        default:
            SKR_UNREACHABLE_CODE();
            return nullptr;
    }
}

void GDIDevice::Free(GDIDevice* device)
{
    SkrDelete(device);
}

GDIViewport* GDIDevice::create_viewport()
{
    return SkrNew<GDIViewportPrivate>();
}

void GDIDevice::free_viewport(GDIViewport* canvas)
{
    SkrDelete(canvas);
}

GDICanvas* GDIDevice::create_canvas()
{
    return SkrNew<GDICanvasPrivate>();
}

void GDIDevice::free_canvas(GDICanvas* render_group)
{
    SkrDelete(render_group);
}
} // namespace skr::gdi