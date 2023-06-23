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
        SKR_GUI_LOG_ERROR("widget is nullptr");
        _cancel_dirty();
        return;
    }

    // create render object and mount
    auto render_object_widget = widget()->type_cast_fast<RenderObjectWidget>();
    if (render_object_widget)
    {
        _render_object = render_object_widget->create_render_object();
        // TODO. attach render object
        _cancel_dirty();
    }
    else
    {
        SKR_GUI_LOG_ERROR("widget is not RenderObjectWidget");
    }
}
void RenderObjectElement::detach() SKR_NOEXCEPT
{
    Super::detach();
    if (_render_object)
    {
        if (_render_object->owner()) { SKR_GUI_LOG_ERROR("render_object is not detached"); }
    }
    else
    {
        SKR_GUI_LOG_ERROR("_render_object is nullptr");
    }
}
void RenderObjectElement::destroy() SKR_NOEXCEPT
{
    if (widget() == nullptr)
    {
        SKR_GUI_LOG_ERROR("widget is nullptr");
        Super::destroy();
        return;
    }
    auto old_widget = widget()->type_cast_fast<RenderObjectWidget>();
    if (old_widget)
    {
        if (_render_object)
        {
            old_widget->did_unmount_render_object(make_not_null(_render_object));
            _render_object->dispose();
            _render_object = nullptr;
        }
        else
        {
            SKR_GUI_LOG_ERROR("_render_object is nullptr");
        }
    }
    else
    {
        SKR_GUI_LOG_ERROR("widget is not RenderObjectWidget");
    }
    Super::destroy();
}

// build & update
void RenderObjectElement::perform_rebuild() SKR_NOEXCEPT
{
    _update_render_object();
}
void RenderObjectElement::update_slot(Slot new_slot) SKR_NOEXCEPT
{
    Super::update_slot(new_slot);
    // TODO. pass to render_object
}
void RenderObjectElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    Super::update(new_widget);
    if (widget() != new_widget) { SKR_GUI_LOG_ERROR("widget is not equal to new_widget"); }
    _update_render_object();
}

// render object (self or child's)
RenderObject* RenderObjectElement::render_object() const SKR_NOEXCEPT
{
    if (_render_object == nullptr) { SKR_GUI_LOG_ERROR("_render_object is nullptr"); }
    return _render_object;
}

// help functions
void RenderObjectElement::_update_render_object() SKR_NOEXCEPT
{
    if (widget() == nullptr)
    {
        SKR_GUI_LOG_ERROR("widget is nullptr");
        _cancel_dirty();
        return;
    }
    auto render_object_widget = widget()->type_cast_fast<RenderObjectWidget>();
    if (render_object_widget)
    {
        render_object_widget->update_render_object(make_not_null(this), make_not_null(_render_object));
    }
    else
    {
        SKR_GUI_LOG_ERROR("widget is not RenderObjectWidget");
    }
    _cancel_dirty();
}
} // namespace skr::gui