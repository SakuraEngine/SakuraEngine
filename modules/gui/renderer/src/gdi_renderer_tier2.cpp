#include "SkrGuiRenderer/gdi_renderer.hpp"
#include "math/rtm/qvvf.h"

namespace skr::gui
{

bool GDIRenderer_RenderGraph::support_mipmap_generation() const SKR_NOEXCEPT
{
    return false;
}

} // namespace skr::gui