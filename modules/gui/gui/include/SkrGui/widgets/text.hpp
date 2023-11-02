#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/text.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "ba805fb4-6b04-4a41-9c7d-605b61d194aa",
    "rtti": true
)
SKR_GUI_API Text : public LeafRenderObjectWidget
{
    SKR_RTTR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    String text = {};
};
} // namespace gui sreflect
} // namespace skr sreflect