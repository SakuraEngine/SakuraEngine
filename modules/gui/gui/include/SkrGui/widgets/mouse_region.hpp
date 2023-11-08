#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/mouse_region.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct PointerEnterEvent;
struct PointerExitEvent;
struct PointerMoveEvent;

sreflect_struct(
    "guid": "f7ee023d-dcd0-4dfc-a095-793128ac0ad9",
    "rtti": true
)
MouseRegin : public SingleChildRenderObjectWidget {
    SKR_RTTR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    Function<bool(PointerEnterEvent*)> on_enter = {};
    Function<bool(PointerExitEvent*)>  on_exit  = {};
    Function<bool(PointerMoveEvent*)>  on_hover = {};

    // bool opaque // 将 hit test 的 dispatch 截断于此，防止上层接收到事件，可以用路由解决
    // HitTestBehavior behaviour // hit test 行为，在 hit test 期间就阻断向下的传播
};
} // namespace gui sreflect
} // namespace skr sreflect