#include "SkrGui/widgets/flex.hpp"

namespace skr::gui
{
void Flex::construct(const Params& params) SKR_NOEXCEPT
{
    // copy params
    _justify_content = params.justify_content;
    _flex_direction = params.flex_direction;
    _align_items = params.align_items;

    // copy slots
    _children.reserve(params.children.size());
    _children_slots.reserve(params.children.size());
    for (const Slot& slot : params.children)
    {
        _children_slots.push_back({ slot.flex, slot.flex_fit, slot.child });
        _children.push_back(slot.child);
    }
}
} // namespace skr::gui