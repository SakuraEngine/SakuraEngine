#pragma once
#include "SkrGui/framework/layer/window_layer.hpp"
#include "SkrGui/framework/layer/native_window_layer.generated.h"

namespace skr::gui
{
struct INativeWindow;
struct [[sattr(guid = "73726a05-07c1-4626-b608-bc78364508c5"
)]] SKR_GUI_API NativeWindowLayer : public WindowLayer
{
    SKR_GENERATE_BODY(NativeWindowLayer)
    NativeWindowLayer(INativeWindow* native_window);
};
} // namespace skr::gui