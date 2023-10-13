#pragma once
#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/framework/render_object/single_child_render_object.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/render_window.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
// 概念性的 Window，并不一定是 Root，Root 通常是 RenderNativeWindow
struct IWindow;
sreflect_struct(
    "guid": "358b1333-d5b8-4529-b4ad-9c800d5c9caf",
    "rtti": true
)
SKR_GUI_API RenderWindow : public RenderObject,
                           public ISingleChildRenderObject
{
    SKR_RTTR_GENERATE_BODY()

    RenderWindow(IWindow * window);

    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;
    void perform_layout() SKR_NOEXCEPT override;

    NotNull<OffsetLayer*> update_layer(OffsetLayer * old_layer) override;

    // getter
    inline IWindow* window() const SKR_NOEXCEPT { return _window; }

private:
    IWindow* _window = nullptr;

    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderWindow, RenderBox)
};
} // namespace gui sreflect
} // namespace skr sreflect