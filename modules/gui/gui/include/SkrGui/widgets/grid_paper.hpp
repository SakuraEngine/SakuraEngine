#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/grid_paper.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "e228c75e-890e-4724-94c9-21d0e7f01587",
    "rtti": true
)
SKR_GUI_API GridPaper : public LeafRenderObjectWidget
{
    SKR_RTTR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;
};
} // namespace gui sreflect
} // namespace skr sreflect