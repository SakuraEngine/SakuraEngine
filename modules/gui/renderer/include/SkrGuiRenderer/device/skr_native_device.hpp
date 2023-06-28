#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/device/device.hpp"

namespace skr::gui
{
struct SKR_GUI_RENDERER_API SkrNativeDevice : public INativeDevice {
    SKR_GUI_OBJECT(SkrNativeDevice, "27cf3f49-efda-4ae8-b5e6-89fb3bb0590e", INativeDevice)
};
} // namespace skr::gui
