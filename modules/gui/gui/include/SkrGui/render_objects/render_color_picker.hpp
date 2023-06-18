#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"

namespace skr::gui
{
struct SKR_GUI_API RenderColorPicker : public RenderBox {
    SKR_GUI_TYPE(RenderColorPicker, "25a95354-b3fa-4729-b06f-1a85d0f227c4", RenderBox);
    using Super = RenderBox;

    RenderColorPicker();
    virtual ~RenderColorPicker();

    void perform_layout() SKR_NOEXCEPT override;
    void paint(NotNull<PaintingContext*> context, Offset offset) SKR_NOEXCEPT override;
};

} // namespace skr::gui
