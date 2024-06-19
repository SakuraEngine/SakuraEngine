#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"
#ifndef __meta__
    #include "SkrGui/widgets/color_picker.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "6e295661-ef44-44a3-9701-e44902de82eb"
)
SKR_GUI_API ColorPicker : public LeafRenderObjectWidget {
    SKR_RTTR_GENERATE_BODY()

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    bool is_srgb = true;
};
} // namespace skr::gui