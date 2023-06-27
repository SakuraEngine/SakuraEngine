#pragma once
#include "SkrGui/framework/element/render_object_element.hpp"

namespace skr::gui
{
struct LeafRenderObjectElement : public RenderObjectElement {
    SKR_GUI_OBJECT(LeafRenderObjectElement, "a9295b40-1b17-417b-899e-7a1fe881f990", RenderObjectElement)
    using Super = RenderObjectElement;
    using Super::Super;

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}

    // child render object ops
    void add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT override;
};
} // namespace skr::gui