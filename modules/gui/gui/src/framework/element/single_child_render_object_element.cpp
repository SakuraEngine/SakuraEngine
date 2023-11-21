#include "SkrGui/framework/element/single_child_render_object_element.hpp"
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"

namespace skr::gui
{
// lifecycle & tree
void SingleChildRenderObjectElement::first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    Super::first_mount(parent, slot);
    if (widget()->type_cast_fast<SingleChildRenderObjectWidget>()->child)
    {
        _child = _update_child(_child, widget()->type_cast_fast<SingleChildRenderObjectWidget>()->child, {});
    }
}
void SingleChildRenderObjectElement::visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT
{
    if (_child)
    {
        visitor(_child);
    }
}

// build & update
void SingleChildRenderObjectElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    Super::update(new_widget);
    _child = _update_child(_child, widget()->type_cast<SingleChildRenderObjectWidget>()->child, {});
}

// child render object ops
void SingleChildRenderObjectElement::add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    ISingleChildRenderObject* single_child_render_object = render_object()->type_cast<ISingleChildRenderObject>();
    if (!child->type_is(single_child_render_object->accept_child_type()))
    {
        SKR_GUI_LOG_ERROR(u8"child type not match");
    }
    single_child_render_object->set_child(child);
}
void SingleChildRenderObjectElement::remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    ISingleChildRenderObject* single_child_render_object = render_object()->type_cast<ISingleChildRenderObject>();
    single_child_render_object->remove_child();
}
void SingleChildRenderObjectElement::move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT
{
    SKR_UNREACHABLE_CODE()
}
} // namespace skr::gui