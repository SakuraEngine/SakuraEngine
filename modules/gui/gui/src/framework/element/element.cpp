#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/element/render_object_element.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/build_owner.hpp"
#include "SkrGui/framework/element/render_object_element.hpp"

namespace skr::gui
{
Element::Element(Widget* widget) SKR_NOEXCEPT
    : _widget(widget)
{
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR(u8"widget is nullptr"); }
}

// lifecycle & tree
// ctor -> mount <-> unmount -> destroy
void Element::mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    // validate
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR(u8"widget is nullptr"); }
    if (_parent != nullptr) { SKR_GUI_LOG_ERROR(u8"already mounted"); }

    // mount
    _parent = parent;
    if (_parent->_owner)
    {
        // recursive call attach()
        struct _RecursiveHelper {
            NotNull<BuildOwner*> owner;

            void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
            {
                obj->attach(owner);
                obj->visit_children(_RecursiveHelper{ owner });
            }
        };
        _RecursiveHelper{ make_not_null(_parent->_owner) }(make_not_null(this));
        this->visit_children(_RecursiveHelper{ make_not_null(_parent->_owner) });

        // attach render object when retake
        if (_lifecycle == EElementLifecycle::Unmounted)
        {
            _attach_render_object_children(slot);
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR(u8"owner is nullptr");
    }
    if (_lifecycle == EElementLifecycle::Initial)
    {
        first_mount(parent, slot);
    }
    _lifecycle = EElementLifecycle::Mounted;
}
void Element::unmount() SKR_NOEXCEPT
{
    // validate
    if (_parent == nullptr)
    {
        SKR_GUI_LOG_ERROR(u8"already unmounted");
        return;
    }

    // unmount
    _parent = nullptr;
    if (_owner)
    {
        // detach render object
        _detach_render_object_children();

        // drop self to owner
        _owner->drop_unmount_element(make_not_null(this));

        // recursive call detach()
        struct _RecursiveHelper {
            void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
            {
                obj->detach();
                obj->visit_children(_RecursiveHelper{});
            }
        };
        _RecursiveHelper{}(make_not_null(this));
        this->visit_children(_RecursiveHelper{});
    }
    _lifecycle = EElementLifecycle::Unmounted;
}
void Element::destroy() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle != EElementLifecycle::Unmounted) { SKR_GUI_LOG_ERROR(u8"before destroy, must unmount"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR(u8"widget is nullptr"); }
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR(u8"owner is nullptr"); }

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
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR(u8"widget is nullptr"); }
    if (owner == nullptr) { SKR_GUI_LOG_ERROR(u8"owner is nullptr"); }

    // update depth
    _owner = owner;
    _depth = _parent ? _parent->_depth + 1 : 0;

    if (_lifecycle != EElementLifecycle::Initial)
    {
        // TODO. process dependencies
        // TODO. process notify
    }
}
void Element::detach() SKR_NOEXCEPT
{
    // validate
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR(u8"widget is nullptr"); }
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR(u8"owner is nullptr"); }

    // TODO. process dependencies
}

// mark functions
void Element::mark_needs_build() SKR_NOEXCEPT
{
    // validate
    if (_lifecycle == EElementLifecycle::Destroyed) { SKR_GUI_LOG_ERROR(u8"already unmounted"); }
    if (_lifecycle != EElementLifecycle::Mounted) { return; }
    if (_owner == nullptr)
    {
        SKR_GUI_LOG_ERROR(u8"owner is nullptr");
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
    if (_lifecycle == EElementLifecycle::Initial) { SKR_GUI_LOG_ERROR(u8"element is incomplete"); }
    if (_lifecycle != EElementLifecycle::Mounted || (!_dirty && !force))
    {
        return;
    }

    perform_rebuild();

    // validate
    if (_dirty) { SKR_GUI_LOG_ERROR(u8"perform_rebuild() must set dirty to false"); }
}
void Element::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    // validate
    if (_lifecycle != EElementLifecycle::Mounted) { SKR_GUI_LOG_ERROR(u8"element is not active"); }
    if (_widget == nullptr) { SKR_GUI_LOG_ERROR(u8"widget is nullptr"); }
    if (new_widget == _widget) { SKR_GUI_LOG_ERROR(u8"new_widget is same as old widget"); }
    if (_widget && !Widget::can_update(make_not_null(_widget), new_widget)) { SKR_GUI_LOG_ERROR(u8"can not update widget"); }

    _widget = new_widget;
}

