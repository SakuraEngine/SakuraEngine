#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/window.hpp"

namespace skr::gui
{
struct SKR_GUI_RENDERER_API SkrNativeWindow : public INativeWindow {
    SKR_GUI_OBJECT(SkrNativeWindow, "093aa38f-f5f8-4aa9-92ed-5eafa6b797d5", INativeWindow)
};
} // namespace skr::gui
