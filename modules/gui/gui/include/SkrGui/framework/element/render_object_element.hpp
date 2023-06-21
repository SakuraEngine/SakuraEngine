#pragma once
#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderObjectElement : public Element {
    SKR_GUI_TYPE(RenderObjectElement, "947962f5-5bd7-4f54-b6dd-277a9c2a3760", Element)
    using Super = Element;
    using Super::Super;

    // life circle
    void mount(Element* parent, uint64_t slot) SKR_NOEXCEPT override;
    void deactivate() SKR_NOEXCEPT override;
    void unmount() SKR_NOEXCEPT override;

    // build & update
    void perform_rebuild() SKR_NOEXCEPT override;
    void update_slot(uint64_t new_slot) SKR_NOEXCEPT override;
    void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    // render object (self or child's)
    RenderObject* render_object() const SKR_NOEXCEPT override;

    // child ops
    virtual void add_render_object_child(NotNull<RenderObject*> child, uint64_t slot) SKR_NOEXCEPT = 0;
    virtual void remove_render_object_child(NotNull<RenderObject*> child, uint64_t slot) SKR_NOEXCEPT = 0;
    virtual void move_render_object_child(NotNull<RenderObject*> child, uint64_t old_slot, uint64_t new_slot) SKR_NOEXCEPT = 0;

private:
    // help functions
    void _update_render_object() SKR_NOEXCEPT;

private:
    RenderObject* _render_object = nullptr;
};
} // namespace skr::gui