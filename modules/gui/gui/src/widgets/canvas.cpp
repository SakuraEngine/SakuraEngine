#include "SkrGui/widgets/canvas.hpp"

namespace skr::gui
{
void Canvas::construct(Params params) SKR_NOEXCEPT
{
    _children.reserve(params.children.size());
    _children_slots.reserve(params.children.size());
    for (const Slot& slot : params.children)
    {
        _children_slots.push_back({ slot.layout, slot.z_index, slot.child });
        _children.push_back(slot.child);
    }
}

} // namespace skr::gui