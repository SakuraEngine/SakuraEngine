#pragma once
#include "SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gui
{
struct SKR_GUI_API Positioned : public SingleChildRenderObjectWidget {
    SKR_GUI_OBJECT(Positioned, "7084ef1f-3c12-43d9-b2b0-c6dfa2fab257", SingleChildRenderObjectWidget)

    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    Positional positional = {};
};
} // namespace skr::gui