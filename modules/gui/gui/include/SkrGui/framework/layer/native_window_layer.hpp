#pragma once
#include "SkrGui/framework/layer/window_layer.hpp"

namespace skr::gui
{
struct INativeWindow;
struct SKR_GUI_API NativeWindowLayer : public WindowLayer {
    SKR_GUI_OBJECT(NativeWindowLayer, "e787005d-8633-42a0-87f3-841e1b9435b3", WindowLayer)
    NativeWindowLayer(INativeWindow* native_window);
};
} // namespace skr::gui