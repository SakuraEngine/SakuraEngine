#include "platform/debug.h"
#include "platform/memory.h"
#include "SkrGui/gdi.h"

#include "nanovg/gdi_nanovg.hpp"

namespace skr {
namespace gdi {

void SGDICanvas::add_element(SGDIElement* element, const skr_float4_t& transform)
{
    all_elements_.emplace_back(element);
}

void SGDICanvas::remove_element(SGDIElement* element)
{
    auto it = eastl::find(all_elements_.begin(), all_elements_.end(), element);
    if (it != all_elements_.end()) 
    {
        all_elements_.erase(it);
    }
}

skr::span<SGDIElement*> SGDICanvas::all_elements()
{
    return all_elements_;
}

void SGDICanvasGroup::add_canvas(SGDICanvas* canvas)
{
    all_canvas_.emplace_back(canvas);
}

void SGDICanvasGroup::remove_canvas(SGDICanvas* canvas)
{
    auto it = eastl::find(all_canvas_.begin(), all_canvas_.end(), canvas);
    if (it != all_canvas_.end()) 
    {
        all_canvas_.erase(it);
    }
}

skr::span<SGDICanvas*> SGDICanvasGroup::all_canvas()
{
    return all_canvas_;
}

SGDIDevice* SGDIDevice::Create(CGPUDeviceId device, EGDIBackend backend)
{
    switch (backend) {
    case EGDIBackend::NANOVG:
    {
        auto nvgDevice =  SkrNew<SGDIDeviceNVG>();
        nvgDevice->initialize(device);
        return nvgDevice;
    }
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

SGDICanvasGroup* SGDIDevice::create_canvas_group()
{
    return SkrNew<SGDICanvasGroup>();
}

void SGDIDevice::free_canvas_group(SGDICanvasGroup* canvas_group)
{
    SkrDelete(canvas_group);
}

} }