//==> Begin IBuildContext API
Widget*       Element::bound_widget() const SKR_NOEXCEPT { return widget(); }
BuildOwner*   Element::build_owner() const SKR_NOEXCEPT { return _owner; }
bool          Element::is_destroyed() const SKR_NOEXCEPT { return _lifecycle == EElementLifecycle::Destroyed; }
RenderObject* Element::find_render_object() const SKR_NOEXCEPT
{
    const Element* cur_element = this;
    while (cur_element)
    {
        if (auto render_object_element = cur_element->type_cast<RenderObjectElement>())
        {
            return render_object_element->render_object();
        }
        else
        {
            cur_element->visit_children([&cur_element](NotNull<Element*> element) {
                cur_element = element;
            });
        }
    }
    return nullptr;
}
RenderObject* Element::find_ancestor_render_object() const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
Optional<Sizef> Element::render_box_size() const SKR_NOEXCEPT
{
    if (auto render_object = find_render_object())
    {
        if (auto render_box = render_object->type_cast<RenderBox>())
        {
            return render_box->size();
        }
    }
    return {};
}
InheritedWidget* Element::depend_on_inherited_element(NotNull<InheritedElement*> ancestor) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
InheritedWidget* Element::depend_on_inherited_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
InheritedElement* Element::get_element_for_inherited_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
Widget* Element::find_ancestor_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
State* Element::find_ancestor_state_of_exact_type(const GUID& type_id) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
State* Element::find_root_ancestor_state_of_exact_type(const GUID& type_id) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
RenderObject* Element::find_ancestor_render_object_of_exact_type(const GUID& type_id) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
void Element::visit_ancestor_elements(FunctionRef<bool(NotNull<Element*>)> visitor) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
void Element::visit_child_elements(FunctionRef<void(NotNull<Element*>)> visitor) SKR_NOEXCEPT
{
    visit_children(visitor);
}
void Element::dispatch_notification(NotNull<Notification*> notification) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
//==> End IBuildContext API

