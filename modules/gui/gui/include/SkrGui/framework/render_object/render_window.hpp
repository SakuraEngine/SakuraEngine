#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"

namespace skr::gui
{
// 概念性的 Window，并不一定是 Root，Root 通常是 RenderNativeWindow
struct IWindow;
struct SKR_GUI_API RenderWindow : public RenderObject, public ISingleChildRenderObject {
    SKR_GUI_OBJECT(RenderWindow, "564ff723-9f85-4c93-8efd-841700dfe104", RenderObject, ISingleChildRenderObject)

    RenderWindow(IWindow* window);

    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;
    void perform_layout() SKR_NOEXCEPT override;

    NotNull<OffsetLayer*> update_layer(OffsetLayer* old_layer) override;

    // getter
    inline IWindow* window() const SKR_NOEXCEPT { return _window; }

private:
    IWindow* _window = nullptr;

    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderWindow, RenderBox)
};
} // namespace skr::gui