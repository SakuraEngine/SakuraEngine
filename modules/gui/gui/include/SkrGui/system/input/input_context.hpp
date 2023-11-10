#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/input_context.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct InputManager;

sreflect_struct(
    "guid": "b4454cc3-68b6-4ac5-b5e1-b0dc5ba25334"
)
SKR_GUI_API InputContext {
    InputContext(NotNull<InputManager*> manager, NotNull<RenderBox*> widget);

    // hit test
    bool hit_test(HitTestResult* result, Offsetf global_position);

    // getter
    inline InputManager* owner() const noexcept { return _owner; }
    inline RenderBox*    widget() const noexcept { return _widget; }

public:
    InputManager* _owner  = nullptr;
    RenderBox*    _widget = nullptr;
};
} // namespace gui sreflect
} // namespace skr sreflect