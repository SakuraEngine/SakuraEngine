#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/sized_box.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "c52bbfa8-a175-4176-8fa1-5519400734cf"
)
SKR_GUI_API SizedBox : public SingleChildRenderObjectWidget {
    SKR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    Sizef size = {};
};
} // namespace skr::gui