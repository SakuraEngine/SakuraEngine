#pragma once
#include "SkrGui/framework/layer/offet_layer.hpp"

namespace skr::gui
{
struct IWindow;
struct SKR_GUI_API WindowLayer : public OffsetLayer {
    SKR_GUI_OBJECT(WindowLayer, "7ffdd5d0-9ed8-4f1b-87ab-8e08a29333c2", OffsetLayer)
    WindowLayer(IWindow* window);

    void update_window();

private:
    IWindow* _window = nullptr;
};
} // namespace skr::gui