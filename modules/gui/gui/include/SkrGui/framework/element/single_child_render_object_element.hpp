#pragma once
#include "SkrGui/framework/element/render_object_element.hpp"

namespace skr::gui
{
struct SingleChildRenderObjectElement : public RenderObjectElement {
    SKR_GUI_OBJECT(SingleChildRenderObjectElement, "a4d38770-bc3b-4404-8224-963563234880", RenderObjectElement)
    using Super = RenderObjectElement;
    using Super::Super;

    // lifecycle & tree
    void first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    // build & update
    void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    // child render object ops
    void add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT override;

private:
    Element* _child = nullptr;
};

} // namespace skr::gui