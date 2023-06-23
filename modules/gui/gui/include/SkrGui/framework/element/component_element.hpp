#pragma once
#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API ComponentElement : public Element {
    SKR_GUI_TYPE(ComponentElement, "46b19d80-a5bb-4bce-9f98-5f218cc75229", Element)
    using Super = Element;
    using Super::Super;

    // lifecycle & tree
    virtual void first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT override;
    void         visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    // build & update
    void perform_rebuild() SKR_NOEXCEPT override;
    void update_slot(Slot new_slot) SKR_NOEXCEPT override;

    // render object (self or child's)
    RenderObject* render_object() const SKR_NOEXCEPT override;

    virtual Widget* build() SKR_NOEXCEPT = 0;

private:
    Element* _child;
};
} // namespace skr::gui