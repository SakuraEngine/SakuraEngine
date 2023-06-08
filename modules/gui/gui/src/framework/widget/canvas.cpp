#include "SkrGui/widgets/canvas.hpp"

namespace skr::gui
{
void Canvas::construct(Params params) SKR_NOEXCEPT
{
    auto& children = _children.get();
    children.assign(params);
}

} // namespace skr::gui