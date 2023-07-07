#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API Text : public LeafRenderObjectWidget {
    SKR_GUI_OBJECT(Text, "b852a75d-8035-422f-8738-e251b15acb26", LeafRenderObjectWidget)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    String text = {};
};
} // namespace skr::gui