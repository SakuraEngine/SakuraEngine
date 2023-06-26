#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
// Device 管理 DeviceViewport, 其既可能是物理设备，比如 desktop 的 nativeWindow
// 也可能是虚拟的，比如 3D UI 就可以单独持有一个虚拟的 Device
// Event 从 Device 流入，由 Device 进行管理，让 Device 告知
struct SKR_GUI_API IDevice {
    virtual ~IDevice() = default;
};

struct SKR_GUI_API INativeDevice : public IDevice {
};
} // namespace skr::gui