#pragma once
#include "SkrGui/framework/render_object/render_window.hpp"

namespace skr::gui
{
struct INativeWindow;
struct SKR_GUI_API RenderNativeWindow : public RenderWindow {
    SKR_GUI_OBJECT(RenderNativeWindow, "f4611440-7768-4975-b24e-5c2c7156f661", RenderWindow)
    RenderNativeWindow(INativeWindow* native_window);
};
} // namespace skr::gui