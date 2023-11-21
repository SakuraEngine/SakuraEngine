#include "SkrGui/framework/element/render_object_element.hpp"
#include "SkrGui/framework/widget/render_object_widget.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"

namespace skr::gui
{
// lifecycle & tree
void RenderObjectElement::first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    Super::first_mount(parent, slot);
    // validate
    if (widget() == nullptr)
    {
        SKR_GUI_LOG_ERROR(u8"widget is nullptr");
        _cancel_dirty();
        return;
    }

    // create render object and mount
    auto render_object_widget = widget()->type_cast_fast<RenderObjectWidget>();
    if (render_object_widget)
    {
        _render_object = render_object_widget->create_render_object();
        attach_render_object_to_parent(slot);
        _cancel_dirty();
    }
    else
    {
        SKR_GUI_LOG_ERROR(u8"widget is not RenderObjectWidget");
    }
}
void RenderObjectElement::detach() SKR_NOEXCEPT
{
    Super::detach();
    if (_render_object)
    {
        if (_render_object->owner()) { SKR_GUI_LOG_ERROR(u8"render_object is not detached"); }
    }
    else
    {
        SKR_GUI_LOG_ERROR(u8"_render_object is nullptr");
    }
}
void RenderObjectElement::destroy() SKR_NOEXCEPT
{
    if (widget() == nullptr)
    {
        SKR_GUI_LOG_ERROR(u8"widget is nullptr");
        Super::destroy();
        return;
    }
    auto old_widget = widget()->type_cast_fast<RenderObjectWidget>();
    if (old_widget)
    {
        if (_render_object)
        {
            old_widget->did_unmount_render_object(_render_object);
            _render_object->destroy();
            _render_object = nullptr;
        }
        else
        {
            SKR_GUI_LOG_ERROR(u8"_render_object is nullptr");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR(u8"widget is not RenderObjectWidget");
    }
    Super::destroy();
}

// build & update
void RenderObjectElement::perform_rebuild() SKR_NOEXCEPT
{
    _update_render_object();
}
void RenderObjectElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    Super::update(new_widget);
    if (widget() != new_widget) { SKR_GUI_LOG_ERROR(u8"widget is not equal to new_widget"); }
    _update_render_object();
}

// render object
RenderObject* RenderObjectElement::render_object() const SKR_NOEXCEPT
{
    if (_render_object == nullptr) { SKR_GUI_LOG_ERROR(u8"_render_object is nullptr"); }
    return _render_object;
}

// attach & detach & move
void RenderObjectElement::update_slot(Slot new_slot) SKR_NOEXCEPT
{
    // validate
    if (lifecycle() != EElementLifecycle::Mounted) { SKR_GUI_LOG_ERROR(u8"element is not active"); }
    if (widget() == nullptr) { SKR_GUI_LOG_ERROR(u8"widget is nullptr"); }
    if (parent() == nullptr) { SKR_GUI_LOG_ERROR(u8"parent is nullptr"); }
    if (parent() && parent()->lifecycle() != EElementLifecycle::Mounted) { SKR_GUI_LOG_ERROR(u8"parent is not active"); }

    if (_ancestor_render_object_element && slot() != new_slot)
    {
        _ancestor_render_object_element->move_render_object_child(_render_object, slot(), new_slot);
    }
}
void RenderObjectElement::attach_render_object_to_parent(Slot slot) SKR_NOEXCEPT
{
    if (_ancestor_render_object_element != nullptr) SKR_GUI_LOG_ERROR(u8"render_object is already attached");
    if (_render_object == nullptr) SKR_GUI_LOG_ERROR(u8"_render_object is nullptr");
    _ancestor_render_object_element = _find_ancestor_render_object_element();
    if (_ancestor_render_object_element == nullptr)
    {
        SKR_GUI_LOG_ERROR(u8"cannot find ancestor render_object");
        return;
    }
    _ancestor_render_object_element->add_render_object_child(_render_object, slot);
}
void RenderObjectElement::detach_render_object_from_parent() SKR_NOEXCEPT
{
    if (_ancestor_render_object_element)
    {
        _ancestor_render_object_element->remove_render_object_child(_render_object, slot());
        _ancestor_render_object_element = nullptr;
    }
}

// help functions
void RenderObjectElement::_update_render_object() SKR_NOEXCEPT
{
    if (widget() == nullptr)
    {
        SKR_GUI_LOG_ERROR(u8"widget is nullptr");
        _cancel_dirty();
        return;
    }
    auto render_object_widget = widget()->type_cast_fast<RenderObjectWidget>();
    if (render_object_widget)
    {
        render_object_widget->update_render_object(this, _render_object);
    }
    else
    {
        SKR_GUI_LOG_ERROR(u8"widget is not RenderObjectWidget");
    }
    _cancel_dirty();
}
RenderObjectElement* RenderObjectElement::_find_ancestor_render_object_element() const SKR_NOEXCEPT
{
    Element* ancestor = parent();
    while (ancestor && !ancestor->type_is<RenderObjectElement>())
    {
        ancestor = ancestor->parent();
    }
    return ancestor ? ancestor->type_cast_fast<RenderObjectElement>() : nullptr;
}

} // namespace skr::gui