#pragma once
#include "SkrGui/framework/element/element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderObjectElement : public Element {
    SKR_GUI_OBJECT(RenderObjectElement, "947962f5-5bd7-4f54-b6dd-277a9c2a3760", Element)
    using Super = Element;
    using Super::Super;

    // lifecycle & tree
    void first_mount(NotNull<Element*> parent, Slot slot) SKR_NOEXCEPT override;
    void detach() SKR_NOEXCEPT override;
    void destroy() SKR_NOEXCEPT override;

    // build & update
    void perform_rebuild() SKR_NOEXCEPT override;
    void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    // getter
    RenderObject* render_object() const SKR_NOEXCEPT;

    // child render object ops
    virtual void add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT                     = 0;
    virtual void remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT                  = 0;
    virtual void move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT = 0;

    // attach & detach & move
    void update_slot(Slot new_slot) SKR_NOEXCEPT;
    void attach_render_object_to_parent(Slot slot) SKR_NOEXCEPT;
    void detach_render_object_from_parent() SKR_NOEXCEPT;

private:
    // for special init
    friend struct RenderNativeWindowElement;

private:
    // help functions
    void                 _update_render_object() SKR_NOEXCEPT;
    RenderObjectElement* _find_ancestor_render_object_element() const SKR_NOEXCEPT;

private:
    RenderObject*        _render_object                  = nullptr;
    RenderObjectElement* _ancestor_render_object_element = nullptr;
};
} // namespace skr::gui