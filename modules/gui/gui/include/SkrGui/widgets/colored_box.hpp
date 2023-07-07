#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/math/color.hpp"

namespace skr::gui
{
struct SKR_GUI_API ColoredBox : public SingleChildRenderObjectWidget {
    SKR_GUI_OBJECT(ColoredBox, "5ae444e6-eb5a-44a1-aeb0-9050c8587795", SingleChildRenderObjectWidget)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    Color color = {};
};
} // namespace skr::gui