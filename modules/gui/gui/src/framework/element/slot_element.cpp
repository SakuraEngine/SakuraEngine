#include "SkrGui/framework/element/slot_element.hpp"
#include "SkrGui/framework/widget/slot_widget.hpp"

namespace skr::gui
{

void SlotElement::updated(NotNull<ProxyWidget*> old_widget)
{
    auto slot_widget = widget()->type_cast<SlotWidget>();

    auto parent_render_object = find_ancestor_render_object();
    auto child_render_object  = find_render_object();

    if (parent_render_object && child_render_object)
    {
        slot_widget->apply_slot_data(parent_render_object, child_render_object);
    }
}

} // namespace skr::gui