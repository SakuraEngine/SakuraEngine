#include "platform/debug.h"
#include "platform/memory.h"
#include "SkrGui/interface/gdi_renderer.hpp"
#include "private/gdi_private.hpp"

#include "nanovg/gdi_nanovg.hpp"

namespace skr {
namespace gdi {

void SGDIRenderGroupPrivate::add_element(SGDIElement* element) SKR_NOEXCEPT
{
    all_elements_.emplace_back(element);
}

void SGDIRenderGroupPrivate::remove_element(SGDIElement* element) SKR_NOEXCEPT
{
    auto it = eastl::find(all_elements_.begin(), all_elements_.end(), element);
    if (it != all_elements_.end()) 
    {
        all_elements_.erase(it);
    }
}

LiteSpan<SGDIElement*> SGDIRenderGroupPrivate::all_elements() SKR_NOEXCEPT
{
    return { all_elements_.data(), all_elements_.size() };
}

void SGDICanvasPrivate::add_render_group(SGDIRenderGroup* group) SKR_NOEXCEPT
{
    all_render_groups_.emplace_back(group);
}

void SGDICanvasPrivate::remove_render_group(SGDIRenderGroup* group) SKR_NOEXCEPT
{
    auto it = eastl::find(all_render_groups_.begin(), all_render_groups_.end(), group);
    if (it != all_render_groups_.end()) 
    {
        all_render_groups_.erase(it);
    }
}

LiteSpan<SGDIRenderGroup*> SGDICanvasPrivate::all_render_groups() SKR_NOEXCEPT
{
    return { all_render_groups_.data(), all_render_groups_.size() };
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

SGDIRenderGroup* SGDIDevice::create_render_group()
{
    return SkrNew<SGDIRenderGroupPrivate>();
}

void SGDIDevice::free_render_group(SGDIRenderGroup* render_group)
{
    SkrDelete(render_group);
}

LiteSpan<SGDIVertex> IGDIRenderer::fetch_element_vertices(SGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<SGDIElementPrivate*>(element);
    return { element_private->vertices.data(), element_private->vertices.size() };
}

LiteSpan<index_t> IGDIRenderer::fetch_element_indices(SGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<SGDIElementPrivate*>(element);
    return { element_private->indices.data(), element_private->indices.size() };
}

LiteSpan<SGDIElementDrawCommand> IGDIRenderer::fetch_element_draw_commands(SGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<SGDIElementPrivate*>(element);
    return { element_private->commands.data(), element_private->commands.size() };
}

} }