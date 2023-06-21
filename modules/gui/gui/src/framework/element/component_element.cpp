#include "SkrGui/framework/element/component_element.hpp"

namespace skr::gui
{
// element tree
void ComponentElement::flush_depth() SKR_NOEXCEPT
{
    Super::flush_depth();
    if (_child) _child->flush_depth();
}
void ComponentElement::visit_children(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT
{
    if (_child) visitor(_child);
}
void ComponentElement::visit_children_recursive(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT
{
    if (_child)
    {
        visitor(_child);
        _child->visit_children_recursive(visitor);
    }
}

// life circle
void ComponentElement::mount(Element* parent, uint64_t slot) SKR_NOEXCEPT
{
    Super::mount(parent, slot);
    rebuild();
}
void ComponentElement::activate() SKR_NOEXCEPT
{
    Super::activate();
    if (_child) _child->activate();
}
void ComponentElement::deactivate() SKR_NOEXCEPT
{
    Super::deactivate();
    if (_child) _child->activate();
}

// build & update
void ComponentElement::perform_rebuild() SKR_NOEXCEPT
{
    Widget* widget = build();
    _cancel_dirty();
    _child = _update_child(_child, widget, slot());
}
void ComponentElement::update_slot(uint64_t new_slot) SKR_NOEXCEPT
{
    Super::update_slot(new_slot);
    if (_child) _child->update_slot(slot());
}

// render object (self or child's)
RenderObject* ComponentElement::render_object() const SKR_NOEXCEPT
{
    return _child ? _child->render_object() : nullptr;
}
} // namespace skr::gui