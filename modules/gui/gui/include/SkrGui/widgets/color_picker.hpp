#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API ColorPicker : public LeafRenderObjectWidget {
    SKR_GUI_OBJECT(ColorPicker, "270fc1d7-19ab-4f1a-8c10-e08b69393425", LeafRenderObjectWidget)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    bool is_srgb = true;
};
} // namespace skr::gui