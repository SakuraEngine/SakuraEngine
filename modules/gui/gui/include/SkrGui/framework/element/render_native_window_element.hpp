#pragma once
#include "SkrGui/framework/element/render_window_element.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderNativeWindowElement : public RenderWindowElement {
    SKR_GUI_OBJECT(RenderNativeWindowElement, "e1cfa7d1-15fd-41b3-b72b-87b97ec24035", RenderWindowElement)
    using Super = RenderWindowElement;
    using Super::Super;

    void perform_rebuild() SKR_NOEXCEPT override;
    void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    void add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    inline void setup_owner(BuildOwner* owner) SKR_NOEXCEPT { _owner = owner; }
    void        prepare_initial_frame() SKR_NOEXCEPT;

    void set_new_child_widget(Widget* widget) SKR_NOEXCEPT;

private:
    // helper
    void _rebuild();

private:
    Widget*  _new_child_widget = nullptr;
    Element* _child            = nullptr;
};
} // namespace skr::gui