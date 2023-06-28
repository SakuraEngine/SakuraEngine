#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/device_view.hpp"

namespace skr::gui
{
struct SKR_GUI_RENDERER_API SkrNativeDeviceView : public INativeDeviceView {
    SKR_GUI_OBJECT(SkrNativeDeviceView, "093aa38f-f5f8-4aa9-92ed-5eafa6b797d5", INativeDeviceView)
};
} // namespace skr::gui
