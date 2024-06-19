#pragma once
#include "SkrGui/framework/element/render_object_element.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/leaf_render_object_element.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "4e452c55-c545-4602-a9d2-76232f561536"
)
LeafRenderObjectElement : public RenderObjectElement {
    SKR_RTTR_GENERATE_BODY()
    using Super = RenderObjectElement;
    using Super::Super;

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override {}

    // child render object ops
    void add_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void remove_render_object_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override;
    void move_render_object_child(NotNull<RenderObject*> child, Slot old_slot, Slot new_slot) SKR_NOEXCEPT override;
};
} // namespace skr::gui