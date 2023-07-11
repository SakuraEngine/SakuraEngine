#pragma once
#include "SkrGui/framework/widget/leaf_render_object_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API GridPaper : public LeafRenderObjectWidget {
    SKR_GUI_OBJECT(GridPaper, "85e5d8b0-d216-40f4-9932-bb284bc2adf9", LeafRenderObjectWidget)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;
};
} // namespace skr::gui