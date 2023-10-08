#pragma once
#include "SkrGui/framework/element/render_object_element.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/single_child_render_object_element.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "a3f51c75-2995-4f99-95c7-1166b3ba17a6"
)
SingleChildRenderObjectElement : public RenderObjectElement {
    SKR_RTTR_GENERATE_BODY()
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

} // namespace gui sreflect
} // namespace skr sreflect