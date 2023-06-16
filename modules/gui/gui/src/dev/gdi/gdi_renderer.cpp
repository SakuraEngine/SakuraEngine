#include "SkrGui/dev/gdi/gdi_renderer.hpp"
#include "dev/gdi/private/gdi_element.hpp"
#include "dev/gdi/private/gdi_viewport.hpp"
#include "dev/gdi/private/gdi_canvas.hpp"

namespace skr::gdi
{
Span<GDIVertex> IGDIRenderer::fetch_element_vertices(IGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<GDIElementPrivate*>(element);
    return { element_private->vertices.data(), element_private->vertices.size() };
}

Span<GDIIndex> IGDIRenderer::fetch_element_indices(IGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<GDIElementPrivate*>(element);
    return { element_private->indices.data(), element_private->indices.size() };
}

Span<GDIElementDrawCommand> IGDIRenderer::fetch_element_draw_commands(IGDIElement* element) SKR_NOEXCEPT
{
    const auto element_private = static_cast<GDIElementPrivate*>(element);
    return { element_private->commands.data(), element_private->commands.size() };
}
} // namespace skr::gdi