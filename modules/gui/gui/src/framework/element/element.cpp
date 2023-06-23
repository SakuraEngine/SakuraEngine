#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/element/render_object_element.hpp"
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

// lifecycle & tree
// ctor -> mount <-> unmount -> destroy
void Element::mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    // validate
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_parent != nullptr) { SKR_GUI_LOG_ERROR("already mounted"); }

    // mount
    _parent = parent;
    if (_lifecycle == EElementLifecycle::Initial)
    {
        first_mount(parent, slot);
    }
    if (_parent->_owner)
    {
        struct _RecursiveHelper {
            NotNull<BuildOwner*> owner;

            void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
            {
                obj->attach(owner);
                obj->visit_children(_RecursiveHelper{ owner });
            }
        };
        this->visit_children(_RecursiveHelper{ make_not_null(_parent->_owner) });
    }
    else
    {
        SKR_GUI_LOG_ERROR("owner is nullptr");
    }
    {
        struct _RecursiveHelper {
            void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
            {
                obj->_slot = Slot::Invalid();
                if (!obj->type_is<RenderObjectElement>())
                {
                    obj->visit_children(_RecursiveHelper{});
                }
            }
        };
    }
    _lifecycle = EElementLifecycle::Mounted;
}
void Element::unmount() SKR_NOEXCEPT
{
    // validate
    if (_parent == nullptr)
    {
        SKR_GUI_LOG_ERROR("already unmounted");
        return;
    }

    // unmount
    _parent = nullptr;
    if (_owner)
    {
        // TODO. detach render object
        _owner->drop_unmount_element(make_not_null(this));
        struct _RecursiveHelper {
            void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
            {
                obj->detach();
                obj->visit_children(_RecursiveHelper{});
            }
        };
        this->visit_children(_RecursiveHelper{});
    }
    _lifecycle = EElementLifecycle::Unmounted;
}
void Element::destroy() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle != EElementLifecycle::Unmounted) { SKR_GUI_LOG_ERROR("before destroy, must unmount"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    // TODO. process global key
    _widget = nullptr;
    // TODO. process dependencies
    _lifecycle = EElementLifecycle::Destroyed;
}
void Element::first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    _slot = slot;
    // TODO. process global key
    // TODO. process dependencies
    // TODO. process notify
}
void Element::attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT
{
    // validate
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    if (_lifecycle != EElementLifecycle::Initial)
    {
        // TODO. process dependencies
        // TODO. process notify
    }
}
void Element::detach() SKR_NOEXCEPT
{
    // validate
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    // TODO. process dependencies
}

// mark functions
void Element::mark_needs_build() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle == EElementLifecycle::Destroyed) { SKR_GUI_LOG_ERROR("already unmounted"); }
    if (_lifecycle != EElementLifecycle::Mounted) { return; }
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
    if (_lifecycle == EElementLifecycle::Initial) { SKR_GUI_LOG_ERROR("element is incomplete"); }
    if (_lifecycle != EElementLifecycle::Mounted || (!_dirty && !force))
    {
        return;
    }

    perform_rebuild();

    // validate
    if (_dirty) { SKR_GUI_LOG_ERROR("perform_rebuild() must set dirty to false"); }
}
void Element::update_slot(Slot new_slot) SKR_NOEXCEPT
{
    // validate
    if (_lifecycle != EElementLifecycle::Mounted) { SKR_GUI_LOG_ERROR("element is not active"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (_parent == nullptr) { SKR_GUI_LOG_ERROR("parent is nullptr"); }
    if (_parent && _parent->_lifecycle != EElementLifecycle::Mounted) { SKR_GUI_LOG_ERROR("parent is not active"); }

    _slot = new_slot;
}
void Element::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    // validate
    if (_lifecycle != EElementLifecycle::Mounted) { SKR_GUI_LOG_ERROR("element is not active"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR("widget is nullptr"); }
    if (new_widget == _widget) { SKR_GUI_LOG_ERROR("new_widget is same as old widget"); }
    if (_widget && !Widget::can_update(make_not_null(_widget), new_widget)) { SKR_GUI_LOG_ERROR("can not update widget"); }

    _widget = new_widget;
}

// render object (self or child's)
RenderObject* Element::render_object() const SKR_NOEXCEPT { return nullptr; }

// help functions
Element* Element::_update_child(Element* child, Widget* new_widget, Slot new_slot) SKR_NOEXCEPT
{
    // validate
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR("owner is nullptr"); }

    // new widget is nullptr, just deactivate child
    if (new_widget == nullptr)
    {
        if (child)
        {
            child->unmount();
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
            child->unmount();
            if (child->_parent != nullptr) { SKR_LOG_ERROR("child's parent is not nullptr after deactivate"); }
            return _inflate_widget(make_not_null(new_widget), new_slot);
        }
    }
    else
    {
        return _inflate_widget(make_not_null(new_widget), new_slot);
    }
}
NotNull<Element*> Element::_inflate_widget(NotNull<Widget*> new_widget, Slot slot) SKR_NOEXCEPT
{
    // TODO. process global key

    Element* const new_child = new_widget->create_element();
    new_child->mount(make_not_null(this), slot);
    return make_not_null(new_child);
}

} // namespace skr::gui