#include "SkrGui/framework/element.hpp"
#include "SkrGui/framework/render_object.hpp"
#include "SkrGui/framework/render_box.hpp"

namespace skr {
namespace gui {
// implement element interface

void Element::activate() SKR_NOEXCEPT
{

}

void Element::deactivate() SKR_NOEXCEPT
{

}

void Element::mount(LiteOptional<Element*> parent, LiteOptional<Slot*> slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::initial);
    SKR_ASSERT(_parent == nullptr);
    SKR_ASSERT(!parent || parent.get()->_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(_slot == nullptr);
    _parent = parent.get();
    _slot = slot.get();
    _lifecycle_state = ElementLifecycle::active;
    _depth = _parent ? _parent->_depth + 1 : 0;
    // if (parent) _owner = parent.get()._owner;
}

void Element::unmount() SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::inactive);
    SKR_ASSERT(_widget != nullptr);
    // SKR_ASSERT(_owner != nullptr);

    _widget = nullptr;
    _lifecycle_state = ElementLifecycle::defunct;
}

void Element::attach_render_object(LiteOptional<Slot*> new_slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_slot == nullptr);

    visit_child_elements([new_slot](Element* child) {
        child->attach_render_object(new_slot);
    });
    _slot = new_slot.get();
}

void Element::detach_render_object() SKR_NOEXCEPT
{
    visit_child_elements([](Element* child) {
        child->detach_render_object();
    });
    _slot = nullptr;
}

Element* Element::inflate_widget(Widget* widget, LiteOptional<Slot*> new_slot) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}

void Element::update(Widget* new_widget) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(_widget != new_widget);
    SKR_ASSERT(new_widget != nullptr);

    _widget = new_widget;
}

void Element::update_slot_for_child(Element* child, LiteOptional<Slot*> new_slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(child->_parent == this);

    child->_update_slot(new_slot);
    child->visit_child_elements([child, new_slot](Element* cc)
    {
        child->update_slot_for_child(cc, new_slot);
    });
}

Element* Element::update_child(LiteOptional<Element*> child, LiteOptional<Widget*> new_widget, LiteOptional<Slot*> new_slot) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}

void Element::visit_children(skr::function_ref<void(Element*)> visitor) SKR_NOEXCEPT
{
    //...
}

void Element::visit_child_elements(skr::function_ref<void(Element*)> visitor) SKR_NOEXCEPT
{
    visit_children(visitor);
}

void Element::forget_child(Element* child) SKR_NOEXCEPT
{
    //...
}

void Element::perform_rebuild() SKR_NOEXCEPT
{
    _dirty = false;
}

void Element::rebuild(bool force) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state != ElementLifecycle::initial);
    if (_lifecycle_state != ElementLifecycle::active)
        return;
    if (!_dirty && !force)
        return;

    perform_rebuild();
}

void Element::_update_slot(LiteOptional<Slot*> new_slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(_parent != nullptr);
    SKR_ASSERT(_parent->_lifecycle_state == ElementLifecycle::active);

    _slot = new_slot.get();
}


// implement build context interfaces

bool Element::mounted() SKR_NOEXCEPT
{
    return _widget;
}

Widget* Element::get_widget() SKR_NOEXCEPT
{
    return _widget;
}

BoxSizeType Element::get_size() SKR_NOEXCEPT
{
    auto render_object = find_render_object();
    if (auto robject = render_object.get())
    {
        if (auto rbox = robject->Cast<RenderBox>())
        {
            return rbox->get_size();;
        }
    }
    return { 0, 0 };
}

LiteOptional<RenderObject*> Element::find_render_object() SKR_NOEXCEPT
{
    Element* current = this;
    while (current != nullptr) 
    {
        if (current->_lifecycle_state == ElementLifecycle::defunct) 
        {
            break;
        }
        else 
        {
            Element* next = nullptr;
            current->visit_child_elements(
                [&](Element* child) {
                assert(next == nullptr);  // This verifies that there's only one child.
                next = child;
            });
            current = next;
        }
    }
    return nullptr;
}

} // namespace gui
} // namespace skr