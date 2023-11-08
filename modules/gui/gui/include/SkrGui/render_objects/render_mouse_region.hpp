#pragma once
#include "SkrGui/framework/render_object/render_proxy_box.hpp"
#ifndef __meta__
    #include "SkrGui/render_objects/render_mouse_region.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct PointerEnterEvent;
struct PointerExitEvent;
struct PointerMoveEvent;

sreflect_struct(
    "guid": "0de9790c-5a02-470f-92bd-81ca6fb282f2",
    "rtti": true
)
RenderMouseRegion : public RenderProxyBox {
    using Super = RenderProxyBox;
    SKR_RTTR_GENERATE_BODY()

    bool hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT override;
    bool handle_event(NotNull<PointerEvent*> event, NotNull<HitTestEntry*> entry) override;

public:
    Function<bool(PointerEnterEvent*)> on_enter = {};
    Function<bool(PointerExitEvent*)>  on_exit  = {};
    Function<bool(PointerMoveEvent*)>  on_hover = {};
};
} // namespace gui sreflect
} // namespace skr sreflect