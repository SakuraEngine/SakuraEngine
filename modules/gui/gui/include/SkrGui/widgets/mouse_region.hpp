#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/system/input/hit_test.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/mouse_region.generated.h"
#endif

namespace skr::gui
{
struct PointerEnterEvent;
struct PointerExitEvent;
struct PointerMoveEvent;
struct PointerDownEvent;
struct PointerUpEvent;

sreflect_struct(
    "guid": "f7ee023d-dcd0-4dfc-a095-793128ac0ad9"
)
SKR_GUI_API MouseRegin : public SingleChildRenderObjectWidget {
    SKR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    EHitTestBehavior hit_test_behaviour = EHitTestBehavior::defer_to_child;

    Function<bool(PointerEnterEvent*)> on_enter = {};
    Function<bool(PointerExitEvent*)>  on_exit  = {};
    Function<bool(PointerMoveEvent*)>  on_hover = {};
    Function<bool(PointerDownEvent*)>  on_down  = {};
    Function<bool(PointerUpEvent*)>    on_up    = {};
};
} // namespace skr::gui