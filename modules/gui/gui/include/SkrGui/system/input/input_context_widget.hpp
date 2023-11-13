#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#ifndef __meta__
    #include "SkrGui/system/input/input_context_widget.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
struct InputManager;

sreflect_struct("guid": "7f7e0555-c7da-423f-a4cd-f4d8193aa0eb")
SKR_GUI_API InputContextWidget : public SingleChildRenderObjectWidget {

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    InputManager* manager;
};
} // namespace gui sreflect
} // namespace skr sreflect