// help functions
Element* Element::_update_child(Element* child, Widget* new_widget, Slot new_slot) SKR_NOEXCEPT
{
    // validate
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR(u8"owner is nullptr"); }

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
            _update_slot_for_child(make_not_null(child), new_slot);
            return child;
        }
        else if (Widget::can_update(make_not_null(child->_widget), make_not_null(new_widget)))
        {
            _update_slot_for_child(make_not_null(child), new_slot);
            child->update(make_not_null(new_widget));
            return child;
        }
        else
        {
            child->unmount();
            if (child->_parent != nullptr) { SKR_LOG_ERROR(u8"child's parent is not nullptr after deactivate"); }
            return _inflate_widget(make_not_null(new_widget), new_slot);
        }
    }
    else
    {
        return _inflate_widget(make_not_null(new_widget), new_slot);
    }
}
void Element::_update_children(Array<Element*>& children, const Array<Widget*>& new_widgets)
{
    size_t children_match_front = 0;
    size_t widget_match_front   = 0;
    size_t children_match_end   = children.size() - 1;
    size_t widget_match_end     = new_widgets.size() - 1;

    // step 1. update array size
    if (children.size() < new_widgets.size())
    {
        children.resize(new_widgets.size(), nullptr);
    }

    // step 2. walk matched front part of children and update node
    while (children_match_front <= children_match_end && widget_match_front <= widget_match_end)
    {
        auto& child      = children[children_match_front];
        auto  new_widget = new_widgets[widget_match_front];

        if (child == nullptr || !Widget::can_update(make_not_null(child->widget()), make_not_null(new_widget)))
        {
            break;
        }
        else
        {
            child = _update_child(child, make_not_null(new_widget), Slot{ widget_match_front });
            ++children_match_front;
            ++widget_match_front;
        }
    }

    // step 3. walk matched back part of children without update node, but move to end
    while (children_match_front <= children_match_end && widget_match_front <= widget_match_end)
    {
        auto& child      = children[children_match_end];
        auto  new_widget = new_widgets[widget_match_end];

        if (child == nullptr || !Widget::can_update(make_not_null(child->widget()), make_not_null(new_widget)))
        {
            break;
        }
        else
        {
            children[widget_match_end] = child;
            --children_match_end;
            --widget_match_end;
        }
    }

    // step 4. walk middle part of children and make key dict
    Map<Key, Element*> old_keyed_children;
    for (auto i = children_match_front; i <= children_match_end; ++i)
    {
        auto& child = children[i];
        if (child)
        {
            if (child->widget()->key.is_none())
            {
                child->unmount();
            }
            else
            {
                old_keyed_children.add(child->widget()->key, child);
            }
        }
        ++children_match_front;
    }

    // step 4. fill middle part of children
    while (widget_match_front <= widget_match_end)
    {
        auto     new_widget = new_widgets[widget_match_front];
        Element* child      = nullptr;

        // search old child
        if (auto found_child = old_keyed_children.find(new_widget->key))
        {
            child = found_child->value;
            if (!Widget::can_update(make_not_null(child->widget()), make_not_null(new_widget)))
            {
                child = nullptr;
            }
        }

        // update child
        Element* new_child           = _update_child(child, make_not_null(new_widget), Slot{ widget_match_front });
        children[widget_match_front] = new_child;

        ++widget_match_front;
    }

    // validate state
    if (children_match_front != children_match_end + 1) { SKR_GUI_LOG_ERROR(u8"end of build but walk pointer not close"); }
    if (widget_match_front != widget_match_end + 1) { SKR_GUI_LOG_ERROR(u8"end of build but walk pointer not close"); }

    // step 5. cleanup unused children
    for (auto& child_pair : old_keyed_children)
    {
        child_pair.value->unmount();
    }

    // step 6. walk back part and update
    for (auto i = widget_match_end; i < children.size(); ++i)
    {
        auto& child      = children[i];
        auto  new_widget = new_widgets[i];

        child = _update_child(child, make_not_null(new_widget), Slot{ i });
    }
}
NotNull<Element*> Element::_inflate_widget(NotNull<Widget*> new_widget, Slot slot) SKR_NOEXCEPT
{
    // TODO. process global key

    Element* const new_child = new_widget->create_element();
    new_child->mount(make_not_null(this), slot);
    return make_not_null(new_child);
}
void Element::_update_slot_for_child(NotNull<Element*> child, Slot new_slot) SKR_NOEXCEPT
{
    struct _RecursiveHelper {
        Slot new_slot;
        void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
        {
            obj->_slot = new_slot;
            if (auto render_object = obj->type_cast<RenderObjectElement>())
            {
                if (render_object->slot() != new_slot)
                {
                    render_object->update_slot(new_slot);
                }
            }
            else
            {
                obj->visit_children(_RecursiveHelper{ new_slot });
            }
        }
    };
    child->visit_children(_RecursiveHelper{ new_slot });
}
void Element::_attach_render_object_children(Slot new_slot) SKR_NOEXCEPT
{
    struct _RecursiveHelper {
        Slot new_slot;
        void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
        {
            if (auto render_object = obj->type_cast<RenderObjectElement>())
            {
                render_object->attach_render_object_to_parent(new_slot);
            }
            else
            {
                obj->visit_children(_RecursiveHelper{ new_slot });
            }
            obj->_slot = new_slot;
        }
    };
    visit_children(_RecursiveHelper{ new_slot });
}
void Element::_detach_render_object_children() SKR_NOEXCEPT
{
    struct _RecursiveHelper {
        void operator()(NotNull<Element*> obj) const SKR_NOEXCEPT
        {
            if (auto render_object = obj->type_cast<RenderObjectElement>())
            {
                render_object->detach_render_object_from_parent();
            }
            else
            {
                obj->visit_children(_RecursiveHelper{});
            }
            // clean up slot
            obj->_slot = Slot::Invalid();
        }
    };
    visit_children(_RecursiveHelper{});
}
} // namespace skr::gui