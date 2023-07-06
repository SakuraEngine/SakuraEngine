#pragma once
#include "SkrGui/framework/widget/render_window_widget.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderNativeWindowWidget : public RenderWindowWidget {
    SKR_GUI_OBJECT(RenderNativeWindowWidget, "b405f95b-3ca9-4fde-819d-39256386d1ca", RenderWindowWidget)

    NotNull<Element*>      create_element() SKR_NOEXCEPT override;
    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    RenderNativeWindow* native_window_render_object = nullptr;
};
} // namespace skr::gui