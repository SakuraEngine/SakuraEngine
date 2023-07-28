#include "SkrGui/framework/element/component_element.hpp"

namespace skr::gui
{
// element tree
void ComponentElement::first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    Super::mount(parent, slot);
    rebuild();
}
void ComponentElement::visit_children(ComponentElement::VisitFuncRef visitor) const SKR_NOEXCEPT
{
    if (_child) visitor(make_not_null(_child));
}

// build & update
void ComponentElement::perform_rebuild() SKR_NOEXCEPT
{
    Widget* widget = build();
    _cancel_dirty();
    _child = _update_child(_child, widget, slot());
}
} // namespace skr::gui