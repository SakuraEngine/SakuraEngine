#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/build_owner.hpp"

namespace skr::gui
{
Element::Element(Widget* widget) SKR_NOEXCEPT
    : _widget(widget)
{
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
}

// element tree
void Element::flush_depth() SKR_NOEXCEPT
{
    _depth = _parent->_depth + 1;
}
void Element::visit_children(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT {}
void Element::visit_children_recursive(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT {}

// life circle
void Element::mount(Element* parent, uint64_t slot) SKR_NOEXCEPT
{
    // validate
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_parent != nullptr) { SKR_GUI_LOG_ERROR("already mounted"); }
    if (parent != nullptr && parent->_lifecycle_state != EElementLifecycle::Active) { SKR_GUI_LOG_ERROR("parent is not active"); }

    // copy value
    _parent = parent;
    _slot = slot;
    _lifecycle_state = EElementLifecycle::Active;
    _depth = _parent ? _parent->_depth + 1 : 0;
    _owner = _parent ? _parent->_owner : nullptr;

    // validate
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    // TODO. process global key
    // TODO. process dependencies
    // TODO. process notify
}
void Element::activate() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle_state != EElementLifecycle::Inactive) { SKR_GUI_LOG_ERROR("already active"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    _lifecycle_state = EElementLifecycle::Active;

    // TODO. process dependencies
    // TODO. process notify
}
void Element::deactivate() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle_state == EElementLifecycle::Active) { SKR_GUI_LOG_ERROR("already inactive"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }

    // TODO. process dependencies

    _lifecycle_state = EElementLifecycle::Inactive;
}
void Element::unmount() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle_state != EElementLifecycle::Active) { SKR_GUI_LOG_ERROR("already unmounted"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    // TODO. process global key
    _widget = nullptr;
    _lifecycle_state = EElementLifecycle::Defunct;
}

// mark functions
void Element::mark_needs_build() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle_state == EElementLifecycle::Defunct) { SKR_GUI_LOG_ERROR("already unmounted"); }
    if (_lifecycle_state != EElementLifecycle::Active) { return; }
    if (_owner == nullptr)
    {
        SKR_GUI_LOG_ERROR("owner is nullptr");
        return;
    }
    if (_dirty) { return; }

    // dirty
    _dirty = true;
    _owner->schedule_build_for(make_not_null(this));
}

// build & update
void Element::rebuild(bool force) SKR_NOEXCEPT
{
    // validate
    if (_lifecycle_state == EElementLifecycle::Initial) { SKR_GUI_LOG_ERROR("element is incomplete"); }
    if (_lifecycle_state != EElementLifecycle::Active || (!_dirty && !force)) { return; }

    perform_rebuild();

    // validate
    if (_dirty) { SKR_GUI_LOG_ERROR("perform_rebuild() must set dirty to false"); }
}
void Element::update_slot(uint64_t new_slot) SKR_NOEXCEPT
{
    // validate
    if (_lifecycle_state != EElementLifecycle::Active) { SKR_GUI_LOG_ERROR("element is not active"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_parent == nullptr) { SKR_GUI_LOG_ERROR("parent is nullptr"); }
    if (_parent && _parent->_lifecycle_state != EElementLifecycle::Active) { SKR_GUI_LOG_ERROR("parent is not active"); }

    _slot = new_slot;
}
void Element::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    // validate
    if (_lifecycle_state != EElementLifecycle::Active) { SKR_GUI_LOG_ERROR("element is not active"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (new_widget == _widget) { SKR_GUI_LOG_ERROR("new_widget is same as old widget"); }
    if (_widget && !Widget::can_update(make_not_null(_widget), new_widget)) { SKR_GUI_LOG_ERROR("can not update widget"); }

    _widget = new_widget;
}

// render object (self or child's)
RenderObject* Element::render_object() const SKR_NOEXCEPT { return nullptr; }

// help functions
Element* Element::_update_child(Element* child, Widget* new_widget, uint64_t new_slot) SKR_NOEXCEPT
{
    // validate
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    // new widget is nullptr, just deactivate child
    if (new_widget == nullptr)
    {
        if (child)
        {
            if (_owner) _owner->deactivate_element(make_not_null(child));
        }
        return nullptr;
    }

    if (child != nullptr)
    {
        if (child->_widget == new_widget)
        {
            if (child->_slot != new_slot) child->update_slot(new_slot);
            return child;
        }
        else if (Widget::can_update(make_not_null(child->_widget), make_not_null(new_widget)))
        {
            if (child->_slot != new_slot) child->update_slot(new_slot);
            child->update(make_not_null(new_widget));
            return child;
        }
        else
        {
            if (_owner) _owner->deactivate_element(make_not_null(child));
            if (child->_parent != nullptr) { SKR_LOG_ERROR("child's parent is not nullptr after deactivate"); }
            return _inflate_widget(make_not_null(new_widget), new_slot);
        }
    }
    else
    {
        return _inflate_widget(make_not_null(new_widget), new_slot);
    }
}
NotNull<Element*> Element::_inflate_widget(NotNull<Widget*> new_widget, uint64_t slot) SKR_NOEXCEPT
{
    // TODO. process global key

    Element* const new_child = new_widget->create_element();
    new_child->mount(this, slot);
    return make_not_null(new_child);
}

} // namespace skr::gui