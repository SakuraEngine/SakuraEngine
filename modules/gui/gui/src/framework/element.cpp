#include "SkrGui/framework/element.hpp"
#include "SkrGui/framework/render_object.hpp"
#include "SkrGui/framework/render_box.hpp"
#include "SkrGui/framework/widget.hpp"
#include "SkrGui/framework/build_owner.hpp"

namespace skr {
namespace gui {
// implement element interface

void Element::activate() SKR_NOEXCEPT
{

}

void Element::deactivate() SKR_NOEXCEPT
{

}

void Element::mount(Element* parent, Slot* slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::initial);
    SKR_ASSERT(_parent == nullptr);
    SKR_ASSERT(!parent || parent->_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(_slot == nullptr);
    _parent = parent;
    _slot = slot;
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

void Element::attach_render_object(Slot* new_slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_slot == nullptr);

    visit_child_elements([new_slot](Element* child) {
        child->attach_render_object(new_slot);
    });
    _slot = new_slot;
}

void Element::detach_render_object() SKR_NOEXCEPT
{
    visit_child_elements([](Element* child) {
        child->detach_render_object();
    });
    _slot = nullptr;
}

not_null<Element*> Element::inflate_widget(not_null<Widget*> widget, Slot* new_slot) SKR_NOEXCEPT
{
    //TODO: global key
    if(widget->key.is_keep_state())
    {
        Element* newChild = _retake_inactive_element(widget->key, widget);
        if(newChild)
        {
            SKR_ASSERT(newChild->_parent == nullptr);
            newChild->_active_with_parent(this, new_slot);
            Element* updatedChild = update_child(newChild, widget, new_slot);
            SKR_ASSERT(updatedChild == newChild);
            return make_not_null(newChild);
        }
    }
    auto element = widget->create_element();
    element->mount(this, new_slot);
    return element;
}

void Element::update(Widget* new_widget) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(_widget != new_widget);
    SKR_ASSERT(new_widget != nullptr);

    _widget = new_widget;
}

void Element::update_slot_for_child(Element* child, Slot* new_slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(child->_parent == this);

    child->_update_slot(new_slot);
    child->visit_child_elements([child, new_slot](Element* cc)
    {
        child->update_slot_for_child(cc, new_slot);
    });
}

Element* Element::update_child(Element* child, Widget* new_widget, Slot* new_slot) SKR_NOEXCEPT
{
    if(!new_widget)
    {
        if(child)
            deactivate_child(child);
        return nullptr;
    }
    auto widget = make_not_null(new_widget);
    Element* newChild = nullptr;
    if(child)
    {
        bool hasSameSuperclass = true;
        //TODO
        if(hasSameSuperclass && child->_widget == new_widget)
        {
            if(child->_slot != new_slot)
            {
                update_slot_for_child(child, new_slot);
            }
            newChild = child;
        }
        else if(hasSameSuperclass && Widget::CanUpdate(make_not_null(child->_widget), widget))
        {
            if(child->_slot != new_slot)
            {
                update_slot_for_child(child, new_slot);
            }
            child->update(new_widget);
            SKR_ASSERT(child->_widget == new_widget);
            newChild = child;
        }
        else
        {
            deactivate_child(child);
            SKR_ASSERT(child->_parent == nullptr);
            newChild = inflate_widget(widget, new_slot);
        }
    }
    return newChild;
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

void Element::deactivate_child(Element* child) SKR_NOEXCEPT
{
    SKR_ASSERT(child->_parent == this);
    child->_parent = nullptr;
    child->detach_render_object();
    _owner->_inactive_elements->push_back(child); // this eventually calls child.deactivate()
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

void Element::_update_slot(Slot* new_slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::active);
    SKR_ASSERT(_parent != nullptr);
    SKR_ASSERT(_parent->_lifecycle_state == ElementLifecycle::active);

    _slot = new_slot;
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
    if (render_object)
    {
        if (auto rbox = render_object->Cast<RenderBox>())
        {
            return rbox->get_size();;
        }
    }
    return { 0, 0 };
}

RenderObject* Element::find_render_object() SKR_NOEXCEPT
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

Element* Element::_retake_inactive_element(Key& key, not_null<Widget*> widget) SKR_NOEXCEPT
{
    auto iter = _owner->_global_key_registry->find(key.get_state());
    if (iter == _owner->_global_key_registry->end())
    {
        return nullptr;
    }
    Element* element = iter->second;
    if(!Widget::CanUpdate(make_not_null(element->_widget), widget))
    {
        return nullptr;
    }
    Element* parent = element->_parent;
    if (parent != nullptr)
    {
        SKR_ASSERT(parent != this);
        parent->forget_child(element);
        parent->deactivate_child(element);
    }
    SKR_ASSERT(element->_parent == nullptr);
    auto& inactiveElements = *_owner->_inactive_elements;
    inactiveElements.erase(std::remove(inactiveElements.begin(), inactiveElements.end(), element), inactiveElements.end());
    return element;
}

void Element::_active_with_parent(Element* parent, Slot* slot) SKR_NOEXCEPT
{
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::inactive);
    _parent = parent;
    _update_depth(_parent->_depth);
    _active_recursively(this);
    attach_render_object(slot);
    SKR_ASSERT(_lifecycle_state == ElementLifecycle::active);
}

void Element::_active_recursively(Element* element) SKR_NOEXCEPT
{
    SKR_ASSERT(element->_lifecycle_state == ElementLifecycle::inactive);
    element->activate();
    SKR_ASSERT(element->_lifecycle_state == ElementLifecycle::active);
    element->visit_child_elements(_active_recursively);
}

void Element::_update_depth(int parentDepth) SKR_NOEXCEPT
{
    int expectedDepth = parentDepth + 1;
    if(_depth < expectedDepth)
    {
        _depth = expectedDepth;
        visit_child_elements([expectedDepth](Element* child)
        {
            child->_update_depth(expectedDepth);
        });
    }
}

int Element::_compare_depth(Element* a, Element* b) SKR_NOEXCEPT
{
    if(a->_depth < b->_depth)
    {
        return -1;
    }
    else if(a->_depth > b->_depth)
    {
        return 1;
    }
    else
    {
        // If the `dirty` values are not equal, sort with non-dirty elements being
        // less than dirty elements.
        if(a->_dirty != b->_dirty)
        {
            return a->_dirty ? 1 : -1;
        }
        else
        {
            return 0;
        }
    }
}

bool Element::_debug_is_in_scope(Element* ancestor) SKR_NOEXCEPT
{
    Element* current = this;
    while (current != nullptr)
    {
        if (current == ancestor)
        {
            return true;
        }
        current = current->_parent;
    }
    return false;
}
} // namespace gui
} // namespace skr