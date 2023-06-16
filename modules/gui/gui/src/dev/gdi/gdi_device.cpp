#include "dev/gdi/private/gdi_device.hpp"
#include "dev/gdi/private/gdi_element.hpp"
#include "dev/gdi/private/gdi_viewport.hpp"
#include "dev/gdi/private/gdi_canvas.hpp"
#include "dev/gdi/nanovg/gdi_device.hpp"

namespace skr::gdi
{
IGDIDevice* IGDIDevice::Create(EGDIBackend backend)
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

void IGDIDevice::Free(IGDIDevice* device)
{
    SkrDelete(device);
}

IGDIViewport* IGDIDevice::create_viewport()
{
    return SkrNew<GDIViewportPrivate>();
}

void IGDIDevice::free_viewport(IGDIViewport* canvas)
{
    SkrDelete(canvas);
}

IGDICanvas* IGDIDevice::create_canvas()
{
    return SkrNew<GDICanvasPrivate>();
}

void IGDIDevice::free_canvas(IGDICanvas* render_group)
{
    SkrDelete(render_group);
}
} // namespace skr::gdi