#include "SkrGui/widgets/canvas.hpp"

namespace skr::gui
{
void Canvas::construct(Params params) SKR_NOEXCEPT
{
    _children->reserve(params.size());
    _children_slots->reserve(params.size());
    for (const Slot& slot : params)
    {
        _children_slots->push_back({
            .layout = slot.layout,
            .z_index = slot.z_index,
        });
        _children->push_back(slot.widget);
    }
}

} // namespace skr::gui