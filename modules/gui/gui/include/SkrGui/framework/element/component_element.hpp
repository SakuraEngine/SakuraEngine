#pragma once
#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API ComponentElement : public Element {
    SKR_GUI_TYPE(ComponentElement, "46b19d80-a5bb-4bce-9f98-5f218cc75229", Element)
    using Super = Element;
    using Super::Super;

    // element tree
    void flush_depth() SKR_NOEXCEPT override;
    void visit_children(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT override;
    void visit_children_recursive(FunctionRef<void(Element*)> visitor) const SKR_NOEXCEPT override;

    // life circle
    void mount(Element* parent, uint64_t slot) SKR_NOEXCEPT override;
    void activate() SKR_NOEXCEPT override;
    void deactivate() SKR_NOEXCEPT override;

    // build & update
    void perform_rebuild() SKR_NOEXCEPT override;
    void update_slot(uint64_t new_slot) SKR_NOEXCEPT override;

    // render object (self or child's)
    RenderObject* render_object() const SKR_NOEXCEPT override;

    virtual Widget* build() SKR_NOEXCEPT = 0;

private:
    Element* _child;
};
} // namespace skr::gui