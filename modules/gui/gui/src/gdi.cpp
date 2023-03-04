#include "platform/debug.h"
#include "platform/memory.h"
#include "private/gdi_private.h"

#include "nanovg/gdi_nanovg.hpp"

namespace skr {
namespace gdi {

void SGDICanvasPrivate::add_element(SGDIElement* element, const skr_float4_t& transform) SKR_NOEXCEPT
{
    all_elements_.emplace_back(element);
}

void SGDICanvasPrivate::remove_element(SGDIElement* element) SKR_NOEXCEPT
{
    auto it = eastl::find(all_elements_.begin(), all_elements_.end(), element);
    if (it != all_elements_.end()) 
    {
        all_elements_.erase(it);
    }
}

LiteSpan<SGDIElement*> SGDICanvasPrivate::all_elements() SKR_NOEXCEPT
{
    return { all_elements_.data(), all_elements_.size() };
}

void SGDICanvasGroupPrivate::add_canvas(SGDICanvas* canvas) SKR_NOEXCEPT
{
    all_canvas_.emplace_back(canvas);
}

void SGDICanvasGroupPrivate::remove_canvas(SGDICanvas* canvas) SKR_NOEXCEPT
{
    auto it = eastl::find(all_canvas_.begin(), all_canvas_.end(), canvas);
    if (it != all_canvas_.end()) 
    {
        all_canvas_.erase(it);
    }
}

LiteSpan<SGDICanvas*> SGDICanvasGroupPrivate::all_canvas() SKR_NOEXCEPT
{
    return { all_canvas_.data(), all_canvas_.size() };
}

SGDIDevice* SGDIDevice::Create(EGDIBackend backend)
{
    switch (backend) {
    case EGDIBackend::NANOVG:
    {
        auto nvgDevice =  SkrNew<SGDIDeviceNVG>();
        nvgDevice->initialize();
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
    return SkrNew<SGDICanvasPrivate>();
}

void SGDIDevice::free_canvas(SGDICanvas* canvas)
{
    SkrDelete(canvas);
}

SGDICanvasGroup* SGDIDevice::create_canvas_group()
{
    return SkrNew<SGDICanvasGroupPrivate>();
}

void SGDIDevice::free_canvas_group(SGDICanvasGroup* canvas_group)
{
    SkrDelete(canvas_group);
}

LiteSpan<SGDIVertex> SGDIRenderer::fetch_element_vertices(SGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<SGDIElementPrivate*>(element);
    return { element_private->vertices.data(), element_private->vertices.size() };
}

LiteSpan<index_t> SGDIRenderer::fetch_element_indices(SGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<SGDIElementPrivate*>(element);
    return { element_private->indices.data(), element_private->indices.size() };
}

LiteSpan<SGDIElementDrawCommand> SGDIRenderer::fetch_element_draw_commands(SGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<SGDIElementPrivate*>(element);
    return { element_private->commands.data(), element_private->commands.size() };
}

} }