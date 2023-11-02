#pragma once
#include "SkrGui/framework/element/render_window_element.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/render_native_window_element.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "cddf8085-4ab4-46be-8de7-34bd1a2c290e",
    "rtti": true
)
SKR_GUI_API RenderNativeWindowElement : public RenderWindowElement
{
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderWindowElement;
    using Super::Super;

    void perform_rebuild() SKR_NOEXCEPT override;
    void update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;

    void add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    inline void setup_owner(BuildOwner * owner) SKR_NOEXCEPT { _owner = owner; }
    void        prepare_initial_frame() SKR_NOEXCEPT;

    void set_new_child_widget(Widget * widget) SKR_NOEXCEPT;

private:
    // helper
    void _rebuild();

private:
    Widget*  _new_child_widget = nullptr;
    Element* _child            = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect