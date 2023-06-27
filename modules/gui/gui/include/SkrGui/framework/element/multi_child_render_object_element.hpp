#pragma once
#include "SkrGui/framework/element/render_object_element.hpp"

namespace skr::gui
{
struct MultiChildRenderObjectElement : public RenderObjectElement {
    SKR_GUI_OBJECT(MultiChildRenderObjectElement, "145cedaa-cc1c-4a06-acab-f83a30d997a4", RenderObjectElement)
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
    Array<Element*> _children = {};
    // TODO. 控件重用与 forgot_child
    // Array<Element*> _forgotten_children = {};
};
} // namespace skr::gui