#pragma once
#include "SkrGui/framework/widget/render_window_widget.hpp"
#include "SkrGui/framework/widget/render_native_window_widget.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "5d639330-f012-4e0e-aef7-86e96c95eb51"
)]] SKR_GUI_API RenderNativeWindowWidget : public RenderWindowWidget
{
    SKR_GENERATE_BODY(RenderNativeWindowWidget)

    NotNull<Element*> create_element() SKR_NOEXCEPT override;
    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    RenderNativeWindow* native_window_render_object = nullptr;
};
} // namespace skr::gui