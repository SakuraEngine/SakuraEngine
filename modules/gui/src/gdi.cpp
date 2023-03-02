#include "platform/debug.h"
#include "platform/memory.h"
#include "SkrGui/gdi.h"

#include "nanovg/gdi_nanovg.hpp"

namespace skr {
namespace gdi {

void SGDICanvas::add_element(SGDIElement* element, const skr_float4x4_t& transform)
{

}

SGDIDevice* SGDIDevice::Create(EGDIBackend backend)
{
    switch (backend) {
    case EGDIBackend::NANOVG:
        return SkrNew<SGDIDeviceNVG>();
    default:
        SKR_UNREACHABLE_CODE();
        return nullptr;
    }
}
    
void SGDIDevice::Free(SGDIDevice* device)
{
    SkrDelete(device);
}

SGDICanvas* SGDIDevice::create_canvas()
{
    return SkrNew<SGDICanvas>();
}

void SGDIDevice::free_canvas(SGDICanvas* canvas)
{
    SkrDelete(canvas);
}

} }