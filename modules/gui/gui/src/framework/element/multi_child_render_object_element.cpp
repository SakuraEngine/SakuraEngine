#include "SkrGui/framework/element/multi_child_render_object_element.hpp"
#include "SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "SkrGui/framework/render_object/multi_child_render_object.hpp"

namespace skr::gui
{
// lifecycle & tree
void MultiChildRenderObjectElement::first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT
{
    Super::first_mount(parent, slot);
    auto multi_child_widget = widget()->type_cast_fast<MultiChildRenderObjectWidget>();
    _children.reserve(multi_child_widget->children.size());

    for (size_t i = 0; i < multi_child_widget->children.size(); ++i)
    {
        auto child_widget = multi_child_widget->children[i];
        auto new_child    = _inflate_widget(child_widget, Slot{ i });
        _children.add(new_child);
    }
    auto multi_child_render_object = render_object()->type_cast<IMultiChildRenderObject>();
    multi_child_render_object->flush_updates();
}
void MultiChildRenderObjectElement::visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT
{
    for (auto child : _children)
    {
        // TODO. forgot child
        if (!visitor(child))
        {
            return;
        }
    }
}

// build & update
void MultiChildRenderObjectElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    Super::update(new_widget);
    _update_children(_children, widget()->type_cast_fast<MultiChildRenderObjectWidget>()->children);
    auto multi_child_render_object = render_object()->type_cast<IMultiChildRenderObject>();
    multi_child_render_object->flush_updates();
}

// child render object ops
void MultiChildRenderObjectElement::add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    auto multi_child_render_object = render_object()->type_cast<IMultiChildRenderObject>();
    if (!child->type_is(multi_child_render_object->accept_child_type()))
    {
        SKR_GUI_LOG_ERROR(u8"child type not match");
    }
    multi_child_render_object->add_child(child, slot);
}
void MultiChildRenderObjectElement::remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
{
    auto multi_child_render_object = render_object()->type_cast<IMultiChildRenderObject>();
    multi_child_render_object->remove_child(child, slot);
}
void MultiChildRenderObjectElement::move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT
{
    auto multi_child_render_object = render_object()->type_cast<IMultiChildRenderObject>();
    multi_child_render_object->move_child(child, old_slot, new_slot);
}
} // namespace skr::gui